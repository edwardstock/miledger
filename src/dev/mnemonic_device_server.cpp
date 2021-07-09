/*!
 * miledger.
 * mock_looper.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "mnemonic_device_server.h"

#include "include/rxqt_instance.hpp"

#include <chrono>
#include <minter/crypto/cxx_secp256k1.h>
#include <minter/ledger/errors.h>
#include <rxcpp/rx-observable.hpp>

MnemonicDeviceServer::MnemonicDeviceServer(const QString& mnemonic)
    : m_privateKey(minter::privkey_t::from_mnemonic(mnemonic.toStdString()))
    , m_address(m_privateKey) {
}

void MnemonicDeviceServer::run() {
    while (isRunning()) {
        if (!isSkipCheck()) {
            dev_state state = dev_state::APP_OPENED;

            if (state != getState() || isInfiniteEmitting()) {
                setState(state);
                qDebug() << " last dev state: " << getState();
                emit deviceStateChanged(getState());
            }
        }

        if (isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            break;
        }
    }
}
void MnemonicDeviceServer::stop() {
    std::cout << "Called DeviceServer::stop" << std::endl;
    setIsRunning(false);
}
rxcpp::observable<minter::address_t> MnemonicDeviceServer::getAddress(bool) {
    return rxcpp::observable<>::just<minter::address_t>(m_address)
        .delay(std::chrono::seconds(1));
}

minter::signature MnemonicDeviceServer::signTx(tb::bytes_data txHash, uint32_t) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    minter::cxx_secp256k1 secp;
    auto sig = secp.sign_message(txHash, m_privateKey);
    if (!sig.success) {
        throw minter::exchange_error(CODE_NO_STATUS_RESULT);
    }

    return sig;
}
rxcpp::observable<bool> MnemonicDeviceServer::checkMinterAppInstalled() {
    return rxcpp::observable<>::just<bool>(true).delay(std::chrono::seconds(1));
}
rxcpp::observable<bool> MnemonicDeviceServer::openMinterApp() {
    return rxcpp::observable<>::just<bool>(true).delay(std::chrono::seconds(1));
}
