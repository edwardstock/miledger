/*!
 * miledger.
 * app.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_APP_H
#define MILEDGER_APP_H

#include "include/device_server.h"

#include <QString>

namespace miledger {

class App {
public:
    static App& get() {
        static App app;
        return app;
    }

    void setUseMnemonic(bool use);
    void setMnemonic(QString m);
    const QString& getMnemonic() const;
    bool useMnemonic() const;

    BaseDeviceServer* createLooper();

private:
    App();

    bool m_useMock = false;
    QString m_mockMnemonic;
};

} // namespace miledger

#endif // MILEDGER_APP_H
