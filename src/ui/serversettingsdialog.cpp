#include "include/ui/serversettingsdialog.h"

#include "include/settings.h"
#include "include/validators.hpp"
#include "ui_serversettingsdialog.h"

#include <QPushButton>

ServerSettingsDialog::ServerSettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ServerSettingsDialog) {
    ui->setupUi(this);
    setWindowTitle(tr("Server Settings"));

    inputPort = new InputField("port", tr("Port"));
    inputAddress = new InputField("address", tr("Address"));
    inputCloseToTray = new InputFieldCheckbox("close_to_tray", tr("Close console to tray when server running"));

    inputGroup.addInput(
        inputPort,
        {new miledger::PortValidator(this)},
        true);
    inputGroup.addInput(
        inputAddress,
        {new miledger::IpAddressValidator(this)},
        true);
    inputGroup.addInput(
        inputCloseToTray, {}, false);

    inputPort->input->setInputMethodHints(Qt::ImhDigitsOnly);
    //    inputPort->input->setInputMask("99999");
    inputPort->setText(QString::number(Settings::get().getUint16(Settings::KEY_SERVER_PORT, 8081)));

    //    inputAddress->setInputMask("000.000.000.000;_");
    inputAddress->setText(Settings::get().getString(Settings::KEY_SERVER_ADDRESS, "127.0.0.1"));

    inputCloseToTray->label->setWordWrap(true);
    inputCloseToTray->setChecked(Settings::get().getValue<>(Settings::KEY_SERVER_CLOSE_TRAY, true));

    connect(&inputGroup, &miledger::InputGroup::formValid, this, &ServerSettingsDialog::formValid);

    ui->formLayout->addRow(inputPort);
    ui->formLayout->addRow(inputAddress);
    ui->formLayout->addRow(inputCloseToTray);
}

ServerSettingsDialog::~ServerSettingsDialog() {
    delete ui;
}

void ServerSettingsDialog::formValid(bool valid) {
    ui->buttons->button(QDialogButtonBox::Save)->setEnabled(valid);

    if (!valid) {
        return;
    }

    valuePort = (uint16_t) inputGroup.getInputData(inputPort).toUInt();
    valueAddress = inputGroup.getInputData(inputAddress);
    valueCloseToTray = inputGroup.getInputData(inputCloseToTray).toUInt() == 1;

    Settings::get()
        .batchSet()
        .set(Settings::KEY_SERVER_PORT, valuePort)
        .set(Settings::KEY_SERVER_ADDRESS, valueAddress)
        .set(Settings::KEY_SERVER_CLOSE_TRAY, valueCloseToTray)
        .save();
}
