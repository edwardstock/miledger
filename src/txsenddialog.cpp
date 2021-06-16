#include "include/txsenddialog.h"

#include "include/input_fields.hpp"
#include "include/utils.h"
#include "ui_txsenddialog.h"

#include <QFormLayout>
#include <QLabel>

miledger::TxSendDialog::TxSendDialog(QString t, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::TxSendDialog),
      title(new QLabel(t)) {

    ui->setupUi(this);
    setWindowFlag(Qt::Dialog);
    setWindowTitle(tr("Confirm Transaction"));

    ui->layout->addWidget(title, 0, 0, 1, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttons, &QDialogButtonBox::accepted, this, &TxSendDialog::accepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &TxSendDialog::rejected);
}

miledger::TxSendDialog::~TxSendDialog() {
    delete ui;
}

void miledger::TxSendDialog::show() {
    for (const auto& item : inputs) {
        ui->layout->addWidget(item, ui->layout->rowCount(), 0, 1, 1);
    }
    ui->layout->addWidget(buttons, ui->layout->rowCount(), 0, 1, 1);
    QWidget::show();
}

int miledger::TxSendDialog::exec() {
    for (const auto& item : inputs) {
        ui->layout->addWidget(item, ui->layout->rowCount(), 0, 1, 1);
    }
    ui->layout->addWidget(buttons, ui->layout->rowCount(), 0, 1, 1);
    return QDialog::exec();
}

void miledger::TxSendDialog::addField(const QString& name, const QString& value) {
    InputFieldReadOnly* field = new InputFieldReadOnly("input", name);
    field->input->setObjectName("sendDialogInput");
    field->setText(value);

    inputs.push_back(std::move(field));
}

void miledger::TxSendDialog::addFieldAmount(const QString& name, const QString& amount, const minter::explorer::coin_item_base& coin) {
    InputFieldReadOnly* field = new InputFieldReadOnly("input", name);
    field->setText(QString("%1 %2").arg(
        miledger::utils::humanDecimal(amount.toStdString()),
        QString::fromStdString(coin.symbol)));

    inputs.push_back(std::move(field));
}
void miledger::TxSendDialog::addFieldAddress(const QString& name, const minter::address_t& address) {
    addFieldAddress(name, QString::fromStdString(address.to_string()));
}
void miledger::TxSendDialog::addFieldAddress(const QString& name, const QString& address) {
    addField(name, address);
}

void miledger::TxSendDialog::reset() {
    for (const auto& item : inputs) {
        ui->layout->removeWidget(item);
        delete item;
    }
    inputs.clear();
}
