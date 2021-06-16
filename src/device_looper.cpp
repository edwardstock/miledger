/*!
 * miledger-qt.
 * DevSearchThread.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/device_looper.h"

#include "include/settings.h"

#include <QDebug>
#include <iostream>
#include <rxcpp/rx-observable.hpp>

DeviceLooper::DeviceLooper(QObject* parent)
    : QObject(parent),
      m_wallet(),
      m_running(true),
      m_skipConnectionCheck(false),
      m_infiniteEmitting(true),
      m_appOpened(false),
      m_appPid(0) {
}

void DeviceLooper::run() {
    while (m_running.load()) {
        if (!m_skipConnectionCheck) {
            dev_state state;
            uint16_t appPid = m_wallet.get_opened_app_pid();
            if (Settings::get().has(Settings::KEY_MINTER_PID)) {
                if (appPid == 0) {
                    //                    qDebug() << "Open HID for system pid";
                    state = m_wallet.init_ext_root();
                } else {
                    if (m_lastState == dev_state::APP_NOT_OPENED) {
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
                    if (m_lastState == dev_state::APP_NOT_OPENED) {
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

            if (state != m_lastState || m_infiniteEmitting.load(std::memory_order_relaxed)) {
                m_lastState = state;
                qDebug() << " last dev state: " << m_lastState;
                emit deviceStateChanged(m_lastState);
            }
        } else {
            m_wallet.close();
        }

        if (m_running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } else {
            break;
        }
    }

    emit finished();
    std::cout << "Finishing DeviceLooper loop" << std::endl;
}

void DeviceLooper::stop() {
    std::cout << "Called DeviceLooper::stop" << std::endl;
    m_running = false;
}
rxcpp::observable<minter::address_t> DeviceLooper::getAddress(bool silent) {
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
            } while (m_lastState == minter::APP_OPENED && tries < max_tries);

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
minter::signature DeviceLooper::signTx(tb::bytes_data txHash, uint32_t deriveIndex) {
    return m_wallet.sign_tx(txHash, deriveIndex);
}
rxcpp::observable<bool> DeviceLooper::checkMinterAppInstalled() {
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

rxcpp::observable<bool> DeviceLooper::openMinterApp() {
    return rxcpp::observable<>::create<bool>([this](rxcpp::subscriber<bool> emitter) {
        try {
            auto apps = m_wallet.run_app("Minter");
            emitter.on_next(true);
            emitter.on_completed();
        } catch (const std::exception& e) {
            emitter.on_error(std::make_exception_ptr(e));
        }
    });
}

void DeviceLooper::setInfiniteEmitting(bool state) {
    m_infiniteEmitting = state;
}
void DeviceLooper::setSkipCheck(bool state) {
    m_skipConnectionCheck = state;
}
minter::nanos_wallet& DeviceLooper::getWallet() {
    return m_wallet;
}
bool DeviceLooper::isRunning() const {
    return m_running.load();
}
