#include "include/mainwindow.h"

#include "include/main_app.h"
#include "include/settings.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenu>
#include <QStyle>
#include <utility>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      app(new MainApp()) {
    ui->setupUi(this);

    connect(app, &MainApp::progressHintChanged, [=]() {
        ui->labelConnecting->setText(app->getProgressHint());
    });
    connect(app, &MainApp::btnInstallEnabledChanged, [=]() {
        ui->btnInstall->setEnabled(app->getBtnInstallEnabled());
    });
    connect(app, &MainApp::btnWalletEnabledChanged, [=]() {
        ui->btnWallet->setEnabled(app->getBtnWalletEnabled());
    });
    connect(app, &MainApp::progressVisibilityChanged, [=]() {
        ui->progressConnecting->setVisible(app->getProgressVisible());
    });

    connect(ui->btnInstall, SIGNAL(clicked()), this, SLOT(onClickedInstall()));
    connect(ui->btnWallet, SIGNAL(clicked()), this, SLOT(onClickedConsole()));

    connect(app, &MainApp::appNotOpened, this, &MainWindow::onAppNotOpened);
}

void MainWindow::onAppNotOpened() {
    ui->labelConnecting->setText("Resolving applications from connected Ledger...");
    app->getDeviceLooper()
        .checkMinterAppInstalled()
        .subscribe_on(RxQt::get().ioThread())
        .observe_on(RxQt::get().uiThread())
        .subscribe(
            [this](bool hasApp) {
                if (!hasApp) {
                    Settings::get().remove(Settings::KEY_MINTER_PID);
                    ui->labelConnecting->setText("Ledger found, but Minter app hasn't been installed.");
                    return;
                }

                auto ret = QMessageBox::question(
                    this,
                    "Open Minter app",
                    "Do you want to open installed Minter app?\nIf yes, please proceed this operation on the Ledger after closing this dialog.",
                    QMessageBox::Open | QMessageBox::Cancel,
                    QMessageBox::Open);

                switch (ret) {
                case QMessageBox::Open:
                    ui->labelConnecting->setText("Opening Minter app... Please accept \"Open app Minter\" on the Ledger");
                    app->getDeviceLooper()
                        .openMinterApp()
                        .subscribe_on(RxQt::get().ioThread())
                        .observe_on(RxQt::get().uiThread())
                        .subscribe(
                            [this](bool) {
                                ui->labelConnecting->setText("Minter app opened. Now you can open Console.");
                            },
                            [this](std::exception_ptr e) {
                                QMessageBox::critical(
                                    this,
                                    "Error",
                                    QString("Unable to open Minter app: %1").arg(miledger::utils::getError(e)),
                                    QMessageBox::Ok);
                            });
                    break;
                case QMessageBox::Cancel:
                default:
                    break;
                }
            },
            [this](std::exception_ptr e) {
                QMessageBox::critical(
                    this,
                    "Error",
                    QString("Unable to get applications list: %1").arg(miledger::utils::getError(e)),
                    QMessageBox::Ok);
            });
}

void MainWindow::onClickedInstall() {
    installWindow = new InstallAppWindow();
    installWindow->show();
    connect(installWindow, &InstallAppWindow::installStarted, [this]() {
        app->getDeviceLooper().setSkipCheck(true);
    });
    connect(installWindow, &InstallAppWindow::installFinished, [this]() {
        app->getDeviceLooper().setSkipCheck(false);
        app->getDeviceLooper().setInfiniteEmitting(true);
    });
    //    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void MainWindow::onClickedConsole() {
    emit openedConsole();
    //    consoleWindow = new ConsoleWindow();
    //    consoleWindow->show();
    //    this->close();
}

void MainWindow::hideEvent(QHideEvent* event) {
    QMainWindow::hideEvent(event);
    delete app;
    app = nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:

        /* otherwise, if the window is visible, it is hidden,
         * Conversely, if hidden, it unfolds on the screen
         * */
        if (!this->isVisible()) {
            this->show();
        } else {
            this->hide();
        }

        break;
    default:
        break;
    }
}

MainWindow::~MainWindow() {
    delete ui;
    delete app;
    //    delete menu;
    //    delete quitAction;
    //    delete viewWindow;

    if (installWindow) {
        installWindow->hide();
        delete installWindow;
    }

    delete openAppDialog;

    if (m_subs.is_subscribed()) {
        m_subs.unsubscribe();
    }
}
