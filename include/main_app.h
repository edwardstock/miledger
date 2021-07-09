#ifndef MILEDGERAPP_H
#define MILEDGERAPP_H

#include "device_server.h"

#include <QObject>
#include <QThread>

class MainApp : public QObject {
    Q_OBJECT
    Q_ENUMS(dev_state)

    Q_PROPERTY(QString progressHint READ getProgressHint WRITE setProgressHint NOTIFY progressHintChanged)
    Q_PROPERTY(bool progressVisible READ getProgressVisible WRITE setProgressVisible NOTIFY progressVisibilityChanged)
    Q_PROPERTY(bool btnInstallEnabled READ getBtnInstallEnabled WRITE setBtnInstallEnabled NOTIFY btnInstallEnabledChanged)
    Q_PROPERTY(bool btnConsoleEnabled READ getBtnConsoleEnabled WRITE setBtnConsoleEnabled NOTIFY btnConsoleEnabledChanged)

public:
    explicit MainApp(QObject* parent = nullptr);
    ~MainApp();

    QString getProgressHint() const {
        return m_progress_hint;
    }

    void setProgressHint(const QString& hint) {
        m_progress_hint = hint;
    }

    bool getProgressVisible() const {
        return m_progress_visible;
    }

    void setProgressVisible(bool visible) {
        m_progress_visible = visible;
    }

    bool getBtnInstallEnabled() const {
        return m_btn_install_enabled;
    }

    void setBtnInstallEnabled(bool enabled) {
        m_btn_install_enabled = enabled;
    }

    bool getBtnConsoleEnabled() const {
        return m_btn_console_enabled;
    }

    void setBtnConsoleEnabled(bool enabled) {
        m_btn_console_enabled = enabled;
    }

    DeviceServer& getDeviceLooper() {
        return dev;
    }

public slots:
    void onDeviceStateChanged(dev_state type);

signals:
    void progressHintChanged();
    void progressVisibilityChanged();
    void btnInstallEnabledChanged();
    void btnConsoleEnabledChanged();
    void stopHandler();
    void guiDeviceStateChanged(QString hint, bool progress, bool enableInstall, bool enableWallet);
    void appNotOpened();

private:
    QString m_progress_hint = "Searching for ledger...";
    bool m_progress_visible = true;
    bool m_btn_install_enabled = false;
    bool m_btn_console_enabled = false;

    DeviceServer dev;
    QThread dev_thread;
};

#endif // MILEDGERAPP_H
