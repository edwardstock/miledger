#include "include/ui/consolewindow.h"

#include "include/app.h"
#include "include/miledger-config.h"
#include "include/net/explorer_repo.h"
#include "include/settings.h"
#include "include/style_helper.h"
#include "include/tab_base.h"
#include "include/tab_exchange.h"
#include "include/tab_send.h"
#include "include/ui/serversettingsdialog.h"
#include "include/utils.h"
#include "ui_consolewindow.h"

#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QSystemTrayIcon>
#include <toolbox/strings.hpp>

using namespace minter;

ConsoleWindow::ConsoleWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ConsoleWindow)
    , app(new miledger::ConsoleApp(this))
    , tabs()
    , statusIcon(new QLabel(this))
    , statusLabel(new QLabel(this))
    , statusProgress(new QProgressBar(this))
    , serverThread(nullptr) {

    mpd_setminalloc(4);

    ui->setupUi(this);
    statusProgress->setMinimum(0);
    statusProgress->setMaximum(0);
    statusProgress->setVisible(false);
    statusLabel->setContentsMargins(8, 0, 0, 0);
    statusIcon->setContentsMargins(16, 0, 0, 0);
    ui->statusBar->addWidget(statusIcon);
    ui->statusBar->addWidget(statusLabel);
    ui->statusBar->addWidget(statusProgress);
    setWindowTitle(QString("Minter Console v%1").arg(QString(MILEDGER_VERSION)));

    {
        balanceLabel = new QLabel(QString(tr("Balance:")));
        balanceRefresh = new QPushButton(StyleHelper::get().icon("ic_refresh"), "");

        addressLabel = new QLabel(QString("Address: loading...%1").arg(QString::fromStdString(toolbox::strings::repeat(" ", 32))));
        addressCopy = new QPushButton(StyleHelper::get().icon("ic_copy_content"), "");
        rightSpace = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        balanceLabel->setContentsMargins(0, 0, 4, 0);
        balanceRefresh->setContentsMargins(4, 0, 4, 0);
        balanceRefresh->setEnabled(false);

        addressLabel->setContentsMargins(0, 0, 4, 0);
        addressCopy->setContentsMargins(4, 0, 4, 0);
        addressCopy->setEnabled(false);

        ui->layoutBalance->addItem(rightSpace, 0, 2, 1, 1);

        ui->layoutBalance->addWidget(balanceLabel, 0, 0, 1, 1);
        ui->layoutBalance->addWidget(balanceRefresh, 0, 1, 1, 1);

        ui->layoutBalance->addWidget(addressLabel, 1, 0, 1, 1);
        ui->layoutBalance->addWidget(addressCopy, 1, 1, 1, 1);

        if (miledger::App::get().useMnemonic()) {
            auto f = addressLabel->font();
            QFontMetrics m(f);

            QLabel* useMnemonicLabel = new QLabel(
                StyleHelper::get().iconText("<b>Mnemonic phrase is used instead real Ledger device</b>", "ic_warning", m.height()),
                this);

            ui->layoutBalance->addWidget(useMnemonicLabel, 2, 0, 1, 2);
        }

        connect(addressCopy, &QPushButton::clicked, [this](bool) {
            QClipboard* cp = QGuiApplication::clipboard();
            if (cp != nullptr) {
                cp->setText(QString::fromStdString(app->address.to_string()));
            }
        });

        ui->btnServer->setEnabled(true);
        connect(ui->btnServer, &QPushButton::clicked, this, &ConsoleWindow::onServerClicked);

        ui->btnServerSettings->setEnabled(true);
        connect(ui->btnServerSettings, &QPushButton::clicked, this, &ConsoleWindow::onServerSettingsClicked);

        createTray();
    }

    connect(balanceRefresh, SIGNAL(clicked()), this, SLOT(updateBalance()));

    connect(app, SIGNAL(started()), this, SLOT(onAppStarted()));
    connect(app, SIGNAL(deviceStateChanged(dev_state)), this, SLOT(onDeviceStateChanged(dev_state)));
    connect(app, SIGNAL(balanceUpdated(minter::explorer::balance_items)), this, SLOT(balanceUpdated(minter::explorer::balance_items)));
    connect(app, &miledger::ConsoleApp::deviceExchangeError, this, &ConsoleWindow::onDevExchangeError);
}

