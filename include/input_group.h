/*!
 * miledger.
 * input_group.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_INPUT_GROUP_H
#define MILEDGER_INPUT_GROUP_H

#include "include/validators.hpp"
#include "input_fields.hpp"

#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QRegularExpressionValidator>
#include <QString>
#include <fmt/format.h>
#include <utility>

namespace miledger {

struct InputGroupData {
    BaseInputField* input;
    std::vector<miledger::BaseMinterValidator*> validators;
    QLabel* errorView;
    bool required;
    bool externalError = false;

    void setError(const QString& error) {
        if (externalError) {
            errorView->setVisible(true);
        }
        errorView->setText(error);
    }
    void clearError() {
        if (externalError) {
            errorView->setVisible(false);
        }
        errorView->clear();
    }
};

class InputGroup : public QObject {
    Q_OBJECT

signals:
    void formValid(bool valid);
    void textChanged(QString, QString, bool);

private slots:
    void inputChanged(const QString& fieldName, const QString& value) {
        auto& item = inputs[fieldName];
        unsigned invalidCount = 0;
        for (const auto& v : item.validators) {
            // ignore non required empty value
            if (!item.required && value.isEmpty()) {
                continue;
            }

            int pos = -1;
            auto val = value;
            if (!v) {
                qDebug() << "Validator for field " << fieldName << " is null!";
                throw std::runtime_error("Validator is null");
            }
            auto validState = v->validate(val, pos);

            if (validState != QValidator::Acceptable) {
                invalidCount++;
                item.setError(v->getErrorMessage());
            } else {
                item.clearError();
            }
        }
        bool valid = invalidCount == 0;
        validStates[fieldName] = valid;
        if (valid) {
            qDebug() << "Set value " << value << " to field " << fieldName;
            inputData[fieldName] = value;
        } else {
            inputData[fieldName].clear();
        }

        emit textChanged(fieldName, value, valid);
        emit formValid(isValidForm());
    }

public:
    InputGroup(QObject* parent = nullptr)
        : QObject(parent) {
    }
    ~InputGroup() {
    }
    void addInput(
        BaseInputField* input,
        std::vector<miledger::BaseMinterValidator*> validators,
        bool required) {

        InputGroupData data{input, std::move(validators), input->errorView, required, false};
        connect(input, &BaseInputField::namedTextChanged, this, &InputGroup::inputChanged);
        inputs.insert(input->getName(), std::move(data));
        inputData[input->getName()] = "";
        // by default, all required inputs are invalid
        if (required) {
            requiredCount++;
            validStates[input->getName()] = false;
        } else {
            validStates[input->getName()] = true;
        }
    }
    void addInput(
        BaseInputField* input,
        QLabel* errorView,
        std::vector<miledger::BaseMinterValidator*> validators,
        bool required) {
        InputGroupData data{input, validators, errorView, required, true};
        this->connect(input, &BaseInputField::namedTextChanged, this, &InputGroup::inputChanged);
        data.errorView->setObjectName("inputFieldError");
        inputs.insert(input->getName(), std::move(data));
        inputData[input->getName()] = "";
        // by default, all required inputs are invalid
        if (required) {
            requiredCount++;
            validStates[input->getName()] = false;
        } else {
            validStates[input->getName()] = true;
        }
    }

    bool validate(const BaseInputField* input, bool emitFormValid = false) {
        return validate(input->getName(), emitFormValid);
    }

    bool validate(QString fieldName, bool emitFormValid = false) {
        auto item = inputs[fieldName];
        unsigned invalidCount = 0;

        for (const auto& v : item.validators) {
            if (!item.required && item.input->getValue().isEmpty()) {
                continue;
            }

            int pos = -1;
            auto validState = v->validate(item.input->getValue(), pos);

            if (validState == QValidator::Invalid) {
                invalidCount++;
                item.setError(v->getErrorMessage());
                break;
            } else {
                item.errorView->clear();
            }
        }
        validStates[fieldName] = invalidCount == 0;

        if (emitFormValid) {
            emit formValid(isValidForm());
        }
        return invalidCount == 0;
    }

    bool isValidForm() const {
        QHashIterator<QString, InputGroupData> i(inputs);
        unsigned validCount = 0;
        while (i.hasNext()) {
            i.next();
            auto fieldName = i.key();
            if (validStates[fieldName]) {
                validCount++;
            }
        }
        return validCount == (unsigned int) validStates.size();
    }

    bool validate() {
        QHashIterator<QString, InputGroupData> i(inputs);
        unsigned validCount = 0;
        while (i.hasNext()) {
            i.next();

            if (validate(i.key())) {
                validCount++;
            }
        }

        emit formValid(validCount == (unsigned int) inputs.size());

        return validCount == (unsigned int) inputs.size();
    }

    void reset() {
        for (const auto& kv : inputs) {
            kv.errorView->clear();
            if (kv.required) {
                validStates[kv.input->getName()] = false;
            }
        }
    }

    void setError(const QString& fieldName, const std::exception& e) {
        if (!inputs.contains(fieldName)) {
            return;
        }
        inputs[fieldName].setError(e.what());
    }

    void setError(const QString& fieldName, QString error) {
        if (!inputs.contains(fieldName)) {
            return;
        }
        inputs[fieldName].setError(std::move(error));
    }

    const QHash<QString, QString>& getInputData() const {
        return inputData;
    }

    QString getInputData(BaseInputField* input) const {
        if (!inputData.contains(input->getName())) {
            throw std::runtime_error(fmt::format("Undefined input field {0}", input->getName().toStdString()));
        }

        return inputData[input->getName()];
    }

private:
    unsigned requiredCount = 0;
    QHash<QString, InputGroupData> inputs;
    QHash<QString, bool> validStates;
    QHash<QString, QString> inputData;
};

} // namespace miledger

#endif // MILEDGER_INPUT_GROUP_H
