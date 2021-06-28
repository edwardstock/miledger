/*!
 * miledger.
 * tab_exchange.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/tab_exchange.h"

#include "include/validators.hpp"

#include <functional>
#include <minter/tx/tx.h>
#include <minter/tx/tx_builder.h>
#include <minter/tx/tx_buy_coin.h>
#include <minter/tx/tx_buy_swap_pool.h>
#include <minter/tx/tx_sell_all_coins.h>
#include <minter/tx/tx_sell_all_swap_pool.h>
#include <minter/tx/tx_sell_coin.h>
#include <minter/tx/tx_sell_swap_pool.h>

using namespace minter;

Ui::TabExchange::TabExchange(miledger::ConsoleApp* app, QWidget* parent)
    : TabBase(app, parent) {

    QTabWidget* subTabs = new QTabWidget();
    buyForm = new Ui::ExchangeBuyForm(app, coinsModel, this);
    buyForm->initViews();
    subTabs->addTab(buyForm->getWidget(), tr("Buy"));

    sellForm = new Ui::ExchangeSellForm(app, coinsModel, this);
    sellForm->initViews();
    subTabs->addTab(sellForm->getWidget(), tr("Sell"));

    sellAllForm = new Ui::ExchangeSellAllForm(app, coinsModel, this);
    sellAllForm->initViews();
    subTabs->addTab(sellAllForm->getWidget(), tr("Sell All"));

    getLayout()->addWidget(subTabs, 0, 0, 1, 1);

    connect(app, SIGNAL(balanceUpdated(minter::explorer::balance_items)), this, SLOT(onBalanceUpdated(minter::explorer::balance_items)));
    connect(app, SIGNAL(initDataUpdated(miledger::repo::tx_init_data)), this, SLOT(onInitDataUpdated(miledger::repo::tx_init_data)));
    connect(app, SIGNAL(coinListUpdated(std::vector<minter::explorer::coin_item>)), this, SLOT(onCoinListUpdated(std::vector<minter::explorer::coin_item>)));

    connect(buyForm, &Ui::ExchangeBuyForm::submitClicked, this, &Ui::TabExchange::onBuyFormSubmit);
    connect(sellForm, &Ui::ExchangeSellForm::submitClicked, this, &Ui::TabExchange::onSellFormSubmit);
    connect(sellAllForm, &Ui::ExchangeSellAllForm::submitClicked, this, &Ui::TabExchange::onSellAllFormSubmit);
}

Ui::TabExchange::~TabExchange() {
}

void Ui::TabExchange::onBalanceUpdated(explorer::balance_items result) {
    //    result.balances
    buyForm->setBalance(result.balances);
    sellForm->setBalance(result.balances);
    sellAllForm->setBalance(result.balances);
}
void Ui::TabExchange::onInitDataUpdated(miledger::repo::tx_init_data) {
}

void Ui::TabExchange::onCoinListUpdated(std::vector<explorer::coin_item>) {
    // buyCoinToBuy
    qDebug() << "Coins list loaded";

    coinsModel.setItems(app->getCoins());
}

void Ui::TabExchange::setDeviceAvailable(bool) {
}

std::shared_ptr<minter::tx> createBaseTx(
    miledger::repo::tx_init_data initData,
    dev::bigint gasCoin,
    std::function<std::shared_ptr<minter::tx>(std::shared_ptr<minter::tx_builder>)> buildTx) {

    auto txBuilder = minter::new_tx();
    txBuilder->set_gas_price(initData.gas);
    txBuilder->set_gas_coin_id(gasCoin);
    txBuilder->set_chain_id(MINTER_CHAIN_ID);
    txBuilder->set_nonce(initData.nonce);

    return buildTx(txBuilder);
}

std::shared_ptr<minter::tx> Ui::TabExchange::createBuyTx(miledger::repo::tx_init_data initData) {
    return createBaseTx(initData, buyForm->gasCoin, [this, initData](std::shared_ptr<minter::tx_builder> txBuilder) {
        if (buyForm->estimateResult.exchangeViaPools()) {
            auto dataBuilder = txBuilder->tx_buy_swap_pool();
            for (const auto& coin : buyForm->estimateResult.route.coins) {
                dataBuilder->add_coin(coin.id);
            }
            dataBuilder->set_max_value_to_sell(buyForm->maxValueToSell);
            dataBuilder->set_value_to_buy(buyForm->amount);
            return dataBuilder->build();
        } else {
            auto dataBuilder = txBuilder->tx_buy_coin();
            dataBuilder->set_coin_id_to_buy(buyForm->coinToBuy.id);
            dataBuilder->set_coin_id_to_sell(buyForm->coinToSell.id);
            dataBuilder->set_max_value_to_sell(buyForm->maxValueToSell);
            dataBuilder->set_value_to_buy(buyForm->amount);
            return dataBuilder->build();
        }
    });
}

std::shared_ptr<minter::tx> Ui::TabExchange::createSellTx(miledger::repo::tx_init_data initData) {
    return createBaseTx(initData, sellForm->gasCoin, [this, initData](std::shared_ptr<minter::tx_builder> txBuilder) {
        if (sellForm->estimateResult.exchangeViaPools()) {
            auto dataBuilder = txBuilder->tx_sell_swap_pool();
            for (const auto& coin : sellForm->estimateResult.route.coins) {
                dataBuilder->add_coin(coin.id);
            }
            dataBuilder->set_min_value_to_buy(sellForm->minValueToBuy);
            dataBuilder->set_value_to_sell(sellForm->amount);
            return dataBuilder->build();
        } else {
            auto dataBuilder = txBuilder->tx_sell_coin();
            dataBuilder->set_coin_id_to_buy(sellForm->coinToBuy.id);
            dataBuilder->set_coin_id_to_sell(sellForm->coinToSell.id);
            dataBuilder->set_min_value_to_buy(sellForm->minValueToBuy);
            dataBuilder->set_value_to_sell(sellForm->amount);
            return dataBuilder->build();
        }
    });
}

std::shared_ptr<minter::tx> Ui::TabExchange::createSellAllTx(miledger::repo::tx_init_data initData) {
    return createBaseTx(initData, sellAllForm->gasCoin, [this, initData](std::shared_ptr<minter::tx_builder> txBuilder) {
        if (sellAllForm->estimateResult.exchangeViaPools()) {
            auto dataBuilder = txBuilder->tx_sell_all_swap_pool();
            for (const auto& coin : sellAllForm->estimateResult.route.coins) {
                dataBuilder->add_coin(coin.id);
            }
            dataBuilder->set_min_value_to_buy(sellAllForm->minValueToBuy);
            return dataBuilder->build();
        } else {
            auto dataBuilder = txBuilder->tx_sell_all_coins();
            dataBuilder->set_coin_id_to_buy(sellAllForm->coinToBuy.id);
            dataBuilder->set_coin_id_to_sell(sellAllForm->coinToSell.id);
            dataBuilder->set_min_value_to_buy(sellAllForm->minValueToBuy);
            return dataBuilder->build();
        }
    });
}

rxcpp::observable<minter::gate::tx_send_result> Ui::TabExchange::createTxSender(ExchangeFormBy exchangeBy) {
    return rxcpp::observable<>::create<gate::tx_send_result>([this, exchangeBy](rxcpp::subscriber<gate::tx_send_result> emitter) {
        app->getInitDataUpdater()
            .subscribe(
                [this, &emitter, exchangeBy](miledger::repo::tx_init_data initData) {
                    std::shared_ptr<minter::tx> tx;
                    dev::bytes_32 rawTx;
                    switch (exchangeBy) {
                    case Buy:
                        tx = createBuyTx(initData);
                        break;
                    case Sell:
                        tx = createSellTx(initData);
                        break;
                    case SellAll:
                        tx = createSellAllTx(initData);
                        break;
                    }
                    rawTx = tx->get_unsigned_hash();

                    minter::signature signature;
                    try {
                        emit progressLabelChanged(QString("Please compare transaction hash and approve it: \n%1").arg(QString::fromStdString(rawTx.to_hex())));
                        signature = app->dev.signTx(rawTx);
                        emit progressLabelChanged("Sending transaction...");
                    } catch (const std::exception& e) {
                        emitter.on_error(std::make_exception_ptr(std::runtime_error(e.what())));
                        return;
                    }

                    auto signedTx = tx->sign_single_external(signature);

                    app->gateRepo.send_tx(signedTx)
                        .subscribe(
                            [&emitter](minter::gate::tx_send_result sendResult) {
                                emitter.on_next(sendResult);
                                emitter.on_completed();
                            },
                            [&emitter](std::exception_ptr eptr) {
                                emitter.on_error(eptr);
                            });
                },
                [&emitter](std::exception_ptr eptr) {
                    emitter.on_error(eptr);
                });
    });
}

void Ui::TabExchange::dialogSubmit(rxcpp::observable<minter::gate::tx_send_result> sender, Ui::ExchangeForm* form) {
    showProgressDialog("Sending transaction...");

    sender
        .subscribe_on(RxQt::get().ioThread())
        .observe_on(RxQt::get().uiThread())
        .subscribe(
            [this, form](gate::tx_send_result result) {
                hideProgressDialog();

                if (result.is_ok()) {
                    form->reset();
                    qDebug() << "Tx sent successfully: " << result.hash;
                    showResultDialog("Transaction has been sent");
                } else {
                    qDebug() << "Error while send tx: [" << result.error.code << "]" << result.error.message;
                    showResultDialog(
                        "Unable to send transaction",
                        QString("Error [%1]: %2").arg(QString::number(result.error.code), QString::fromStdString(result.error.message)));
                }

                app->updateBalance();
                app->updateInitData();
            },
            [this](std::exception_ptr err) {
                hideProgressDialog();

                showResultDialog("Unable to send transaction", miledger::utils::getError(err));
            });
}

void Ui::TabExchange::buyDialogConfigure(miledger::TxSendDialog* dialog) {
    dialog->addFieldAmount(tr("You buy"), buyForm->inputGroup.getInputData(buyForm->inputAmount), buyForm->coinToBuy);
    dialog->addFieldAmount(tr("You will pay approximately"), miledger::utils::humanDecimal(buyForm->estimateResult.amount), buyForm->coinToSell);

    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(buyForm->coinToBuy.symbol)),
        buyForm->estimateResult.formatTargetRate(buyForm->inputGroup.getInputData(buyForm->inputAmount), buyForm->coinToSell));
    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(buyForm->coinToSell.symbol)),
        buyForm->estimateResult.formatSourceRate(buyForm->inputGroup.getInputData(buyForm->inputAmount), buyForm->coinToBuy));

    dialog->addField(tr("Swap type"), buyForm->estimateResult.formatSwapType());

    if (buyForm->estimateResult.exchangeViaPools()) {
        dialog->addField(
            tr("Fee"),
            app->initData.calc_fee_swap_text(
                minter::tx_type_val::buy_swap_pool,
                buyForm->estimateResult.getPoolsExtraCoinsSize()));
    } else {
        dialog->addField(tr("Fee"), app->initData.calc_fee_swap_text(minter::tx_type_val::buy_coin));
    }
}
void Ui::TabExchange::buyDialogSubmit() {
    dialogSubmit(createTxSender(Buy), buyForm);
}
void Ui::TabExchange::sellDialogConfigure(miledger::TxSendDialog* dialog) {
    dialog->addFieldAmount(tr("You will spend"), sellForm->inputGroup.getInputData(sellForm->inputAmount), sellForm->coinToSell);
    dialog->addFieldAmount(tr("You will get approximately"), miledger::utils::humanDecimal(sellForm->estimateResult.amount), sellForm->coinToBuy);

    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(sellForm->coinToSell.symbol)),
        sellForm->estimateResult.formatSourceRate(sellForm->inputGroup.getInputData(sellForm->inputAmount), sellForm->coinToBuy));

    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(sellForm->coinToBuy.symbol)),
        sellForm->estimateResult.formatTargetRate(sellForm->inputGroup.getInputData(sellForm->inputAmount), sellForm->coinToSell));

    dialog->addField(tr("Swap type"), sellForm->estimateResult.formatSwapType());

    if (sellForm->estimateResult.exchangeViaPools()) {
        dialog->addField(
            tr("Fee"),
            app->initData.calc_fee_swap_text(
                minter::tx_type_val::sell_swap_pool,
                sellForm->estimateResult.getPoolsExtraCoinsSize()));
    } else {
        dialog->addField(tr("Fee"), app->initData.calc_fee_swap_text(minter::tx_type_val::sell_coin));
    }
}
void Ui::TabExchange::sellDialogSubmit() {
    dialogSubmit(createTxSender(Sell), sellForm);
}
void Ui::TabExchange::sellAllDialogConfigure(miledger::TxSendDialog* dialog) {
    dialog->addFieldAmount(tr("You will spend"), miledger::utils::humanDecimal(sellAllForm->amount), sellAllForm->coinToSell);
    dialog->addFieldAmount(tr("You will get approximately"), miledger::utils::humanDecimal(sellAllForm->estimateResult.amount), sellAllForm->coinToBuy);

    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(sellAllForm->coinToSell.symbol)),
        sellAllForm->estimateResult.formatSourceRate(QString::fromStdString(sellAllForm->amount.format(".18f")), sellAllForm->coinToBuy));

    dialog->addField(
        QString("1 %1 rate").arg(QString::fromStdString(sellAllForm->coinToBuy.symbol)),
        sellAllForm->estimateResult.formatTargetRate(QString::fromStdString(sellAllForm->amount.format(".18f")), sellAllForm->coinToSell));

    dialog->addField(tr("Swap type"), sellAllForm->estimateResult.formatSwapType());

    if (sellAllForm->estimateResult.exchangeViaPools()) {
        dialog->addField(
            tr("Fee"),
            app->initData.calc_fee_swap_text(
                minter::tx_type_val::sell_all_swap_pool,
                sellAllForm->estimateResult.getPoolsExtraCoinsSize()));
    } else {
        dialog->addField(tr("Fee"), app->initData.calc_fee_swap_text(minter::tx_type_val::sell_all_coins));
    }
}
void Ui::TabExchange::sellAllDialogSubmit() {
    dialogSubmit(createTxSender(SellAll), sellAllForm);
}

void Ui::TabExchange::onBuyFormSubmit() {
    if (!buyForm->inputGroup.isValidForm()) {
        return;
    }

    createSendDialog(
        tr("Buy Coins"),
        std::bind(&Ui::TabExchange::buyDialogConfigure, this, std::placeholders::_1),
        std::bind(&Ui::TabExchange::buyDialogSubmit, this));
}
void Ui::TabExchange::onSellFormSubmit() {
    if (!sellForm->inputGroup.isValidForm()) {
        return;
    }

    createSendDialog(
        tr("Sell Coins"),
        std::bind(&Ui::TabExchange::sellDialogConfigure, this, std::placeholders::_1),
        std::bind(&Ui::TabExchange::sellDialogSubmit, this));
}
void Ui::TabExchange::onSellAllFormSubmit() {
    if (!sellAllForm->inputGroup.isValidForm()) {
        return;
    }

    createSendDialog(
        tr("Sell All Coins"),
        std::bind(&Ui::TabExchange::sellAllDialogConfigure, this, std::placeholders::_1),
        std::bind(&Ui::TabExchange::sellAllDialogSubmit, this));
}
