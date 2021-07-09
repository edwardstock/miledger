#ifndef INSTALLAPPWINDOW_H
#define INSTALLAPPWINDOW_H

#include "ui_installappwindow.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QProcess>

namespace Ui {
class InstallAppWindow;
}

class InstallAppWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit InstallAppWindow(QWidget* parent = nullptr);
    ~InstallAppWindow();

    void closeEvent(QCloseEvent* event) override {
        QMainWindow::closeEvent(event);
        if (event->isAccepted())
            emit closed();
    }

signals:
    void installStarted();
    void installFinished();
    void closed();

private slots:
    void onClickedInstall();
    void writeOutput();
    void onProcessFinished(int, QProcess::ExitStatus);

private:
    void buildInstallArgs(QStringList& args);

    Ui::InstallAppWindow* ui;
    QProcess* process;
};

#endif // INSTALLAPPWINDOW_H
