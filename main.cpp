#include "include/app.h"
#include "include/miledger-config.h"
#include "include/style_helper.h"
#include "include/ui/mainwindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
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
#include "include/ui/testwindow.h"

#include <iostream>
#include <qlogging.h>

// example for custom message handler
void customLogHandler(QtMsgType, const QMessageLogContext&, const QString& msg) {

    std::cout << msg.toStdString() << std::endl;
}

#ifdef Q_OS_WINDOWS
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    int argc;
    char** argv;
    {
        LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
        argv = (char**) malloc(argc * sizeof(char*));
        int size, i = 0;
        for (; i < argc; ++i) {
            size = wcslen(lpArgv[i]) + 1;
            argv[i] = (char*) malloc(size);
            wcstombs(argv[i], lpArgv[i], size);
        }
        LocalFree(lpArgv);
    }
#else
int main(int argc, char* argv[]) {
#endif

    QApplication a(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    // custom handler start
    qInstallMessageHandler(&customLogHandler);

    QApplication::setOrganizationName("MinterTeam");
    QApplication::setOrganizationDomain("minter.network");
    QApplication::setApplicationName("MiLedger");
    QApplication::setApplicationVersion(QString(MILEDGER_VERSION));

    {
        QCommandLineParser parser;
        parser.setApplicationDescription("MiLedger");
        parser.addHelpOption();

        QCommandLineOption enableMock(QStringList() << "m"
                                                    << "mnemonic",
                                      "Use mnemonic to emulate ledger", "mnemonic");
        parser.addOption(enableMock);
        parser.process(a);

        if (parser.isSet(enableMock)) {
            QString mockMnemonic = parser.value(enableMock);
            if (mockMnemonic.isEmpty()) {
                std::cerr << "Mnemonic is empty" << std::endl;
                return 1;
            }

            miledger::App::get().setUseMnemonic(true);
            miledger::App::get().setMnemonic(mockMnemonic);
            qDebug() << "\t- Using mock mnemonic instead Ledger";
        }
    }

    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/resources/fonts/_inter_italic.ttf");

#ifdef Q_OS_MACOS
    QPalette p = a.palette();
    QFile style(":/stylesheet/style_osx.qss");
    style.open(QFile::ReadOnly);

    QString styleSheet{QLatin1String(style.readAll())};
    a.setStyleSheet(styleSheet);

    if (p.color(QPalette::Base).name() == "#171717") {
        StyleHelper::get().setIsDarkMode(true);
    }
#elif defined(Q_OS_LINUX)
    QFile style(":/stylesheet/style_linux.qss");
    style.open(QFile::ReadOnly);

    QString styleSheet{QLatin1String(style.readAll())};
    a.setStyleSheet(styleSheet);
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
