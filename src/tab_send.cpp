/*!
 * miledger.
 * tab_send.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */
#include "include/tab_send.h"

#include "include/input_group.h"
#include "include/ui/txsenddialog.h"
#include "include/utils.h"
#include "include/validators.hpp"

#include <QDebug>
#include <QGroupBox>
#include <minter/tx/tx.h>
#include <minter/tx/tx_builder.h>
#include <minter/tx/tx_send_coin.h>
#include <rxcpp/operators/rx-switch_on_next.hpp>
#include <vector>

using namespace minter;

Ui::TabSend::TabSend(miledger::ConsoleApp* app, QWidget* parent)
    : TabBase(app, parent),
      gasCoin(minter::def_coin_id),
      inputGroup(this) {

    labelCoin = new QLabel(tr("Coin:"));
    labelRecipient = new QLabel(tr("Recipient:"));
    labelRecipientError = new QLabel();
    labelRecipientError->setVisible(false);
    labelAmount = new QLabel(tr("Amount:"));
    labelAmountError = new QLabel();
    labelAmountError->setVisible(false);
    buttonUse25 = new QPushButton(QString("25%"));
    buttonUse50 = new QPushButton(QString("50%"));
    buttonUse75 = new QPushButton(QString("75%"));
    buttonUseMax = new QPushButton(QString("100%"));
    labelPayload = new QLabel(tr("Payload (message):"));
    labelPayloadError = new QLabel();
    labelPayloadError->setVisible(false);
    inputCoin = new QComboBox();
    inputRecipient = new InputField("recipient", labelRecipient, labelRecipientError);
    inputAmount = new InputField("amount", labelAmount, labelAmountError);
    inputPayload = new InputFieldPlainMultiline("payload", labelPayload, labelPayloadError);

    labelGasCoin = new QLabel(tr("Gas Coin:"));
    inputGasCoin = new QComboBox();

    labelFee = new QLabel(tr("Fee:"));
    labelFeeValue = new QLabel(tr("loading..."));
    buttonSubmit = new QPushButton("Submit");

    inputGroup.addInput(
        inputRecipient,
        labelRecipientError,
        {new miledger::MinterAddressValidator(this)},
        true);
    inputGroup.addInput(
        inputAmount,
        labelAmountError,
        {new miledger::MinterAmountValidator(this)},
        true);
    inputGroup.addInput(
        inputPayload,
        labelPayloadError,
        {new miledger::MinterPayloadValidator(10000, this)},
        false);

    connect(&inputGroup, SIGNAL(formValid(bool)), this, SLOT(onFormValidated(bool)));

    labelCoin->setBuddy(inputCoin);
    labelRecipient->setBuddy(inputRecipient->input);
    labelAmount->setBuddy(inputAmount->input);
    labelPayload->setBuddy(inputPayload->input);

    getLayout()->addWidget(labelCoin, 0, 0, 1, 1);
    getLayout()->addWidget(inputCoin, 0, 1, 1, 4);

    getLayout()->addWidget(labelRecipient, 1, 0, 1, 1);
    getLayout()->addWidget(inputRecipient->input, 1, 1, 1, 4);
    getLayout()->addWidget(labelRecipientError, 2, 1, 1, 4);

    getLayout()->addWidget(buttonUse25, 5, 1, 1, 1);
    getLayout()->addWidget(buttonUse50, 5, 2, 1, 1);
    getLayout()->addWidget(buttonUse75, 5, 3, 1, 1);
    getLayout()->addWidget(buttonUseMax, 5, 4, 1, 1);
    getLayout()->addWidget(labelAmount, 3, 0, 1, 1);
    getLayout()->addWidget(inputAmount->input, 3, 1, 1, 4);
    getLayout()->addWidget(labelAmountError, 4, 1, 1, 4);

    getLayout()->addWidget(labelPayload, 6, 0, 1, 1);
    getLayout()->addWidget(inputPayload->input, 6, 1, 1, 4);
    getLayout()->addWidget(labelPayloadError, 7, 1, 1, 4);

    getLayout()->addWidget(labelGasCoin, 8, 0, 1, 1);
    getLayout()->addWidget(inputGasCoin, 8, 1, 1, 4);

    getLayout()->addWidget(labelFee, 9, 0, 1, 1);
    getLayout()->addWidget(labelFeeValue, 9, 1, 1, 1);
    getLayout()->addWidget(buttonSubmit, 10, 4, 1, 1);

    buttonSubmit->setEnabled(false);
    connect(buttonSubmit, SIGNAL(clicked()), this, SLOT(onSubmit()));

    connect(buttonUse25, &QPushButton::clicked, [this]() {
        onUsePercentClicked(Quarter);
    });
    connect(buttonUse50, &QPushButton::clicked, [this]() {
        onUsePercentClicked(Half);
    });
    connect(buttonUse75, &QPushButton::clicked, [this]() {
        onUsePercentClicked(HalfAndQuarter);
    });
    connect(buttonUseMax, &QPushButton::clicked, [this]() {
        onUsePercentClicked(Full);
    });

    // clang-format off
    connect(inputCoin, SIGNAL(currentIndexChanged(int)), this, SLOT(onCoinSelected(int)));
    connect(inputGasCoin, SIGNAL(currentIndexChanged(int)), this, SLOT(onGasCoinSelected(int)));
    connect(inputPayload, &BaseInputField::namedTextChanged, this, &TabSend::onPayloadChanged);
    connect(inputAmount, &BaseInputField::namedTextChanged, this, &TabSend::onAmountChanged);

    connect(app, SIGNAL(balanceUpdated(minter::explorer::balance_items)), this, SLOT(onBalanceUpdated(minter::explorer::balance_items)));
    connect(app, SIGNAL(initDataUpdated(miledger::repo::tx_init_data)), this, SLOT(onInitDataUpdated(miledger::repo::tx_init_data)));
    // clang-format on
}

