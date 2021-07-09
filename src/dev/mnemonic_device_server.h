/*!
 * miledger.
 * mock_looper.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_MNEMONIC_DEVICE_SERVER_H
#define MILEDGER_MNEMONIC_DEVICE_SERVER_H

#include "include/device_server.h"

#include <minter/address.h>
#include <minter/private_key.h>
#include <minter/tx/signature.h>

class MnemonicDeviceServer : public BaseDeviceServer {
    Q_OBJECT
public slots:
    void run() override;
    void stop() override;

public:
    MnemonicDeviceServer(const QString& mnemonic);
    ~MnemonicDeviceServer() override = default;
    rxcpp::observable<minter::address_t> getAddress(bool silent) override;
    minter::signature signTx(tb::bytes_data txHash, uint32_t deriveIndex) override;
    rxcpp::observable<bool> checkMinterAppInstalled() override;
    rxcpp::observable<bool> openMinterApp() override;

private:
    minter::privkey_t m_privateKey;
    minter::address_t m_address;
};

#endif // MILEDGER_MNEMONIC_DEVICE_SERVER_H
