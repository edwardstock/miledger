/*!
 * miledger.
 * device_looper.h
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
#include <minter/ledger/nanos_wallet.h>
#include <rxcpp/rx-observable.hpp>
#include <rxcpp/rx-subscriber.hpp>
#include <unordered_map>

using dev_state = minter::dev_connection_type;

Q_DECLARE_METATYPE(dev_state)

class DeviceServer;

class BaseDeviceServer : public QObject {
    friend class DeviceServer;

    Q_OBJECT
    Q_ENUM(dev_state)

signals:
    void deviceStateChanged(dev_state type);
    void finished();

public slots:
    virtual void run() = 0;
    virtual void stop() = 0;

public:
    static const std::unordered_map<dev_state, std::string> state_string_map;
    static std::string stateToString(dev_state state);

    BaseDeviceServer(QObject* parent = nullptr);
    virtual ~BaseDeviceServer() = default;

    virtual rxcpp::observable<minter::address_t> getAddress(bool silent = true) = 0;
    virtual minter::signature signTx(tb::bytes_data txHash, uint32_t deriveIndex = 0) = 0;
    virtual rxcpp::observable<bool> checkMinterAppInstalled() = 0;
    virtual rxcpp::observable<bool> openMinterApp() = 0;

    virtual bool isRunning() const;
    virtual void setInfiniteEmitting(bool state);
    virtual bool isInfiniteEmitting() const;
    virtual bool canInteract() const;
    virtual dev_state getState() const;
    virtual std::string getStateString() const;
    virtual void setSkipCheck(bool skip);
    virtual bool isSkipCheck() const;

protected:
    virtual void setState(dev_state);
    virtual void setIsRunning(bool running);

private:
    dev_state lastState = dev_state::DISCONNECTED;
    std::atomic_bool running;
    std::atomic_bool skipConnectionCheck;
    std::atomic_bool infiniteEmitting;
};

class DeviceServer : public BaseDeviceServer {
    Q_OBJECT
public:
    DeviceServer(BaseDeviceServer* impl, QObject* parent = nullptr);
    ~DeviceServer();

    rxcpp::observable<minter::address_t> getAddress(bool silent = true) override;
    minter::signature signTx(tb::bytes_data txHash, uint32_t deriveIndex = 0) override;
    rxcpp::observable<bool> checkMinterAppInstalled() override;
    rxcpp::observable<bool> openMinterApp() override;

    bool isRunning() const override;
    void setInfiniteEmitting(bool state) override;
    bool isInfiniteEmitting() const override;
    bool canInteract() const override;
    dev_state getState() const override;
    std::string getStateString() const override;
    void setSkipCheck(bool skip) override;
    bool isSkipCheck() const override;

protected:
    void setState(dev_state state) override;
    void setIsRunning(bool running) override;

public slots:
    void run() override;
    void stop() override;

private:
    BaseDeviceServer* m_looperImpl;
};

#endif // MILEDGER_QT_DEVICEHANDLER_H
