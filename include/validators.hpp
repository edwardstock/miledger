/*!
 * miledger.
 * validators.hpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_VALIDATORS_HPP
#define MILEDGER_VALIDATORS_HPP

#include "console_app.h"

#include <QDoubleValidator>
#include <QRegularExpressionValidator>
#include <QString>
#include <QValidator>
#include <minter/minter_tx_config.h>
#include <minter/tx/utils.h>
#include <vector>

namespace miledger {

class BaseMinterValidator : public QValidator {
    Q_OBJECT
public:
    BaseMinterValidator(QObject* parent = nullptr)
        : QValidator(parent) {
    }
    BaseMinterValidator(QString errorMessage = "", QObject* parent = nullptr)
        : QValidator(parent), m_errorMessage(std::move(errorMessage)) {
    }

    virtual const QString& getErrorMessage() const {
        return m_errorMessage;
    }

    void setErrorMessage(QString errorMessage) {
        m_errorMessage = std::move(errorMessage);
    }

private:
    QString m_errorMessage;
};

class MinterRegexpValidator : public BaseMinterValidator {
    Q_OBJECT
public:
    MinterRegexpValidator(QRegularExpression exp, QObject* parent = nullptr)
        : BaseMinterValidator("Invalid input", parent), m_validator(exp) {
    }
    MinterRegexpValidator(QString errorMessage, QRegularExpression exp, QObject* parent = nullptr)
        : BaseMinterValidator(errorMessage, parent),
          m_validator(exp) {
    }

    State validate(QString& string, int& i) const override {
        return m_validator.validate(string, i);
    }
    void fixup(QString& string) const override {
        m_validator.fixup(string);
    }

protected:
    QRegularExpressionValidator& getValidator() {
        return m_validator;
    }

private:
    QRegularExpressionValidator m_validator;
};

class MinterDoubleValidator : public BaseMinterValidator {
    Q_OBJECT
public:
    MinterDoubleValidator(QObject* parent = nullptr)
        : BaseMinterValidator(tr("Invalid float number"), parent), m_validator() {
    }
    MinterDoubleValidator(QString errorMessage, QObject* parent = nullptr)
        : BaseMinterValidator(errorMessage, parent), m_validator() {
    }

    virtual State validate(QString& string, int& i) const override {
        return m_validator.validate(string, i);
    }

    void setDecimals(int dec) {
        m_validator.setDecimals(dec);
    }

    void setNotation(QDoubleValidator::Notation notation) {
        m_validator.setNotation(notation);
    }

    virtual void fixup(QString& string) const override {
        m_validator.fixup(string);
    }

protected:
    QDoubleValidator& getValidator() {
        return m_validator;
    }

private:
    QDoubleValidator m_validator;
};

class MinterAddressValidator : public virtual MinterRegexpValidator {
    Q_OBJECT
public:
    MinterAddressValidator(QObject* parent = nullptr)
        : MinterRegexpValidator(tr("Invalid minter address format: it must have prefix \"Mx\" and hex-characters body with length = 40"),
                                QRegularExpression("^(Mx)?([a-fA-F0-9]{40})$", QRegularExpression::PatternOption::CaseInsensitiveOption), parent) {
    }
    State validate(QString& string, int& i) const override {
        return MinterRegexpValidator::validate(string, i);
    }

    void fixup(QString& string) const override {
        MinterRegexpValidator::fixup(string);
    }
};

class MinterPubkeyValidator : public virtual MinterRegexpValidator {
    Q_OBJECT
public:
    MinterPubkeyValidator(QObject* parent = nullptr)
        : MinterRegexpValidator(
              tr("Invalid public key format: it must have prefix \"Mp\" and hex-characters body with length = 64"),
              QRegularExpression("^(Mp)?([a-fA-F0-9]{64})$",
                                 QRegularExpression::PatternOption::CaseInsensitiveOption),
              parent) {
    }
    State validate(QString& string, int& i) const override {
        return MinterRegexpValidator::validate(string, i);
    }

    void fixup(QString& string) const override {
        MinterRegexpValidator::fixup(string);
    }
};

class MinterAmountValidator : public virtual MinterDoubleValidator {
    Q_OBJECT
    QStringList _decimalPoints;

public:
    MinterAmountValidator(QObject* parent = nullptr)
        : MinterDoubleValidator(tr("Invalid number format"), parent) {
        setDecimals(18);
        setNotation(QDoubleValidator::StandardNotation);
        _decimalPoints.append(".");
        _decimalPoints.append(",");
        _decimalPoints.append("comma");
        getValidator().setBottom(0);
    }

    State validate(QString& str, int& pos) const override {
        QString s(str);

        for (QStringList::ConstIterator point = _decimalPoints.begin(); point != _decimalPoints.end(); ++point) {
            s.replace(*point, locale().decimalPoint());
        }
        return MinterDoubleValidator::validate(s, pos);
    }

    void fixup(QString& string) const override {
        string.replace(QChar(','), QChar('.'), Qt::CaseInsensitive);
    }
};

class MinterPayloadValidator : public virtual BaseMinterValidator {
    Q_OBJECT
public:
    MinterPayloadValidator(unsigned maxLength = 10000, QObject* parent = nullptr)
        : BaseMinterValidator(tr("Invalid payload length: maximum 10 000 bytes allowed"), parent),
          m_maxLength(maxLength) {
    }
    State validate(QString& string, int&) const override {
        if (string.length() > m_maxLength) {
            return Invalid;
        }

        return Acceptable;
    }

private:
    unsigned m_maxLength;
};

class MinterCoinValidator : public virtual MinterRegexpValidator {
    Q_OBJECT
public:
    MinterCoinValidator(QObject* parent = nullptr)
        : MinterRegexpValidator(
              tr("Invalid coin name"),
              QRegularExpression(R"(^[a-zA-Z0-9]{3,10}(-\d+)?$)", QRegularExpression::CaseInsensitiveOption), parent) {
    }

    QValidator::State validate(QString& input, int& pos) const override {
        return MinterRegexpValidator::validate(input, pos);
    }

    void fixup(QString&) const override {
    }
};

class MinterLoadedCoinValidator : public virtual BaseMinterValidator {
    Q_OBJECT
public:
    MinterLoadedCoinValidator(miledger::ConsoleApp* app, QObject* parent = nullptr)
        : BaseMinterValidator("", parent),
          app(app) {
    }

    const QString& getErrorMessage() const override {
        return m_error;
    }

    State validate(QString& string, int&) const override {
        if (app->findCoinBySymbol(string).has_value()) {
            return Acceptable;
        }

        if (string.isEmpty()) {
            m_error = QString("Coin name required");
            return Invalid;
        }

        m_error = QString("Coin %1 not found").arg(string);
        return Invalid;
    }

private:
    mutable QString m_error;
    miledger::ConsoleApp* app;
};

class MinterBigdecRangeValidator : public virtual BaseMinterValidator {
    Q_OBJECT
public:
    MinterBigdecRangeValidator(dev::bigdec18 minValue, QObject* parent = nullptr)
        : BaseMinterValidator(parent), m_min(minValue), m_maxSet(false) {
    }

    MinterBigdecRangeValidator(dev::bigdec18 minValue, dev::bigdec18 maxValue, QObject* parent = nullptr)
        : BaseMinterValidator(parent), m_min(minValue), m_max(maxValue), m_maxSet(true) {
    }

    const QString& getErrorMessage() const override {
        return m_error;
    }

    State validate(QString& string, int&) const override {
        try {
            dev::bigdec18 n(string.toStdString());
            if (m_maxSet) {
                bool valid = n >= m_min && n <= m_max;
                if (!valid) {
                    m_error = QString("Number out of range: minimum acceptable value: %1, maximum: %2")
                                  .arg(QString::fromStdString(minter::utils::to_string(m_min)),
                                       QString::fromStdString(minter::utils::to_string(m_max)));
                }
                return valid ? Acceptable : Invalid;
            } else {
                bool valid = n >= m_min;
                if (!valid) {
                    m_error = QString("Number out of range: minimum acceptable value: %1")
                                  .arg(QString::fromStdString(minter::utils::to_string(m_min)));
                }
                return valid ? Acceptable : Invalid;
            }
        } catch (const std::exception& e) {
            qDebug() << "NaN range input: " << e.what();
            m_error = "Invalid number";
            return State::Invalid;
        }
        return Acceptable;
    }

private:
    mutable QString m_error;
    dev::bigdec18 m_min;
    dev::bigdec18 m_max;
    bool m_maxSet = false;
};

} // namespace miledger

#endif // MILEDGER_VALIDATORS_HPP
