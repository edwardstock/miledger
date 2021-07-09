/*!
 * miledger.
 * tab_base.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_TAB_BASE_H
#define MILEDGER_TAB_BASE_H

#include "console_app.h"
#include "include/ui/txsenddialog.h"

#include <QGridLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QWidget>
#include <functional>

class TabBase : public QWidget {
    Q_OBJECT
signals:
    void progressLabelChanged(QString);

public:
    TabBase(miledger::ConsoleApp* app, QWidget* parent = nullptr)
        : QWidget(parent),
          app(app) {
        setLayout(new QGridLayout());
    }
    ~TabBase() {
        delete sendDialog;
    }
    virtual void setDeviceAvailable(bool available) = 0;

protected slots:
    void onProgressLabelChanged(QString label) {
        if (progressDialog == nullptr) {
            return;
        }
        progressDialog->setLabelText(label);
    }

protected:
    void showResultDialog(QString text, QString details = "") {
        resultDialog = new QMessageBox();
        resultDialog->setModal(true);
        resultDialog->setStandardButtons(QMessageBox::Ok);
        resultDialog->setDefaultButton(QMessageBox::Ok);
        resultDialog->setText(text);
        if (!details.isEmpty()) {
            resultDialog->setDetailedText(details);
        }
        resultDialog->exec();
        delete resultDialog;
        resultDialog = nullptr;
    }
    void showProgressDialog(QString text) {
        progressDialog = new QProgressDialog();
        progressDialog->setLabelText(text);
        progressDialog->setCancelButton(nullptr);
        progressDialog->setMinimum(0);
        progressDialog->setMaximum(0);
        progressDialog->setModal(true);

        connect(this, &TabBase::progressLabelChanged, this, &TabBase::onProgressLabelChanged);
        progressDialog->show();
    }
    void hideProgressDialog() {
        if (progressDialog) {
            progressDialog->hide();
            delete progressDialog;
            progressDialog = nullptr;
        }
    }

    void createSendDialog(QString title,
                          std::function<void(miledger::TxSendDialog*)> dialogConfig,
                          std::function<void()> onDialogOpen) {
        if (sendDialog) {
            sendDialog->hide();
            delete sendDialog;
            sendDialog = nullptr;
        }
        sendDialog = new miledger::TxSendDialog(title);
        dialogConfig(sendDialog);
        connect(sendDialog, &miledger::TxSendDialog::accepted, [this, &onDialogOpen]() {
            sendDialog->hide();
            delete sendDialog;
            sendDialog = nullptr;
            onDialogOpen();
        });
        connect(sendDialog, &miledger::TxSendDialog::rejected, [this]() {
            sendDialog->hide();
            delete sendDialog;
            sendDialog = nullptr;
        });

        sendDialog->exec();
    }

    QGridLayout* getLayout() {
        return static_cast<QGridLayout*>(layout());
    }
    miledger::ConsoleApp* app;

    miledger::TxSendDialog* sendDialog = nullptr;
    QProgressDialog* progressDialog = nullptr;
    QMessageBox* resultDialog = nullptr;
};

#endif // MILEDGER_TAB_BASE_H
