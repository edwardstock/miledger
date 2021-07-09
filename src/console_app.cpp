/*!
 * miledger.
 * console_app.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/console_app.h"

#include "include/app.h"
#include "include/errors.h"
#include "include/image_cache.h"
#include "include/utils.h"

#include <QDir>
#include <QIcon>
#include <QSize>
#include <cpr/cpr.h>
#include <fstream>

miledger::ConsoleApp::ConsoleApp(QObject* parent)
    : QObject(parent)
    , devThread()
    , dev(miledger::App::get().createLooper())
    , explorerRepo()
    , gateRepo() {

    dev.moveToThread(&devThread);

    connect(this, &ConsoleApp::addressResolved, [this](const minter::address_t&) {
        updateBalance();
        updateInitData();
        updateCoinList();
    });
}

miledger::ConsoleApp::~ConsoleApp() {
    if (subs.is_subscribed()) {
        subs.unsubscribe();
    }
    isStarted = false;
    std::cout << "stop dev" << std::endl;
    dev.stop();
    std::cout << "wait thread..." << std::endl;
    devThread.quit();
    devThread.wait();
    std::cout << "exited from miledger" << std::endl;
}
void miledger::ConsoleApp::start() {
    // receive handler signals
    connect(&dev, SIGNAL(deviceStateChanged(dev_state)), this, SLOT(onDeviceStateChanged(dev_state)));
    // run handler after thread started
    connect(&devThread, SIGNAL(started()), &dev, SLOT(run()));
    // stop thread when handler finished
    connect(&dev, SIGNAL(finished()), &devThread, SLOT(quit()));

    devThread.start();
    isStarted = true;
    emit started();
}

void miledger::ConsoleApp::onDeviceStateChanged(dev_state state) {
    if (state == dev_state::APP_OPENED) {
        dev.getAddress()
            .subscribe_on(RxQt::get().ioThread())
            .observe_on(RxQt::get().uiThread())
            .subscribe(
                [this](minter::address_t add) {
                    address = add;
                    emit addressResolved(address);
                },
                [this](std::exception_ptr e) {
                    emit deviceExchangeError(e);
                });
    }
    // re-emit state to owner
    emit deviceStateChanged(state);
}

rxcpp::observable<miledger::repo::tx_init_data> miledger::ConsoleApp::getInitDataUpdater() {
    if (!address) {
        return rxcpp::observable<>::error<miledger::repo::tx_init_data>(std::runtime_error("Empty address"));
    }
    return gateRepo.get_tx_init_data(address);
}
void miledger::ConsoleApp::updateInitData() {
    auto sub = getInitDataUpdater()
                   .subscribe_on(RxQt::get().ioThread())
                   .observe_on(RxQt::get().uiThread())
                   .subscribe(
                       [this](miledger::repo::tx_init_data result) {
                           initData = result;
                           emit initDataUpdated(initData);
                       },
                       [](const std::exception_ptr& e) {
                           qDebug() << miledger::utils::getError(e);
                       });

    subs.add(sub);
}
void miledger::ConsoleApp::updateCoinList() {
    auto sub = explorerRepo.get_coins()
                   .subscribe_on(RxQt::get().ioThread())
                   .observe_on(RxQt::get().uiThread())
                   .subscribe(
                       [this](minter::explorer::result<std::vector<minter::explorer::coin_item>> result) {
                           if (result.error.message.empty()) {
                               {
                                   std::lock_guard<std::mutex> lock(mCoinsLock);

                                   auto tmp = std::move(result.data);
                                   std::sort(tmp.begin(), tmp.end(), [this](const minter::explorer::coin_item& left, const minter::explorer::coin_item& right) {
                                       for (const auto& top_id : mTopCoinsIds) {
                                           if (left.id == top_id) {
                                               return true;
                                           }
                                           if (right.id == top_id) {
                                               return false;
                                           }
                                       }

                                       return left.reserve_valance > right.reserve_valance;
                                   });
                                   coins = tmp;
                                   for (size_t i = 0; i < coins.size(); i++) {
                                       coinsIndex.insert(QString::fromStdString(coins.at(i).symbol).toUpper(), &coins.at(i));
                                   }
                               }
                               emit coinListUpdated(coins);
                           } else {
                               qDebug() << "[" << result.error.code << "] " << QString::fromStdString(result.error.message);
                           }
                       },
                       [](const std::exception_ptr& e) {
                           qDebug() << miledger::utils::getError(e);
                       });
    subs.add(sub);
}
void miledger::ConsoleApp::updateBalance() {
    if (!address) {
        return;
    }

    auto sub = explorerRepo.get_balance(address)
                   .subscribe_on(RxQt::get().ioThread())
                   .observe_on(RxQt::get().uiThread())
                   .subscribe(
                       [this](minter::explorer::result<minter::explorer::balance_items> result) {
                           if (result.error.message.empty()) {
                               balances = result.data;
                               qDebug() << "Update balance";
                               emit balanceUpdated(balances);
                           } else {
                               qDebug() << "[" << result.error.code << "] " << QString::fromStdString(result.error.message);
                           }
                       },
                       [](const std::exception_ptr& e) {
                           qDebug() << miledger::utils::getError(e);
                       });
    subs.add(sub);
}
const std::vector<minter::explorer::coin_item>& miledger::ConsoleApp::getCoins() const {
    std::lock_guard<std::mutex> lock(mCoinsLock);
    return coins;
}
const QHash<QString, minter::explorer::coin_item*>& miledger::ConsoleApp::getCoinsIndex() const {
    std::lock_guard<std::mutex> lock(mCoinsLock);
    return coinsIndex;
}
optns::optional<minter::explorer::coin_item*> miledger::ConsoleApp::findCoinBySymbol(const QString& symbol) const {
    std::lock_guard<std::mutex> lock(mCoinsLock);
    if (coinsIndex.contains(symbol.toUpper())) {
        return coinsIndex.value(symbol.toUpper());
    }
    return {};
}
optns::optional<minter::explorer::coin_item*> miledger::ConsoleApp::findCoinBySymbol(const std::string& symbol) const {
    return findCoinBySymbol(QString::fromStdString(symbol));
}
bool miledger::ConsoleApp::hasCoinIcon(const QString& idString) {
    return mTopCoinsIcons.count(idString);
}
bool miledger::ConsoleApp::hasCoinIcon(const dev::bigint& id) {
    return hasCoinIcon(QString::fromStdString(minter::utils::to_string(id)));
}
QIcon miledger::ConsoleApp::getCoinIcon(const QString& idString) {
    return QIcon(mTopCoinsIcons.at(idString));
}
QIcon miledger::ConsoleApp::getCoinIcon(const dev::bigint& id) {
    return getCoinIcon(QString::fromStdString(minter::utils::to_string(id)));
}
