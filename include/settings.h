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
#include <unordered_map>

class Settings {
public:
    class BatchSave {
        friend class Settings;

    private:
        BatchSave(Settings& sets)
            : sets(sets) {
        }
        Settings& sets;
        std::unordered_map<QString, QVariant> data;

    public:
        template<typename T>
        BatchSave& set(const QString& key, const T& val) {
            data[key] = QVariant(val);
            return *this;
        }

        void save() {
            std::lock_guard<std::mutex> lock(sets.m_lock);
            std::for_each(data.begin(), data.end(), [this](std::pair<QString, QVariant> it) {
                sets.m_sets.setValue(it.first, it.second);
            });
        }
    };

    const static QString KEY_MINTER_PID;
    const static QString KEY_SERVER_PORT;
    const static QString KEY_SERVER_ADDRESS;
    const static QString KEY_SERVER_CLOSE_TRAY;

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
        m_sets.sync();
    }

    void set(const QString& key, bool value) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_sets.setValue(key, QVariant(value));
        m_sets.sync();
    }

    void set(const QString& key, const QString& value) {
        std::lock_guard<std::mutex> lock(m_lock);
        m_sets.setValue(key, QVariant(value));
        m_sets.sync();
    }

    uint16_t getUint16(const QString& key, uint16_t def = 0) const {
        std::lock_guard<std::mutex> lock(m_lock);
        QVariant res = m_sets.value(key, QVariant((uint16_t) def));
        return (uint16_t) res.toUInt();
    }

    QString getString(const QString& key, const QString& def = "") const {
        std::lock_guard<std::mutex> lock(m_lock);
        QVariant res = m_sets.value(key, QVariant(def));
        return res.toString();
    }

    template<typename T>
    T getValue(const QString& key, const T& def) {
        std::lock_guard<std::mutex> lock(m_lock);
        QVariant res = m_sets.value(key, QVariant((T) def));
        return res.value<T>();
    }

    template<typename T>
    T getValue(const QString& key) {
        std::lock_guard<std::mutex> lock(m_lock);
        QVariant res = m_sets.value(key);
        return res.value<T>();
    }

    BatchSave batchSet() {
        return BatchSave(*this);
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
