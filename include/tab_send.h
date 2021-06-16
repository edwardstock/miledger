/*!
 * miledger.
 * tab_send.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_TAB_SEND_H
#define MILEDGER_TAB_SEND_H

#include "device_looper.h"
#include "include/input_group.h"
#include "tab_base.h"
#include "txsenddialog.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QCompleter>
#include <QDataWidgetMapper>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QToolButton>
#include <memory>

namespace Ui {

class TabSend : public TabBase {
    Q_OBJECT
public:
    TabSend(miledger::ConsoleApp* app, QWidget* parent);
    ~TabSend();

    QLabel* labelCoin;
    QLabel* labelRecipient;
    QLabel* labelRecipientError;
    QLabel* labelAmount;
    QLabel* labelAmountError;
    QPushButton* buttonUse25;
    QPushButton* buttonUse50;
    QPushButton* buttonUse75;
    QPushButton* buttonUseMax;
    QLabel* labelPayload;
    QLabel* labelPayloadError;
    QComboBox* inputCoin;
    InputField* inputRecipient;
    InputField* inputAmount;
    InputFieldPlainMultiline* inputPayload;
    QLabel* labelFee;
    QLabel* labelFeeValue;
    QPushButton* buttonSubmit;
    QLabel* labelGasCoin;
    QComboBox* inputGasCoin;

    void setDeviceAvailable(bool available) override;

private:
    enum UsePercentAction {
        Quarter,
        Half,
        HalfAndQuarter,
        Full
    };

private slots:
    void onCoinSelected(int idx);
    void onRecipientChanged(QString, QString value);
    void onBalanceUpdated(minter::explorer::balance_items);
    void onInitDataUpdated(miledger::repo::tx_init_data);
    void onFormValidated(bool valid);
    void onSubmit();
    void onUsePercentClicked(UsePercentAction action);
    void onPayloadChanged(const QString& fieldName, QString value);
    void onGasCoinSelected(int index);

private:
    void calculateFee(size_t payloadLen = 0);
    int currentAccountIdx = -1;
    minter::explorer::balance_item currentAccount;
    dev::bigint gasCoin;
    miledger::InputGroup inputGroup;

    rxcpp::observable<minter::gate::tx_send_result> sendTx();
};

} // namespace Ui

#endif // MILEDGER_TAB_SEND_H
