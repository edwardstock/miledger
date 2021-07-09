#ifndef TXSENDDIALOG_H
#define TXSENDDIALOG_H

#include "include/input_fields.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <memory>
#include <minter/api/explorer/explorer_results.h>
#include <unordered_map>

namespace Ui {
class TxSendDialog;
}

namespace miledger {

class TxSendDialog : public QDialog {
    Q_OBJECT

signals:
    void accepted();
    void rejected();

public:
    explicit TxSendDialog(QString title, QWidget* parent = nullptr);
    ~TxSendDialog();

    void addField(const QString& name, const QString& value);
    void addFieldAmount(const QString& name, const QString& amount, const minter::explorer::coin_item_base& coin);
    void addFieldAddress(const QString& name, const minter::address_t& address);
    void addFieldAddress(const QString& name, const QString& address);
    void reset();
    void show();
    int exec() override;

private slots:
    //    void onAccept();
    //    void onReject();

private:
    Ui::TxSendDialog* ui;

    QLabel* title;
    QDialogButtonBox* buttons;
    std::vector<InputFieldReadOnly*> inputs;
};

} // namespace miledger

#endif // TXSENDDIALOG_H
