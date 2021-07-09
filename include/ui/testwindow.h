#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include "include/coin_model.h"
#include "include/console_app.h"

#include <QMainWindow>

namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TestWindow(QWidget* parent = nullptr);
    ~TestWindow();

private:
    Ui::TestWindow* ui;
    miledger::ConsoleApp* app;
    CoinModel* model;
};

#endif // TESTWINDOW_H
