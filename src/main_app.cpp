#include "include/main_app.h"

#include "include/settings.h"

#include <QDebug>
#include <iostream>

MainApp::MainApp(QObject* parent)
    : QObject(parent),
      dev(),
      dev_thread() {

    qRegisterMetaType<dev_state>("dev_state");

    dev.moveToThread(&dev_thread);
    // receive handler signals
    connect(&dev, &DeviceLooper::deviceStateChanged, this, &MainApp::onDeviceStateChanged);
    // run handler after thread started
    connect(&dev_thread, SIGNAL(started()), &dev, SLOT(run()));
    // stop thread when handler finished
    connect(&dev, SIGNAL(finished()), &dev_thread, SLOT(quit()));

    dev_thread.start();
}

MainApp::~MainApp() {
    std::cout << "stop dev" << std::endl;
    dev.stop();
    std::cout << "wait thread..." << std::endl;
    dev_thread.quit();
    dev_thread.wait();
    std::cout << "exited from miledger" << std::endl;
}
void MainApp::onDeviceStateChanged(dev_state type) {
    switch (type) {
    case dev_state::DISCONNECTED:
        setProgressHint("Waiting for device...");
        setProgressVisible(true);
        setBtnInstallEnabled(false);
        setBtnWalletEnabled(false);
        qDebug() << "DISCONNECTED";
        break;
    case dev_state::APP_NOT_OPENED:
        dev.setInfiniteEmitting(false);
        setProgressHint("Ledger found. Please accept \"Allow listing applications\" on the Ledger");
        setProgressVisible(false);
        setBtnInstallEnabled(true);
        setBtnWalletEnabled(false);
        qDebug() << "APP_NOT_OPENED";
        emit appNotOpened();
        break;
    case dev_state::PERMISSION_ERROR:
        setProgressHint("Unable to connect to ledger: permission denied. Try run app with sudo.");
        setProgressVisible(true);
        setBtnInstallEnabled(false);
        setBtnWalletEnabled(false);
        qDebug() << "PERMISSION_ERROR";
        emit stopHandler();
        break;
    case dev_state::APP_OPENED:
        setProgressHint("Minter app opened. Now you can open Console.");
        setProgressVisible(false);
        setBtnInstallEnabled(false);
        setBtnWalletEnabled(true);
        qDebug() << "APP_OPENED";
        emit stopHandler();
        break;
    }

    emit progressHintChanged();
    emit progressVisibilityChanged();
    emit btnInstallEnabledChanged();
    emit btnWalletEnabledChanged();

    // if(!dev.isRunning()) {
    //         dev.stop();
    //     }
}
