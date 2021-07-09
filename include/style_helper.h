/*!
 * miledger.
 * style_helper.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_STYLE_HELPER_H
#define MILEDGER_STYLE_HELPER_H

#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QScreen>
#include <QString>

class StyleHelper {
private:
    bool m_isDarkMode = false;

public:
    static StyleHelper& get() {
        static StyleHelper inst;
        return inst;
    }

    void setIsDarkMode(bool dark) {
        m_isDarkMode = dark;
    }

    QIcon icon(const QString& fileName) const {
        auto i = QIcon(QString(":/icons/%1%2.png").arg(fileName, m_isDarkMode ? "_dark" : "_light"));
        if (i.isNull()) {
            return QIcon(QString(":/icons/%1.png").arg(fileName));
        }
        return i;
    }

    void setIconToLabel(QLabel* label, const QString& fileName) {
        QIcon i = icon(fileName);
        int wh = label->fontMetrics().height();
        label->setPixmap(
            i.pixmap(wh, wh));
    }

    QString iconText(const QString& text, const QString& fileName, int iconSize = 32, bool iconLeft = true) {
        const QString iconPath = QString(":/icons/%1%2.png").arg(fileName, m_isDarkMode ? "_dark" : "_light");
        if (iconLeft) {
            return QString("<html><span style='display:inline-block;'><img width=%1 height=%1 style='width:%1;height:%1;' src='%2'/> %3</span></html>").arg(QString::number(pxToPt(iconSize)), iconPath, text);
        }

        return QString("<html><span>%1 <img width=%2 height=%2 style='width:%2;height:%2;' src='%3'/></span></html>").arg(text, QString::number(pxToPt(iconSize)), iconPath);
    }

    double ptToPx(double pt) {
        return pt / 72 * dpi();
    }

    double pxToPt(double px) {
        return px * 72 / dpi();
    }

    double dpi() {
        return QGuiApplication::primaryScreen()->physicalDotsPerInch();
    }

    //    static bool IS_DARK_MODE = false;
};

#endif // MILEDGER_STYLE_HELPER_H
