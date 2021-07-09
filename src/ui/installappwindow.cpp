#include "include/ui/installappwindow.h"

#include "include/miledger-config.h"
#include "ui_installappwindow.h"

#include <QDir>
#include <QStringList>
#include <chrono>
#include <thread>

InstallAppWindow::InstallAppWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::InstallAppWindow) {
    ui->setupUi(this);

    connect(ui->btnInstall, SIGNAL(clicked()), this, SLOT(onClickedInstall()));
}

void InstallAppWindow::onClickedInstall() {

    ui->installLog->append("Initializing installation...");

    QString baseCommand(CMD_BIN);
    QString baseCommandArg(CMD_BIN_ARG);
    QStringList args;

    if (!baseCommandArg.isEmpty()) {
        args << baseCommandArg;
    }

    emit installStarted();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    buildInstallArgs(args);

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(baseCommand, args);
    process->waitForStarted();

    ui->progressInstall->setMaximum(0);
    ui->progressInstall->setMinimum(0);
    ui->installLog->append("Please follow instructions on device\n\n");

    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(writeOutput()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));
}

void InstallAppWindow::onProcessFinished(int, QProcess::ExitStatus status) {
    ui->installLog->append("\n\n");
    if (status == QProcess::ExitStatus::NormalExit) {
        ui->progressInstall->setMaximum(0);
        ui->progressInstall->setMinimum(1);
        ui->progressInstall->setValue(1);
        ui->btnInstall->setEnabled(false);
        ui->installLog->append("Installation completed.\nNow you can open Minter application on ledger to continue.");
    } else {
        ui->installLog->append("Installation failed!\nPlease report log to issues of repo: https://github.com/edwardstock/miledger/issues");
    }
    emit installFinished();
}

void InstallAppWindow::buildInstallArgs(QStringList& args) {
    args << QCoreApplication::applicationDirPath() + QDir::separator() + QString(LEDGER_APP_INSTALLER);
}

void InstallAppWindow::writeOutput() {
    Q_ASSERT(process != nullptr);
    ui->installLog->append(process->readAllStandardOutput());
    ui->installLog->append(process->readAllStandardError());
}

InstallAppWindow::~InstallAppWindow() {
    delete ui;
    delete process;
}
