#include "include/consolewindow.h"

#include "include/miledger-config.h"
#include "include/net/explorer_repo.h"
#include "include/style_helper.h"
#include "include/tab_base.h"
#include "include/tab_exchange.h"
#include "include/tab_send.h"
#include "include/utils.h"
#include "ui_consolewindow.h"

#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <toolbox/strings.hpp>

using namespace minter;

ConsoleWindow::ConsoleWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::ConsoleWindow),
      app(new miledger::ConsoleApp(this)),
      tabs(),
      statusIcon(new QLabel(this)),
      statusLabel(new QLabel(this)),
      statusProgress(new QProgressBar(this)) {

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

        connect(addressCopy, &QPushButton::clicked, [this](bool) {
            QClipboard* cp = QGuiApplication::clipboard();
            if (cp != nullptr) {
                cp->setText(QString::fromStdString(app->address.to_string()));
            }
        });
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
}

void ConsoleWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    app->dev.setInfiniteEmitting(true);
    app->start();
}

void ConsoleWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
    if (event->isAccepted()) {
        emit closed();
    } else {
        event->accept();
    }
}

void ConsoleWindow::updateBalance() {
    app->updateBalance();
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
