/*!
 * miledger.
 * exchange_forms.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/exchange_forms.h"

dev::bigdec18 const Ui::ExchangeBuyForm::MAX_VALUE = dev::bigdec18("1000000000000000");

void Ui::ExchangeBuyForm::reset() {
    inputAmount->input->clear();
    inputCoinToSell->input->clear();
    inputCoinToBuy->input->clear();
    inputMaxValueToSell->input->clear();
    inputEstimate->input->setText("≈ 0");
    inputGroup.reset();
}

void Ui::ExchangeSellForm::reset() {
    inputAmount->input->clear();
    inputCoinToSell->input->clear();
    inputCoinToBuy->input->clear();
    inputMinValueToBuy->input->clear();
    inputEstimate->input->setText("≈ 0");
    inputGroup.reset();
}

void Ui::ExchangeSellAllForm::reset() {
    inputCoinToSell->input->clear();
    inputCoinToBuy->input->clear();
    inputMinValueToBuy->input->clear();
    inputEstimate->input->setText("≈ 0");
    inputGroup.reset();
}
