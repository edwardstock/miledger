/*!
 * miledger.
 * console_app.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_CONSOLE_APP_H
#define MILEDGER_CONSOLE_APP_H

#include "device_looper.h"
#include "net/explorer_repo.h"
#include "net/gate_repo.h"
#include "optional.hpp"
#include "rxqt_instance.hpp"

#include <QIcon>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QThread>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

Q_DECLARE_METATYPE(minter::explorer::coin_item)
Q_DECLARE_METATYPE(minter::explorer::coin_item_base)

namespace miledger {

class ConsoleApp : public QObject {
    Q_OBJECT

signals:
    void deviceStateChanged(dev_state state);
    void started();
    void addressResolved(const minter::address_t& address);
    void balanceUpdated(minter::explorer::balance_items balances);
    void initDataUpdated(miledger::repo::tx_init_data initData);
    void coinListUpdated(std::vector<minter::explorer::coin_item> coins);
    void deviceExchangeError(std::exception_ptr e);

private slots:
    void onDeviceStateChanged(dev_state state);

public:
    ConsoleApp(QObject* parent);

    ~ConsoleApp();

    void start();

    rxcpp::observable<miledger::repo::tx_init_data> getInitDataUpdater();
    void updateInitData();
    void updateCoinList();
    void updateBalance();

    const std::vector<minter::explorer::coin_item>& getCoins() const;
    const QHash<QString, minter::explorer::coin_item*>& getCoinsIndex() const;
    optns::optional<minter::explorer::coin_item*> findCoinBySymbol(const QString& symbol) const;
    optns::optional<minter::explorer::coin_item*> findCoinBySymbol(const std::string& symbol) const;

    bool hasCoinIcon(const QString& idString);
    bool hasCoinIcon(const dev::bigint& id);
    QIcon getCoinIcon(const QString& idString);
    QIcon getCoinIcon(const dev::bigint& id);

    QThread devThread;
    DeviceLooper dev;

    minter::explorer::balance_items balances;
    miledger::repo::tx_init_data initData;
    std::vector<minter::explorer::coin_item> coins;
    QHash<QString, minter::explorer::coin_item*> coinsIndex;
    rxcpp::composite_subscription subs;

    miledger::repo::explorer_repo explorerRepo;
    miledger::repo::gate_repo gateRepo;
    minter::address_t address;

private:
    bool isStarted = false;
    mutable std::mutex mCoinsLock;
    const std::vector<dev::bigint> mTopCoinsIds{
        dev::bigint("0"),    // bip
        dev::bigint("2024"), // musd
        dev::bigint("2064"), // btc
        dev::bigint("1902"), // hub
        dev::bigint("1993"), // usdte
        dev::bigint("1994"), // usdte
        dev::bigint("2065"), // eth
    };

    const std::unordered_map<QString, QString> mTopCoinsIcons{
        {QString("2024"), ":/icons/ic_logo_musd.png"}, // musd
        {QString("2064"), ":/icons/ic_logo_btc.png"},  // btc
        {QString("1902"), ":/icons/ic_logo_hub.png"},  // hub
        {QString("1993"), ":/icons/ic_logo_usdt.png"}, // usdte
        {QString("1994"), ":/icons/ic_logo_usdc.png"}, // usdce
        {QString("2065"), ":/icons/ic_logo_eth.png"}   // eth
    };
};

} // namespace miledger

#endif // MILEDGER_CONSOLE_APP_H
