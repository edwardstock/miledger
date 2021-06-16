#include "include/mainwindow.h"
#include "include/miledger-config.h"
#include "include/style_helper.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFontDatabase>
#include <QPalette>
#include <QPixmapCache>
#include <QSettings>
#include <QString>
#include <QStyle>
#include <QStyleFactory>

#ifdef _MSC_VER
#include <direct.h>
#define GET_WORKDIR _getcwd
#else
#include <unistd.h>
#define GET_WORKDIR getcwd
#endif

#include "include/rxqt_instance.hpp"
#include "include/testwindow.h"

#include <iostream>
#include <qlogging.h>

// example for custom message handler
void customLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {

    std::cout << msg.toStdString() << std::endl;
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    // custom handler start
    qInstallMessageHandler(&customLogHandler);

    QApplication::setOrganizationName("MinterTeam");
    QApplication::setOrganizationDomain("minter.network");
    QApplication::setApplicationName("MiLedger");
    QApplication::setApplicationVersion(QString(MILEDGER_VERSION));

    QPixmapCache::setCacheLimit(102400);
    QPixmapCache::clear();

    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_italic.ttf");

    QPalette p = a.palette();

#ifdef MILEDGER_APPLE
    QFile style(":/stylesheet/style_osx.qss");
    style.open(QFile::ReadOnly);

    QString styleSheet{QLatin1String(style.readAll())};
    a.setStyleSheet(styleSheet);

    if (p.color(QPalette::Base).name() == "#171717") {
        StyleHelper::get().setIsDarkMode(true);
    }
#else
    QFile style(":/stylesheet/style_win.qss");
    style.open(QFile::ReadOnly);

    QString styleSheet{QLatin1String(style.readAll())};
    a.setStyleSheet(styleSheet);
#endif

    RxQt::get();

    MainWindow mainWindow;
    std::shared_ptr<ConsoleWindow> consoleWindow;

    QObject::connect(&mainWindow, &MainWindow::openedConsole, &mainWindow, &MainWindow::hide);
    QObject::connect(&mainWindow, &MainWindow::openedConsole, [&consoleWindow]() {
        consoleWindow = std::make_shared<ConsoleWindow>();
        consoleWindow->show();
        QObject::connect(consoleWindow.get(), &ConsoleWindow::closed, []() {
            QApplication::quit();
        });
    });

    QObject::connect(&mainWindow, &MainWindow::closed, &a, &QApplication::quit);

    mainWindow.show();

    //
    //        TestWindow win;
    //        win.show();

    return a.exec();
}
