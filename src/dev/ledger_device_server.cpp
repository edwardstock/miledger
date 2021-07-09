/*!
 * miledger.
 * ledger_looper.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "ledger_device_server.h"

#include "include/settings.h"

miledger::LedgerDeviceServer::LedgerDeviceServer(): m_wallet(), m_appPid(0) {
}

rxcpp::observable<minter::address_t> miledger::LedgerDeviceServer::getAddress(bool silent) {
    return rxcpp::observable<>::create<minter::address_t>([this, silent](rxcpp::subscriber<minter::address_t> emitter) {
        try {
            unsigned tries = 0;
            unsigned max_tries = 3;
            do {
                tries++;
                try {
                    auto address = m_wallet.get_address(0, silent);
                    // cache app pid only if it's really minter app. we know it because our method to get address is successful
                    if (!Settings::get().has(Settings::KEY_MINTER_PID)) {
                        Settings::get().set(Settings::KEY_MINTER_PID, m_wallet.get_opened_app_pid());
                    }
                    emitter.on_next(address);
                    emitter.on_completed();
                } catch (const std::out_of_range& e) {
                    qDebug() << "Unable to get address: possibly due inconsistent buffer. Try again";
                }
            } while (getState() == minter::APP_OPENED && tries < max_tries);

            if (tries == max_tries) {
                emitter.on_error(std::make_exception_ptr(
                    std::runtime_error("Unable to get address: possibly due inconsistent buffer. Please try to disconnect Ledger and connect it back")));
                return;
            }
        } catch (...) {
            emitter.on_error(std::current_exception());
        }
    });
}
minter::signature miledger::LedgerDeviceServer::signTx(tb::bytes_data txHash, uint32_t deriveIndex) {
    return m_wallet.sign_tx(txHash, deriveIndex);
}
rxcpp::observable<bool> miledger::LedgerDeviceServer::checkMinterAppInstalled() {
    return rxcpp::observable<>::create<bool>([this](rxcpp::subscriber<bool> emitter) {
        try {
            auto apps = m_wallet.get_app_list();
            for (const auto& app : apps) {
                if (app.name == "Minter") {
                    emitter.on_next(true);
                    emitter.on_completed();
                    return;
                }
            }
            emitter.on_next(false);
            emitter.on_completed();
        } catch (const std::exception& e) {
            emitter.on_error(std::make_exception_ptr(std::runtime_error(e.what())));
        }
    });
}
rxcpp::observable<bool> miledger::LedgerDeviceServer::openMinterApp() {
    return rxcpp::observable<>::create<bool>([this](rxcpp::subscriber<bool> emitter) {
        try {
            auto appPid = m_wallet.run_app("Minter");
            if (appPid != 0) {
                Settings::get().set(Settings::KEY_MINTER_PID, appPid);
            }

            emitter.on_next(true);
            emitter.on_completed();
        } catch (const std::exception& e) {
            emitter.on_error(std::make_exception_ptr(e));
        }
    });
}
void miledger::LedgerDeviceServer::run() {
    while (isRunning()) {
        if (!isSkipCheck()) {
            dev_state state;
            uint16_t appPid = m_wallet.get_opened_app_pid();
            if (Settings::get().has(Settings::KEY_MINTER_PID)) {
                if (appPid == 0) {
                    //                    qDebug() << "Open HID for system pid";
                    state = m_wallet.init_ext_root();
                } else {
                    if (getState() == dev_state::APP_NOT_OPENED) {
                        //                        qDebug() << "ReOpen HID for pid " << Settings::get().getUint16(Settings::KEY_MINTER_PID);
                        state = m_wallet.reopen_with_app(Settings::get().getUint16(Settings::KEY_MINTER_PID));
                    } else {
                        //                        qDebug() << "Open HID for pid " << Settings::get().getUint16(Settings::KEY_MINTER_PID);
                        state = m_wallet.init_ext(Settings::get().getUint16(Settings::KEY_MINTER_PID));
                    }
                }
            } else {

                if (appPid != 0) {
                    state = m_wallet.reopen_with_app(appPid);
                    if (getState() == dev_state::APP_NOT_OPENED) {
                        //                        qDebug() << "ReOpen HID for pid " << appPid;
                        state = m_wallet.reopen_with_app(Settings::get().getUint16(Settings::KEY_MINTER_PID));
                    } else {
                        //                        qDebug() << "Open HID for pid " << appPid;
                        state = m_wallet.init_ext(Settings::get().getUint16(Settings::KEY_MINTER_PID));
                    }
                } else {
                    //                    qDebug() << "Open HID for system pid";
                    state = m_wallet.init_ext_root();
                }
            }

            if (state != getState() || isInfiniteEmitting()) {
                setState(state);
                qDebug() << " last dev state: " << getState();
                emit deviceStateChanged(getState());
            }
        } else {
            m_wallet.close();
        }

        if (isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            break;
        }
    }

    emit finished();
    std::cout << "Finishing DeviceServer loop" << std::endl;
}
void miledger::LedgerDeviceServer::stop() {
    std::cout << "Called DeviceServer::stop" << std::endl;
    setIsRunning(false);
}