ConsoleWindow::~ConsoleWindow() {

    delete ui;
    delete app;

    delete server;
    if (serverThread && serverThread->isRunning()) {
        serverThread->quit();
        serverThread->wait();
        delete serverThread;
    }
}

void ConsoleWindow::createTray() {
    trayRestoreAction = new QAction(tr("Open Console"), this);
    connect(trayRestoreAction, &QAction::triggered, this, &QWidget::showNormal);

    trayMinimizeAction = new QAction(tr("Minimize"), this);
    connect(trayMinimizeAction, &QAction::triggered, this, &QWidget::hide);

    trayStopServerAndClose = new QAction(tr("Stop server and close"), this);
    trayStopServerAndClose->setEnabled(false);
    connect(trayStopServerAndClose, &QAction::triggered, this, [this](bool) {
        onServerClicked(false);
        close();
    });

    trayMenu = new QMenu(this);
    trayMenu->addAction(trayMinimizeAction);
    trayMenu->addAction(trayRestoreAction);
    trayMenu->addSeparator();
    trayMenu->addAction(trayStopServerAndClose);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->setIcon(QIcon(":/icons/ic_launcher_32.png"));

    connect(trayIcon, &QSystemTrayIcon::activated, this, &ConsoleWindow::iconActivated);
}

void ConsoleWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!showedFirstTime) {
        app->dev.setInfiniteEmitting(true);
        app->start();
        showedFirstTime = true;
    }
}

void ConsoleWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        show();
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    case QSystemTrayIcon::Context:
        break;
    default:
        break;
    }
}
void ConsoleWindow::showMessage() {
}
void ConsoleWindow::messageClicked() {
}

void ConsoleWindow::closeEvent(QCloseEvent* event) {
    if (Settings::get().getValue<bool>(Settings::KEY_SERVER_CLOSE_TRAY, true)) {
#ifdef Q_OS_MACOS
        if (!event->spontaneous() || !isVisible()) {
            return;
        }
#endif

        if (trayIcon->isVisible()) {
            QMessageBox::information(this, tr("Closing window"),
                                     tr("The program will keep running in the "
                                        "system tray. To terminate the program, "
                                        "choose <b>Stop server and close</b> in the context menu "
                                        "of the system tray entry."));
            hide();
            event->ignore();
            return;
        }
    }

    QMainWindow::closeEvent(event);
    if (event->isAccepted()) {
        emit closed();
    } else {
        event->accept();
    }
}

void ConsoleWindow::onServerClicked(bool) {
    if (server && server->isRunning()) {
        server->stop();
        onServerStopped();
        serverThread->quit();
        serverThread->wait();
        delete server;
        server = nullptr;
        delete serverThread;
        serverThread = nullptr;

        if (Settings::get().getValue<bool>(Settings::KEY_SERVER_CLOSE_TRAY, true)) {
            trayIcon->hide();
        }
    } else {
        if (Settings::get().getValue<bool>(Settings::KEY_SERVER_CLOSE_TRAY, true)) {
            trayStopServerAndClose->setEnabled(true);
            trayIcon->show();
        }

        serverThread = new QThread();
        server = new miledger::WsServer(app);
        server->moveToThread(serverThread);

        connect(serverThread, &QThread::started, server, &miledger::WsServer::run);
        serverThread->start();

        ui->btnServer->setText(tr("Stop server"));
        ui->btnServerSettings->setEnabled(false);
        ui->tabWidget->setEnabled(false);
    }
}

