/*!
 * miledger.
 * settings.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_SETTINGS_H
#define MILEDGER_SETTINGS_H

#include "miledger-config.h"

#include <QSettings>
#include <mutex>
class Settings {
public:
    const static QString KEY_MINTER_PID;

    static Settings& get() {
        static Settings inst;
        return inst;
    }

    bool has(const QString& key) {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_sets.contains(key);
    }

    void remove(const QString& key) {
        m_sets.remove(key);
    }

    void set(const QString& key, const uint16_t value) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_sets.setValue(key, QVariant(value));
    }

    uint16_t getUint16(const QString& key) const {
        std::lock_guard<std::mutex> lock(m_lock);
        QVariant res = m_sets.value(key, QVariant((uint16_t) 0));
        return (uint16_t) res.toUInt();
    }

    QSettings& settings() {
        return m_sets;
    }

private:
    Settings()
        : m_sets("MinterTeam", "MiLedger") {
    }
    QSettings m_sets;
    mutable std::mutex m_lock;
};

#endif // MILEDGER_SETTINGS_H