Ui::TabSend::~TabSend() {
    delete inputRecipient;
    delete inputAmount;
    delete inputPayload;
}

void Ui::TabSend::onUsePercentClicked(Ui::TabSend::UsePercentAction action) {
    if (currentAccountIdx < 0) {
        return;
    }
    switch (action) {
    case Quarter:
        inputAmount->setText(QString::fromStdString(
            (currentAccount.amount * dev::bigdec18("0.25")).format(".18f")));
        break;
    case Half:
        inputAmount->setText(QString::fromStdString(
            (currentAccount.amount * dev::bigdec18("0.5")).format(".18f")));
        break;
    case HalfAndQuarter:
        inputAmount->setText(QString::fromStdString(
            (currentAccount.amount * dev::bigdec18("0.75")).format(".18f")));
        break;
    case Full:
        inputAmount->setText(QString::fromStdString(
            (currentAccount.amount).format(".18f")));
        useMax = true;
        break;
    }
}

void Ui::TabSend::onSubmit() {
    qDebug() << "on Submit";
    if (!inputGroup.isValidForm()) {
        return;
    }

    createSendDialog(
        tr("Send Coins"),
        [this](miledger::TxSendDialog* dialog) {
            dialog->addFieldAmount(tr("You send"), inputGroup.getInputData(inputAmount), currentAccount.coin);
            dialog->addFieldAddress(tr("To the address"), inputGroup.getInputData(inputRecipient));
            dialog->addField(tr("Fee"), labelFeeValue->text());
        },
        [this]() {
            showProgressDialog("Sending transaction...");

            sendTx()
                .subscribe_on(RxQt::get().ioThread())
                .observe_on(RxQt::get().uiThread())
                .subscribe(
                    [this](gate::tx_send_result result) {
                        hideProgressDialog();

                        if (result.is_ok()) {
                            inputRecipient->input->clear();
                            inputAmount->input->clear();
                            inputPayload->input->clear();
                            inputGroup.reset();
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
        });
}

void Ui::TabSend::onFormValidated(bool valid) {
    buttonSubmit->setEnabled(valid);
}

rxcpp::observable<gate::tx_send_result> Ui::TabSend::sendTx() {
    return rxcpp::observable<>::create<gate::tx_send_result>([this](rxcpp::subscriber<gate::tx_send_result> emitter) {
        app->getInitDataUpdater()
            .subscribe(
                [this, &emitter](miledger::repo::tx_init_data initData) {
                    auto txBuilder = minter::new_tx();
                    txBuilder->set_gas_price(initData.gas);
                    txBuilder->set_gas_coin_id(gasCoin);
                    txBuilder->set_chain_id(MINTER_CHAIN_ID);
                    txBuilder->set_nonce(initData.nonce);

                    auto payload = inputGroup.getInputData(inputPayload);
                    if (!payload.isEmpty()) {
                        auto payloadBytesQt = payload.toLocal8Bit();
                        if (payloadBytesQt.size() > 0) {
                            dev::bytes payloadBytes;
                            payloadBytes.resize(payloadBytesQt.size());
                            for (size_t i = 0; i < ((size_t) payloadBytesQt.size()); i++) {
                                payloadBytes[i] = (uint8_t) payloadBytesQt.at(i);
                            }
                            txBuilder->set_payload(std::move(payloadBytes));
                        }
                    }

                    auto dataBuilder = txBuilder->tx_send_coin();
                    dataBuilder->set_coin_id(currentAccount.coin.id);
                    dataBuilder->set_value(inputGroup.getInputData(inputAmount).toStdString());
                    dataBuilder->set_to(inputGroup.getInputData(inputRecipient).toStdString());

                    auto tx = dataBuilder->build();
                    auto rawTx = tx->get_unsigned_hash();

                    minter::signature signature;
                    try {
                        emit progressLabelChanged(QString("Please compare transaction hash and approve it: \n%1").arg(QString::fromStdString(rawTx.to_hex())));
                        signature = app->dev.signTx(rawTx);
                        emit progressLabelChanged("Sending transaction...");
                    } catch (const std::exception& e) {
                        emitter.on_error(std::make_exception_ptr(std::runtime_error(std::string(e.what()))));
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

void Ui::TabSend::onPayloadChanged(const QString&, QString value) {
    calculateFee(value.toLocal8Bit().size());
}

void Ui::TabSend::onAmountChanged(const QString&, QString) {
    useMax = false;
}

void Ui::TabSend::onInitDataUpdated(miledger::repo::tx_init_data) {
    calculateFee(0);
}

void Ui::TabSend::onBalanceUpdated(explorer::balance_items result) {
    inputCoin->clear();
    for (const auto& balance : result.balances) {
        inputCoin->addItem(QString("%1 (%2)")
                               .arg(
                                   QString::fromStdString(balance.coin.symbol),
                                   miledger::utils::humanDecimal(balance.amount)));
    }
    currentAccountIdx = 0;
    currentAccount = app->balances.balances[0];

    inputGasCoin->clear();
    for (const auto& balance : result.balances) {
        if (balance.coin.type != explorer::coin_type::pool_token) {
            inputGasCoin->addItem(QString("%1 (%2)").arg(
                QString::fromStdString(balance.coin.symbol),
                miledger::utils::humanDecimal(balance.amount)));
        }
    }
    gasCoin = minter::def_coin_id;
}

void Ui::TabSend::onCoinSelected(int idx) {
    qDebug() << "Selected account " << idx;
    currentAccountIdx = idx;
    if (idx < 0) {
        return;
    }
    currentAccount = app->balances.balances[idx];
}

void Ui::TabSend::onGasCoinSelected(int index) {
    if (index <= 0) {
        return;
    }
    gasCoin = app->balances.balances[index].coin.id;
}

void Ui::TabSend::onRecipientChanged(QString, QString) {
}

void Ui::TabSend::setDeviceAvailable(bool available) {
    inputCoin->setEnabled(available);
    inputRecipient->input->setEnabled(available);
    inputAmount->input->setEnabled(available);
    inputPayload->input->setEnabled(available);

    if (available) {
        //        auto address = dev->getWallet().get_address(0, true);
    }
}

void Ui::TabSend::calculateFee(size_t payloadLen) {
    labelFeeValue->setText(
        app->initData.calc_fee_text(minter::send_coin, payloadLen));
}