void ConsoleWindow::onServerStopped() {
    ui->btnServer->setText(tr("Run"));
    ui->tabWidget->setEnabled(true);
    ui->btnServerSettings->setEnabled(true);
}

void ConsoleWindow::onServerSettingsClicked(bool) {
    ServerSettingsDialog dialog(this);
    dialog.exec();
}

void ConsoleWindow::updateBalance() {
    app->updateBalance();
    app->updateInitData();
}

void ConsoleWindow::balanceUpdated(explorer::balance_items balances) {
    auto bipBalance = std::find_if(balances.balances.begin(), balances.balances.end(), [](explorer::balance_item item) {
        return item.coin.id == minter::def_coin_id;
    });
    if (bipBalance != balances.balances.end()) {
        addressLabel->setText(
            QString("Address: %1").arg(QString::fromStdString(balances.address.to_string())));
        balanceLabel->setText(
            QString("Balance: %1 %2").arg(miledger::utils::humanDecimal(bipBalance->amount, 4, 8), QString::fromStdString(bipBalance->coin.symbol)));
    }
}

void ConsoleWindow::onDevExchangeError(std::exception_ptr eptr) {

    statusLabel->setText(QString("Ledger: <b>Error</b>. %1").arg(miledger::utils::getError(eptr)));
}

void ConsoleWindow::onDeviceStateChanged(dev_state type) {
    qDebug() << "Device state in console: " << type;

    balanceRefresh->setEnabled(type == dev_state::APP_OPENED);
    addressCopy->setEnabled(type == dev_state::APP_OPENED);

    switch (type) {
    case dev_state::DISCONNECTED:
        balanceLabel->setText("Balance: <b>Unavailable</b>");
        addressLabel->setText("Address: <b>Unavailable</b>");
        statusLabel->setText("Ledger: <b>Disconnected</b>. Please connect and open Minter App.");
        statusProgress->setVisible(true);
        StyleHelper::get().setIconToLabel(statusIcon, "ic_usb_off");
        break;
    case dev_state::APP_NOT_OPENED:
        balanceLabel->setText("Balance: <b>Unavailable</b>");
        addressLabel->setText("Address: <b>Unavailable</b>");
        statusLabel->setText(tr("Ledger: <b>Connected, but not opened Minter app</b>"));
        statusProgress->setVisible(true);
        StyleHelper::get().setIconToLabel(statusIcon, "ic_usb_off");
        break;

    case dev_state::PERMISSION_ERROR:
        balanceLabel->setText("Balance: <b>Unavailable</b>");
        addressLabel->setText("Address: <b>Unavailable</b>");

        StyleHelper::get().setIconToLabel(statusIcon, "ic_usb_off");
        statusLabel->setText("Ledger: <b>Unable to connect to Ledger.</b> Please check usb permissions or restart console with admin privileges.");
        statusProgress->setVisible(true);
        break;

    case dev_state::APP_OPENED:
    default:
        app->dev.setInfiniteEmitting(false);

        StyleHelper::get().setIconToLabel(statusIcon, "ic_usb_on");
        statusLabel->setText("Ledger: <b>Connected</b>");
        statusProgress->setVisible(false);
    }

    //    std::cout << "Resolved address: " << address.to_string() << std::endl;
    //    std::for_each(tabs.begin(), tabs.end(), [type](const std::shared_ptr<TabBase>& tab) {
    //        tab->setDeviceAvailable(type == dev_state::APP_OPENED);
    //    });
}

void ConsoleWindow::onAppStarted() {
    ui->tabWidget->addTab(new Ui::TabSend(app, nullptr),
                          StyleHelper::get().icon("ic_tx_send"),
                          tr("Send"));

    ui->tabWidget->addTab(new Ui::TabExchange(app, nullptr),
                          StyleHelper::get().icon("ic_tx_exchange"),
                          tr("Swap"));

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
}

void ConsoleWindow::onTabChanged(int) {
}
