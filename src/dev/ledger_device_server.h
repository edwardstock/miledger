/*!
 * miledger.
 * ledger_looper.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_LEDGER_DEVICE_SERVER_H
#define MILEDGER_LEDGER_DEVICE_SERVER_H

#include "include/device_server.h"

namespace miledger {

class LedgerDeviceServer : public BaseDeviceServer {
    Q_OBJECT
    // signals:
    //     void deviceStateChanged(dev_state type);
    //     void finished();

public:
    LedgerDeviceServer();
    ~LedgerDeviceServer() override = default;
    rxcpp::observable<minter::address_t> getAddress(bool silent) override;
    minter::signature signTx(tb::bytes_data txHash, uint32_t deriveIndex) override;
    rxcpp::observable<bool> checkMinterAppInstalled() override;
    rxcpp::observable<bool> openMinterApp() override;
    void run() override;
    void stop() override;

private:
    minter::nanos_wallet m_wallet;
    std::atomic<uint16_t> m_appPid;
};

} // namespace miledger

#endif // MILEDGER_LEDGER_DEVICE_SERVER_H
