#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "consolewindow.h"
#include "include/main_app.h"
#include "installappwindow.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <functional>
#include <rxcpp/rx-subscription.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void onClickedInstall();
    void onClickedConsole();

signals:
    void closed();
    void openedConsole();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    /* The slot that will accept the signal from the event
     * Click on the application icon in the system tray
     */
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onAppNotOpened();

private:
    Ui::MainWindow* ui;
    InstallAppWindow* installWindow = nullptr;

    /* Declare the object of future applications for the tray icon */
    QSystemTrayIcon* trayIcon;
    QMenu* menu;
    QAction* viewWindow;
    QAction* quitAction;

    MainApp* app;

    QMessageBox* openAppDialog = nullptr;

    bool showWindow = true;
    rxcpp::composite_subscription m_subs;
};
#endif // MAINWINDOW_H
