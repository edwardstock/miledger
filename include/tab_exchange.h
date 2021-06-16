/*!
 * miledger.
 * tab_exchange.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_TAB_EXCHANGE_H
#define MILEDGER_TAB_EXCHANGE_H

#include "coin_model.h"
#include "exchange_calculator.h"
#include "exchange_forms.h"
#include "input_fields.hpp"
#include "input_group.h"
#include "tab_base.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAbstractListModel>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDataWidgetMapper>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QStandardItemModel>
#include <QStringList>
#include <memory>
#include <optional>

namespace Ui {

class TabExchange : public TabBase {
    Q_OBJECT
public:
    TabExchange(miledger::ConsoleApp* app, QWidget* parent);
    ~TabExchange();
    void setDeviceAvailable(bool available) override;

private slots:
    void onBalanceUpdated(minter::explorer::balance_items);
    void onInitDataUpdated(miledger::repo::tx_init_data);
    void onCoinListUpdated(std::vector<minter::explorer::coin_item>);
    void onBuyFormSubmit();
    void onSellFormSubmit();
    void onSellAllFormSubmit();

private:
    enum ExchangeFormBy {
        Buy,
        Sell,
        SellAll
    };

    ExchangeBuyForm* buyForm;
    ExchangeSellForm* sellForm;
    ExchangeSellAllForm* sellAllForm;
    CoinModel coinsModel;

    rxcpp::observable<minter::gate::tx_send_result> createTxSender(ExchangeFormBy exchangeBy);
    std::shared_ptr<minter::tx> createBuyTx(miledger::repo::tx_init_data initData);
    std::shared_ptr<minter::tx> createSellTx(miledger::repo::tx_init_data initData);
    std::shared_ptr<minter::tx> createSellAllTx(miledger::repo::tx_init_data initData);

    void buyDialogConfigure(miledger::TxSendDialog* dialog);
    void buyDialogSubmit();
    void sellDialogConfigure(miledger::TxSendDialog* dialog);
    void sellDialogSubmit();
    void sellAllDialogConfigure(miledger::TxSendDialog* dialog);
    void sellAllDialogSubmit();

    void dialogSubmit(
        rxcpp::observable<minter::gate::tx_send_result> sender,
        Ui::ExchangeForm* form);
};

} // namespace Ui

#endif // MILEDGER_TAB_EXCHANGE_H
