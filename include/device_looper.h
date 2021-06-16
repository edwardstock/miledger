/*!
 * miledger-qt.
 * DevSearchThread.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_QT_DEVICEHANDLER_H
#define MILEDGER_QT_DEVICEHANDLER_H

#include <QDebug>
#include <QThread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <minter/nanos_wallet.h>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-subscriber.hpp>

using dev_state = minter::dev_connection_type;

Q_DECLARE_METATYPE(dev_state)

class DeviceLooper : public QObject {
    Q_OBJECT
    Q_ENUM(dev_state)

public:
    DeviceLooper(QObject* parent = nullptr);

    ~DeviceLooper() {
        stop();
    }

    bool isRunning() const;
    void setInfiniteEmitting(bool state);
    void setSkipCheck(bool state);

    minter::nanos_wallet& getWallet();

    rxcpp::observable<minter::address_t> getAddress(bool silent = true);
    minter::signature signTx(tb::bytes_data txHash, uint32_t deriveIndex = 0);
    rxcpp::observable<bool> checkMinterAppInstalled();
    rxcpp::observable<bool> openMinterApp();

public slots:
    void run();
    void stop();

signals:
    void deviceStateChanged(dev_state type);
    void finished();

private:
    dev_state m_lastState = dev_state::DISCONNECTED;
    minter::nanos_wallet m_wallet;
    std::atomic_bool m_running;
    std::atomic_bool m_skipConnectionCheck;
    std::atomic_bool m_infiniteEmitting;
    std::atomic_bool m_appOpened;
    std::atomic_uint16_t m_appPid;
};

#endif // MILEDGER_QT_DEVICEHANDLER_H
