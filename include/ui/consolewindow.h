#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include "include/api/ws_server.h"
#include "include/console_app.h"
#include "include/device_server.h"
#include "include/net/explorer_repo.h"
#include "include/net/gate_repo.h"
#include "include/tab_base.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QComboBox>
#include <QDataWidgetMapper>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSystemTrayIcon>
#include <QThread>
#include <QToolButton>
#include <memory>
#include <minter/api/explorer/explorer_results.h>
#include <minter/ledger/nanos_wallet.h>
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
    void createTray();
private slots:
    void onAppStarted();
    void onTabChanged(int idx);
    void onDeviceStateChanged(dev_state type);
    void updateBalance();
    void balanceUpdated(minter::explorer::balance_items);
    void onDevExchangeError(std::exception_ptr e);
    void onServerClicked(bool);
    void onServerSettingsClicked(bool);
    void onServerStopped();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();

private:
    Ui::ConsoleWindow* ui;
    miledger::ConsoleApp* app;
    bool showedFirstTime = false;

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

    QThread* serverThread = nullptr;
    miledger::WsServer* server = nullptr;

    // tray
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
    QAction* trayRestoreAction = nullptr;
    QAction* trayMinimizeAction = nullptr;
    QAction* trayStopServerAndClose = nullptr;

public:
};

#endif // CONSOLEWINDOW_H
