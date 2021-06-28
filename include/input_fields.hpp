/*!
 * miledger.
 * input_fields.hpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_INPUT_FIELDS_HPP
#define MILEDGER_INPUT_FIELDS_HPP

#include "coin_model.h"
#include "include/console_app.h"
#include "include/image_cache.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QComboBox>
#include <QCompleter>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QString>
#include <QStyledItemDelegate>
#include <functional>
#include <memory>

class CoinItemViewDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using IconUpdatedCallback = std::function<void(const QModelIndex& index)>;

private:
    miledger::ConsoleApp* app;
    IconUpdatedCallback m_iconUpdatedCallback;

    QFont m_font;
    QFont m_fontSub;
    QFontMetrics m_fm;
    QFontMetrics m_fmSub;
    QSize m_iconSize;
    QIcon m_defIcon;

public:
    CoinItemViewDelegate(miledger::ConsoleApp* app)
        : app(app),
#ifndef MILEDGER_LINUX
          m_font("Inter", 14, 600),
          m_fontSub("Inter", 12, 400),
#else
          m_font("Inter", 10, 600),
          m_fontSub("Inter", 8, 400),
#endif
          m_fm(m_font),
          m_fmSub(m_fontSub),
          m_iconSize(26, 26),
          m_defIcon(":/icons/ic_launcher_64.png") {
        // todo: check this does not lead to change font metrics
        m_font.setBold(true);
    }
    ~CoinItemViewDelegate() {
    }

    void setIconUpdatedCallback(IconUpdatedCallback cb) {
        m_iconUpdatedCallback = std::move(cb);
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const override {
        return QSize(option.rect.width(), 40);
    }
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QString idS = index.data(CoinModel::IdRole).toString();

        QString symbol = index.data(CoinModel::SymbolRole).toString();
        QString name = index.data(CoinModel::NameRole).toString();
        QString avatarUrl = index.data(CoinModel::AvatarUrlRole).toString();

        QRect titleRect = option.rect;
        QRect subtitleRect = option.rect;
        QRect iconRect = option.rect;

        iconRect.setTop(iconRect.top() + 8);
        iconRect.setLeft(8);
        iconRect.setRight(m_iconSize.width() + 8);
        iconRect.setBottom(iconRect.top() + m_iconSize.height() + 8);

        titleRect.setLeft(iconRect.right() + 8);
        titleRect.setTop(titleRect.top() + 6);
        titleRect.setBottom(titleRect.top() + m_fm.height());

        subtitleRect.setLeft(iconRect.right() + 8);
        subtitleRect.setTop(titleRect.bottom());
        subtitleRect.setBottom(subtitleRect.top() + m_fmSub.height());

        QPoint iconPoint(
            iconRect.left(),
            iconRect.top());

        // Begin drawing
        painter->save();

        if (app->hasCoinIcon(idS)) {
            painter->drawPixmap(
                iconPoint,
                app->getCoinIcon(idS).pixmap(m_iconSize.width(), m_iconSize.height()));
        } else {
            const QString imageKey = Imager::get().keyForCoin(symbol);
            auto imageRequest = Imager::get()
                                    .createRequest(avatarUrl, imageKey)
                                    .transform([this](std::shared_ptr<Imager::ImageTransformer> transformer) {
                                        transformer->resize(m_iconSize.width(), m_iconSize.height());
                                        transformer->circular();
                                    });

            if (!imageRequest.exist()) {
                imageRequest.download(
                    [this, index](const QPixmap*) { m_iconUpdatedCallback(index); },
                    [](QString error) {
                        qDebug() << error;
                    });
                painter->drawPixmap(
                    iconPoint,
                    m_defIcon.pixmap(m_iconSize.width(), m_iconSize.height()));
            } else {
                QPixmap pm = *imageRequest.get();

                painter->drawPixmap(
                    iconPoint,
                    pm);
            }
        }

        painter->setFont(m_font);
        painter->drawText(titleRect, symbol);

        painter->setFont(m_fontSub);
        painter->drawText(subtitleRect, name);

        painter->restore();
    }
};

class BaseInputField : public QWidget {
    Q_OBJECT

signals:
    void namedTextChanged(const QString& name, QString value);
    void completerSelected(QModelIndex index);

public:
    BaseInputField(QString name, QWidget* parent = nullptr)
        : QWidget(parent),
          m_layout(new QGridLayout()),
          m_name(std::move(name)),
          label(new QLabel(this)),
          errorView(new QLabel(this)) {
    }

    BaseInputField(QString name, QString placeholder, QWidget* parent = nullptr)
        : QWidget(parent),
          m_layout(new QGridLayout()),
          m_name(std::move(name)),
          m_placeholder(std::move(placeholder)),
          label(new QLabel(m_placeholder, this)),
          errorView(new QLabel(this)) {
    }

    BaseInputField(QString name, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : QWidget(parent),
          m_layout(new QGridLayout()),
          m_name(std::move(name)),
          label(label),
          errorView(errorView),
          externalLabels(true) {
    }

    BaseInputField(QString name, QString placeholder, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : QWidget(parent),
          m_layout(new QGridLayout()),
          m_name(std::move(name)),
          m_placeholder(std::move(placeholder)),
          label(label),
          errorView(errorView),
          externalLabels(true) {
    }

    ~BaseInputField() {
    }

    void setName(QString name) {
        m_name = std::move(name);
    }
    const QString& getName() const {
        return m_name;
    }

    const QString& getValue() const {
        return m_value;
    }

    QString& getValue() {
        return m_value;
    }

private:
    QGridLayout* m_layout;
    QString m_name;

protected:
    void internalSetup(QWidget* input) {
        input->setFont(QFont("Inter"));
        m_layout->setContentsMargins(0, 0, 0, 0);
        setLayout(m_layout);

        if (!externalLabels) {
            m_layout->addWidget(label, 0, 0, 1, 1);
            m_layout->addWidget(input, 1, 0, 1, 1);
            m_layout->addWidget(errorView, 2, 0, 1, 1);
        } else {
            m_layout->addWidget(input, 0, 0, 1, 1);
        }

        input->setObjectName("inputFieldInput");
        if (!externalLabels) {
            label->setObjectName("inputFieldLabel");
            errorView->setObjectName("inputFieldError");
        }
        if (!m_placeholder.isEmpty()) {
            label->setText(m_placeholder);
        }
    }
    void addWidget(QWidget* widget, int row, int column, int rowSpan, int columnSpan) {
        m_layout->addWidget(widget, row, column, rowSpan, columnSpan);
    }

    // non-virtual as we using it in constructor
    void setupViews() {
    }

    QString m_value;
    QString m_placeholder;

public:
    QLabel* label;
    QLabel* errorView;
    bool externalLabels = false;
};

class InputField : public BaseInputField {
    Q_OBJECT
private slots:
    void onCompleterSelected(QModelIndex index) {
        emit completerSelected(index);
    };

public:
    QLineEdit* input;
    QCompleter* completer = nullptr;

    InputField(QString name, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), "", parent),
          input(new QLineEdit(this)) {

        setupViews();
        connect(input, &QLineEdit::textChanged, [this](const QString& val) {
            m_value = val;
            emit namedTextChanged(getName(), val);
        });
    }
    InputField(QString name, QString placeholder, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), std::move(placeholder), parent),
          input(new QLineEdit(this)) {

        setupViews();
        connect(input, &QLineEdit::textChanged, [this](const QString& val) {
            m_value = val;
            emit namedTextChanged(getName(), val);
        });
    }

    InputField(QString name, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(name, label, errorView, parent),
          input(new QLineEdit(this)) {
        setupViews();
        connect(input, &QLineEdit::textChanged, [this](const QString& val) {
            m_value = val;
            emit namedTextChanged(getName(), val);
        });
    }

    InputField(QString name, QString placeholder, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(name, placeholder, label, errorView, parent),
          input(new QLineEdit(this)) {
        setupViews();
        connect(input, &QLineEdit::textChanged, [this](const QString& val) {
            m_value = val;
            emit namedTextChanged(getName(), val);
        });
    }

    ~InputField() {
    }

    void setCompleterModel(QAbstractItemModel& model, int column, Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive, QStyledItemDelegate* delegate = nullptr) {
        if (!completer) {
            completer = new QCompleter(this);
            completer->setModel(&model);
            if (delegate != nullptr) {
                completer->popup()->setItemDelegate(delegate);
            }
            completer->setCompletionColumn(column);
            completer->setCaseSensitivity(sensitivity);
            connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(onCompleterSelected(QModelIndex)));
            input->setCompleter(completer);

        } else {
            completer->setModel(&model);
            if (delegate != nullptr) {
                completer->popup()->setItemDelegate(delegate);
            }
        }
    }

    void setText(const QString& text) {
        input->setText(text);
    }

protected:
    void setupViews() {
        input->setFont(QFont("Inter", 10));
        //        input->setSizePolicy(
        //            QSizePolicy::Expanding,
        //            QSizePolicy::Expanding
        //            );
        internalSetup((QWidget*) input);
    }
};

class InputFieldReadOnly : public InputField {
    Q_OBJECT
public:
    InputFieldReadOnly(QString name, QWidget* parent = nullptr)
        : InputField(name, parent) {
        setupViews();
    }
    InputFieldReadOnly(QString name, QString placeholder, QWidget* parent = nullptr)
        : InputField(name, placeholder, parent) {
        setupViews();
    }
    InputFieldReadOnly(QString name, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : InputField(name, label, errorView, parent) {
        setupViews();
    }
    InputFieldReadOnly(QString name, QString placeholder, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : InputField(name, placeholder, label, errorView, parent) {
        setupViews();
    }
    ~InputFieldReadOnly() {
    }

    void setupViews() {
        InputField::setupViews();
        input->setReadOnly(true);
        errorView->setVisible(false);
    }
};

class InputFieldPlainMultiline : public BaseInputField {
    Q_OBJECT

public:
    InputFieldPlainMultiline(QString name, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), "", parent),
          input(new QPlainTextEdit(this)) {
        setupViews();
        connect(input, &QPlainTextEdit::textChanged, [this]() {
            m_value = input->toPlainText();
            emit namedTextChanged(getName(), m_value);
        });
    }
    InputFieldPlainMultiline(QString name, QString placeholder, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), std::move(placeholder), parent),
          input(new QPlainTextEdit(this)) {
        setupViews();
        connect(input, &QPlainTextEdit::textChanged, [this]() {
            m_value = input->toPlainText();
            emit namedTextChanged(getName(), m_value);
        });
    }
    InputFieldPlainMultiline(QString name, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), label, errorView, parent),
          input(new QPlainTextEdit(this)) {
        setupViews();
        connect(input, &QPlainTextEdit::textChanged, [this]() {
            m_value = input->toPlainText();
            emit namedTextChanged(getName(), m_value);
        });
    }
    InputFieldPlainMultiline(QString name, QString placeholder, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), placeholder, label, errorView, parent),
          input(new QPlainTextEdit(this)) {
        setupViews();
        connect(input, &QPlainTextEdit::textChanged, [this]() {
            m_value = input->toPlainText();
            emit namedTextChanged(getName(), m_value);
        });
    }
    ~InputFieldPlainMultiline() {
    }

    QPlainTextEdit* input;

protected:
    void setupViews() {
        input->setFont(QFont("Inter", 10));
        internalSetup((QWidget*) input);
    }
};

class InputFieldDropDown : public BaseInputField {
    Q_OBJECT
signals:
    void itemSelected(int index, QVariant data);

public:
    InputFieldDropDown(QString name, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), "", parent),
          input(new QComboBox(this)) {
        setupViews();
        connect(input, qOverload<int>(&QComboBox::activated), [this](int index) {
            emitFromIndex(index);
        });
    }
    InputFieldDropDown(QString name, QString placeholder, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), std::move(placeholder), parent),
          input(new QComboBox(this)) {
        setupViews();
        connect(input, qOverload<int>(&QComboBox::activated), [this](int index) {
            emitFromIndex(index);
        });
    }
    InputFieldDropDown(QString name, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), label, errorView, parent),
          input(new QComboBox(this)) {
        setupViews();
        connect(input, qOverload<int>(&QComboBox::activated), [this](int index) {
            emitFromIndex(index);
        });
    }
    InputFieldDropDown(QString name, QString placeholder, QLabel* label, QLabel* errorView, QWidget* parent = nullptr)
        : BaseInputField(std::move(name), placeholder, label, errorView, parent),
          input(new QComboBox(this)) {
        setupViews();
        connect(input, qOverload<int>(&QComboBox::activated), [this](int index) {
            emitFromIndex(index);
        });
    }
    ~InputFieldDropDown() {
    }

    void setItems(std::vector<std::pair<QString, QVariant>> data) {
        m_data = std::move(data);
        input->clear();
        for (const auto& item : m_data) {
            input->addItem(item.first, item.second);
        }
    }

    template<typename T>
    void setItems(std::vector<T> data, std::function<std::pair<QString, QVariant>(const T&)> mapper) {
        std::vector<std::pair<QString, QVariant>> out;
        for (const auto& item : data) {
            out.push_back(mapper(item));
        }
        setItems(std::move(out));
    }

    QComboBox* input;

protected:
    std::vector<std::pair<QString, QVariant>> m_data;
    void setupViews() {
        BaseInputField::setupViews();
        internalSetup(input);
    }

private:
    void emitFromIndex(int index) {
        if (index >= 0 && ((size_t) index) < m_data.size()) {
            emit itemSelected(index, m_data[index].second);
        }
    }
};

#endif // MILEDGER_INPUT_FIELDS_HPP
