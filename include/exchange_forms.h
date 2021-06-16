/*!
 * miledger.
 * exchange_forms.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_EXCHANGE_FORMS_H
#define MILEDGER_EXCHANGE_FORMS_H

#include "coin_model.h"
#include "console_app.h"
#include "exchange_calculator.h"
#include "input_fields.hpp"
#include "input_group.h"
#include "rxqt_instance.hpp"
#include "utils.h"
#include "validators.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <memory>
#include <utility>

namespace Ui {

class ExchangeForm : public QObject {
    Q_OBJECT

signals:
    void formValid(bool valid);
    void submitClicked();

public slots:
    virtual void onSubmit() = 0;
    virtual void onFormValid(bool valid) = 0;
    void onCoinToBuySelected(QModelIndex index) {
        coinToBuy = getCoinFromModelIndex(index);
        inputGroup.validate(inputCoinToBuy, true);
        qDebug() << "BASE: coin to buy: " << QString::fromStdString(coinToBuy.symbol) << " index=" << index;
    }
    void onCoinToSellSelected(QModelIndex index) {
        coinToSell = getCoinFromModelIndex(index);
        inputGroup.validate(inputCoinToSell, true);
        qDebug() << "BASE: coin to sell: " << QString::fromStdString(coinToSell.symbol) << " index=" << index;
    }
    void onGasCoinSelected(int index, QVariant data) {
        if (index < 0) {
            gasCoin = minter::def_coin_id;
            return;
        }
        minter::explorer::coin_item_base item = qvariant_cast<minter::explorer::coin_item_base>(data);
        gasCoin = item.id;
        qDebug() << "Selected gas coin: " << item;
        //        gasCoin = app->balances.balances.at(index).coin.id;
    }

public:
    miledger::ConsoleApp* app;
    CoinModel& coinModel;
    miledger::InputGroup inputGroup;

    InputFieldDropDown* inputGasCoin;
    QGroupBox* groupBox;
    QGridLayout* layout;

    InputField* inputCoinToBuy;
    InputField* inputCoinToSell;

    minter::explorer::coin_item coinToBuy;
    minter::explorer::coin_item coinToSell;
    dev::bigint gasCoin;

    std::vector<minter::explorer::balance_item> balances;
    miledger::EstimateResult estimateResult;

    ExchangeForm(miledger::ConsoleApp* app, CoinModel& coinModel, QObject* parent = nullptr)
        : QObject(parent), app(app), coinModel(coinModel), gasCoin(minter::def_coin_id) {
    }

    ~ExchangeForm() {
        delete groupBox;
    }

    std::optional<minter::explorer::balance_item> findBalanceByCoin(const dev::bigint& coinId) const {
        for (const auto& balance : balances) {
            if (balance.coin.id == coinId) {
                return balance;
            }
        }

        return {};
    }

    std::optional<minter::explorer::balance_item> findBalanceByCoin(const minter::explorer::coin_item_base& coin) const {
        return findBalanceByCoin(coin.id);
    }

    virtual void reset() = 0;

    virtual void setBalance(std::vector<minter::explorer::balance_item> b) {
        inputGasCoin->setItems<minter::explorer::balance_item>(b, [](const minter::explorer::balance_item& item) {
            QVariant v;
            v.setValue(item.coin);

            return std::pair<QString, QVariant>(
                QString("%1 (%2)").arg(QString::fromStdString(item.coin.symbol), miledger::utils::humanDecimal(item.amount)),
                std::move(v));
        });
        balances = b;
        // clang-format off
        connect(inputGasCoin, SIGNAL(itemSelected(int,QVariant)), this, SLOT(onGasCoinSelected(int,QVariant)));
        // clang-format on
    }

    void addToParent(QGridLayout* gridLayout, int row) {
        if (layout->children().size() < 0) {
            return;
        }

        for (size_t i = 0; i < ((size_t) layout->children().size()); i++) {
            layout->setRowStretch(i, 1);
        }
        gridLayout->addWidget(groupBox, row, 0, 1, 1);
    }

    QWidget* getWidget() {
        QSpacerItem* vSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout->addItem(vSpacer, layout->rowCount(), 0, 1, 1);
        return groupBox;
    }

    virtual void initViews(QString groupName) {

        groupBox = new QGroupBox(groupName);
        layout = new QGridLayout();
        groupBox->setLayout(layout);

        inputGasCoin = new InputFieldDropDown("gas_coin", tr("Gas Coin"));
        inputCoinToBuy = new InputField("coin_to_buy", tr("Buy coin"));
        inputCoinToSell = new InputField("coin_to_sell", tr("Sell coin"));

        CoinItemViewDelegate* d1 = new CoinItemViewDelegate(app);
        d1->setIconUpdatedCallback([this](const QModelIndex& index) {
            emit coinModel.dataChanged(index, index);
        });
        //        CoinItemViewDelegate* d2 = new CoinItemViewDelegate(app);
        //        d2->setIconUpdatedCallback([this](const QModelIndex& index) {
        //            emit coinModel.dataChanged(index, index);
        //        });

        inputCoinToBuy->setCompleterModel(coinModel, 1, Qt::CaseInsensitive, d1);
        inputCoinToBuy->completer->setMaxVisibleItems(10);

        inputCoinToSell->setCompleterModel(coinModel, 1, Qt::CaseInsensitive, d1);
        inputCoinToSell->completer->setMaxVisibleItems(10);

        inputGroup.addInput(
            inputCoinToBuy,
            {new miledger::MinterCoinValidator(this), new miledger::MinterLoadedCoinValidator(app, this)},
            true);
        inputGroup.addInput(
            inputCoinToSell,
            {new miledger::MinterCoinValidator(this), new miledger::MinterLoadedCoinValidator(app, this)},
            true);

        // clang-format off
        connect(inputCoinToSell, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToSellSelected(QModelIndex)));
        connect(inputCoinToBuy, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToBuySelected(QModelIndex)));
        connect(inputGasCoin, SIGNAL(itemSelected(int,QVariant)), this, SLOT(onGasCoinSelected(int,QVariant)));
        // clang-format on
    }

protected:
    minter::explorer::coin_item getCoinFromModelIndex(const QModelIndex& index) const {
        if (!index.isValid()) {
            qDebug() << "BASE: coin to buy: invalid index " << index;
            return minter::explorer::coin_item{};
        }
        QString coinName = index.data().toString();
        auto searchCoin = app->findCoinBySymbol(coinName);
        if (!searchCoin.has_value()) {
            return minter::explorer::coin_item{};
        }
        return *searchCoin.value();
    }
};

class ExchangeBuyForm : public ExchangeForm {
    Q_OBJECT
public slots:
    void onMaxValueToSellChanged(QString, QString) {
    }
    void onAmountChanged(QString, QString) {
    }
    void onSubmit() override {
        emit submitClicked();
    }
    void onFormValid(bool valid) override {
        submit->setEnabled(valid);
        emit formValid(valid);

        if (!valid) {
            return;
        }

        coinToSell = *app->findCoinBySymbol(
                             inputGroup
                                 .getInputData(inputCoinToSell))
                          .value();
        coinToBuy = *app->findCoinBySymbol(inputGroup.getInputData(inputCoinToBuy)).value();
        amount = dev::bigdec18(inputGroup.getInputData(inputAmount).toStdString());
        maxValueToSell = dev::bigdec18(inputGroup.getInputData(inputMaxValueToSell).toStdString());
        if (maxValueToSell.isnan()) {
            maxValueToSell = MAX_VALUE;
        }

        exchangeCalculator.calculate()
            .subscribe_on(RxQt::get().ioThread())
            .observe_on(RxQt::get().uiThread())
            .subscribe(
                [this](miledger::EstimateResult estimate) {
                    estimateResult = estimate;
                    inputEstimate->input->setText(estimate.formatAmountToTarget(coinToSell.symbol));
                    emit formValid(true);
                },
                [this](std::exception_ptr e) {
                    try {
                        rethrow_exception(e);
                    } catch (const std::exception& e) {
                        inputGroup.setError(inputEstimate->getName(), e);
                        qDebug() << e.what();
                        emit formValid(false);
                    }
                });
    }

public:
    static const dev::bigdec18 MAX_VALUE;
    InputField* inputAmount;
    InputField* inputMaxValueToSell;
    InputField* inputEstimate;
    QPushButton* submit;

    dev::bigdec18 maxValueToSell;
    dev::bigdec18 amount;

    miledger::ExchangeCalculator exchangeCalculator;

    ExchangeBuyForm(miledger::ConsoleApp* app, CoinModel& coinModel, QObject* parent = nullptr)
        : ExchangeForm(app, coinModel, parent),
          maxValueToSell(MAX_VALUE),
          amount("0"),
          exchangeCalculator(&coinToSell, &coinToBuy, &amount, true) {
    }

    void reset() override;

    void initViews(QString groupName = "Buy Coins") override {
        ExchangeForm::initViews(groupName);
        inputAmount = new InputField("amount", tr("Buy amount"));
        inputMaxValueToSell = new InputField("max_value_to_sell", tr("Max amount to sell (optional)"));
        inputEstimate = new InputField("estimate", tr("You will pay approximately"));
        inputEstimate->input->setReadOnly(true);
        inputEstimate->input->setText("≈ 0");
        submit = new QPushButton(tr("Buy"));
        submit->setEnabled(false);

        layout->addWidget(inputCoinToBuy, 0, 0, 1, 1);
        layout->addWidget(inputAmount, 0, 1, 1, 1);
        layout->addWidget(inputCoinToSell, 1, 0, 1, 1);
        layout->addWidget(inputEstimate, 1, 1, 1, 1);
        layout->addWidget(inputMaxValueToSell, 2, 0, 1, 1);
        layout->addWidget(inputGasCoin, 3, 0, 1, 2);
        layout->addWidget(submit, 4, 1, 1, 1);

        inputGroup.addInput(
            inputAmount,
            {new miledger::MinterAmountValidator(this)},
            true);
        inputGroup.addInput(
            inputMaxValueToSell,
            {new miledger::MinterAmountValidator(this)},
            false);

        // clang-format off
        connect(inputCoinToBuy, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToBuySelected(QModelIndex)));
        connect(inputCoinToSell, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToSellSelected(QModelIndex)));
        connect(inputMaxValueToSell, SIGNAL(namedTextChanged(QString,QString)), this, SLOT(onMaxValueToSellChanged(QString,QString)));
        connect(inputAmount, SIGNAL(namedTextChanged(QString,QString)), this, SLOT(onAmountChanged(QString,QString)));
        connect(&inputGroup, SIGNAL(formValid(bool)), this, SLOT(onFormValid(bool)));
        connect(submit, SIGNAL(clicked()), this, SLOT(onSubmit()));
        // clang-format on
    }
};

class ExchangeSellForm : public ExchangeForm {
    Q_OBJECT
public slots:
    void onMinValueToSellChanged(QString, QString) {
    }
    void onAmountChanged(QString, QString) {
        //        amount
    }
    void onSubmit() override {
        emit submitClicked();
    }
    void onFormValid(bool valid) override {
        submit->setEnabled(valid);
        emit formValid(valid);

        if (!valid) {
            return;
        }

        coinToSell = *app->findCoinBySymbol(inputGroup.getInputData(inputCoinToSell)).value();
        coinToBuy = *app->findCoinBySymbol(inputGroup.getInputData(inputCoinToBuy)).value();
        amount = dev::bigdec18(inputGroup.getInputData(inputAmount).toStdString());
        minValueToBuy = dev::bigdec18(inputGroup.getInputData(inputMinValueToBuy).toStdString());
        if (minValueToBuy.isnan()) {
            minValueToBuy = dev::bigdec18("0");
        }

        exchangeCalculator.calculate()
            .subscribe_on(RxQt::get().ioThread())
            .observe_on(RxQt::get().uiThread())
            .subscribe(
                [this](miledger::EstimateResult estimate) {
                    estimateResult = estimate;
                    inputEstimate->input->setText(estimate.formatAmountToTarget(coinToBuy.symbol));
                    emit formValid(true);
                },
                [this](std::exception_ptr e) {
                    try {
                        rethrow_exception(e);
                    } catch (const std::exception& e) {
                        inputGroup.setError(inputEstimate->getName(), e);
                        qDebug() << e.what();
                        emit formValid(false);
                    }
                });
    }

public:
    InputField* inputAmount;
    InputField* inputMinValueToBuy;
    InputField* inputEstimate;
    QPushButton* submit;

    dev::bigdec18 minValueToBuy;
    dev::bigdec18 amount;

    miledger::ExchangeCalculator exchangeCalculator;

    ExchangeSellForm(miledger::ConsoleApp* app, CoinModel& coinModel, QObject* parent = nullptr)
        : ExchangeForm(app, coinModel, parent),
          minValueToBuy("0"),
          amount("0"),
          exchangeCalculator(&coinToSell, &coinToBuy, &amount, false) {
    }

    void reset() override;

    void initViews(QString groupName = "Sell Coins") override {
        ExchangeForm::initViews(groupName);
        inputAmount = new InputField("amount", tr("Sell amount"));
        inputMinValueToBuy = new InputField("min_value_to_buy", tr("Min amount to buy (optional)"));
        inputEstimate = new InputField("estimate", tr("You will get approximately"));
        inputEstimate->input->setReadOnly(true);
        inputEstimate->input->setText("≈ 0");
        submit = new QPushButton(tr("Sell"));
        submit->setEnabled(false);

        layout->addWidget(inputCoinToSell, 0, 0, 1, 1);
        layout->addWidget(inputAmount, 0, 1, 1, 1);
        layout->addWidget(inputCoinToBuy, 1, 0, 1, 1);
        layout->addWidget(inputEstimate, 1, 1, 1, 1);
        layout->addWidget(inputMinValueToBuy, 2, 0, 1, 1);
        layout->addWidget(inputGasCoin, 3, 0, 1, 2);
        layout->addWidget(submit, 4, 1, 1, 1);

        inputGroup.addInput(
            inputAmount,
            {new miledger::MinterAmountValidator(this)},
            true);
        inputGroup.addInput(
            inputMinValueToBuy,
            {new miledger::MinterAmountValidator(this)},
            false);

        // clang-format off
        connect(inputCoinToSell, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToSellSelected(QModelIndex)));
        connect(inputCoinToBuy, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToBuySelected(QModelIndex)));
        connect(inputMinValueToBuy, SIGNAL(namedTextChanged(QString,QString)), this, SLOT(onMinValueToSellChanged(QString,QString)));
        connect(inputAmount, SIGNAL(namedTextChanged(QString,QString)), this, SLOT(onAmountChanged(QString,QString)));
        connect(&inputGroup, SIGNAL(formValid(bool)), this, SLOT(onFormValid(bool)));
        connect(submit, SIGNAL(clicked()), this, SLOT(onSubmit()));
        // clang-format on
    }
};

class ExchangeSellAllForm : public ExchangeForm {
    Q_OBJECT
public slots:
    void onMinValueToBuyChanged(QString, QString) {
    }
    void onSubmit() override {
        emit submitClicked();
    }
    void onFormValid(bool valid) override {
        submit->setEnabled(valid);

        if (!valid) {
            return;
        }

        coinToSell = *app->findCoinBySymbol(inputGroup.getInputData(inputCoinToSell)).value();
        coinToBuy = *app->findCoinBySymbol(inputGroup.getInputData(inputCoinToBuy)).value();

        auto balanceSell = findBalanceByCoin(coinToSell);
        if (balanceSell) {
            amount = balanceSell->amount;
        }

        exchangeCalculator.calculate()
            .subscribe_on(RxQt::get().ioThread())
            .observe_on(RxQt::get().uiThread())
            .subscribe(
                [this](miledger::EstimateResult estimate) {
                    estimateResult = estimate;
                    inputEstimate->input->setText(estimate.formatAmountToTarget(coinToBuy.symbol));
                    emit formValid(true);
                },
                [this](std::exception_ptr e) {
                    try {
                        rethrow_exception(e);
                    } catch (const std::exception& e) {
                        inputGroup.setError(inputEstimate->getName(), e);
                        qDebug() << e.what();
                    }
                    emit formValid(false);
                });
    }

public:
    InputField* inputMinValueToBuy;
    InputField* inputEstimate;
    QPushButton* submit;

    dev::bigdec18 minValueToBuy;
    dev::bigdec18 amount;

    miledger::ExchangeCalculator exchangeCalculator;

    ExchangeSellAllForm(miledger::ConsoleApp* app, CoinModel& coinModel, QObject* parent = nullptr)
        : ExchangeForm(app, coinModel, parent),
          minValueToBuy("0"),
          amount("0"),
          exchangeCalculator(&coinToSell, &coinToBuy, &amount, false) {
    }

    void reset() override;

    void setBalance(std::vector<minter::explorer::balance_item> b) override {
        ExchangeForm::setBalance(b);
        if (!coinToSell.symbol.empty()) {
            auto coinBalance = findBalanceByCoin(coinToSell);
            if (coinBalance.has_value()) {
                amount = coinBalance.value().amount;
            } else {
                amount = dev::bigdec18("0");
            }
        }
    }

    void setAmount(dev::bigdec18 fullBalance) {
        amount = fullBalance;
    }

    void initViews(QString groupName = "Sell All Coins") override {
        ExchangeForm::initViews(groupName);
        inputMinValueToBuy = new InputField("min_value_to_buy", tr("Min amount to buy (optional)"));
        inputEstimate = new InputField("estimate", tr("You will get approximately"));
        inputEstimate->input->setReadOnly(true);
        inputEstimate->input->setText("≈ 0");
        submit = new QPushButton(tr("Sell All"));
        submit->setEnabled(false);

        layout->addWidget(inputCoinToSell, 0, 0, 1, 1);
        layout->addWidget(inputCoinToBuy, 0, 1, 1, 1);
        layout->addWidget(inputEstimate, 1, 0, 1, 1);
        layout->addWidget(inputMinValueToBuy, 1, 1, 1, 1);
        layout->addWidget(inputGasCoin, 2, 0, 1, 2);
        layout->addWidget(submit, 3, 1, 1, 1);

        inputGroup.addInput(
            inputMinValueToBuy,
            {new miledger::MinterAmountValidator(this)},
            false);

        // clang-format off
        connect(inputCoinToSell, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToSellSelected(QModelIndex)));
        connect(inputCoinToBuy, SIGNAL(completerSelected(QModelIndex)), this, SLOT(onCoinToBuySelected(QModelIndex)));
        connect(inputMinValueToBuy, SIGNAL(namedTextChanged(QString,QString)), this, SLOT(onMinValueToBuyChanged(QString,QString)));
        connect(&inputGroup, SIGNAL(formValid(bool)), this, SLOT(onFormValid(bool)));
        connect(submit, SIGNAL(clicked()), this, SLOT(onSubmit()));
        // clang-format on
    }
};

} // namespace Ui

#endif // MILEDGER_EXCHANGE_FORMS_H
