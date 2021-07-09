#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include "include/input_fields.hpp"
#include "include/input_group.h"

#include <QDialog>

namespace Ui {
class ServerSettingsDialog;
}

class ServerSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ServerSettingsDialog(QWidget* parent = nullptr);
    ~ServerSettingsDialog();

private slots:
    void formValid(bool valid);

private:
    Ui::ServerSettingsDialog* ui;

    miledger::InputGroup inputGroup;

    InputField* inputPort;
    InputField* inputAddress;
    InputFieldCheckbox* inputCloseToTray;

    uint16_t valuePort;
    QString valueAddress;
    bool valueCloseToTray = true;
};

#endif // SERVERSETTINGSDIALOG_H
