/*!
 * miledger.
 * app.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/app.h"

#include "dev/ledger_device_server.h"
#include "dev/mnemonic_device_server.h"

miledger::App::App() {
}

void miledger::App::setUseMnemonic(bool use) {
    m_useMock = use;
}
void miledger::App::setMnemonic(QString m) {
    m_mockMnemonic = std::move(m);
}
const QString& miledger::App::getMnemonic() const {
    return m_mockMnemonic;
}
bool miledger::App::useMnemonic() const {
    return m_useMock;
}

BaseDeviceServer* miledger::App::createLooper() {
    if (useMnemonic()) {
        return new MnemonicDeviceServer(m_mockMnemonic);
    }

    return new miledger::LedgerDeviceServer();
}
