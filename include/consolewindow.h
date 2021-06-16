#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include "include/console_app.h"
#include "include/device_looper.h"
#include "include/net/explorer_repo.h"
#include "include/net/gate_repo.h"
#include "include/tab_base.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QDataWidgetMapper>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QToolButton>
#include <memory>
#include <minter/api/explorer/explorer_results.h>
#include <minter/nanos_wallet.h>
#include <vector>

namespace Ui {
class ConsoleWindow;
} // namespace Ui

class ConsoleWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ConsoleWindow(QWidget* parent = nullptr);
    ~ConsoleWindow();

signals:
    void closed();

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
private slots:
    void onAppStarted();
    void onTabChanged(int idx);
    void onDeviceStateChanged(dev_state type);
    void updateBalance();
    void balanceUpdated(minter::explorer::balance_items);
    void onDevExchangeError(std::exception_ptr e);

private:
    Ui::ConsoleWindow* ui;
    miledger::ConsoleApp* app;

    std::vector<TabBase*> tabs;
    QLabel* balanceLabel;
    QPushButton* balanceRefresh;
    QLabel* addressLabel;
    QPushButton* addressCopy;
    QSpacerItem* rightSpace;
    QScrollArea* exchangeScroll;

    QLabel* statusIcon;
    QLabel* statusLabel;
    QProgressBar* statusProgress;

public:
};

#endif // CONSOLEWINDOW_H
