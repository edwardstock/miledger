/*!
 * miledger.
 * coin_model.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_COIN_MODEL_H
#define MILEDGER_COIN_MODEL_H

#include "include/miledger-config.h"

#include <QAbstractListModel>
#include <minter/api/explorer/explorer_results.h>
#include <vector>

class CoinModel : public QAbstractListModel {
private:
    std::vector<minter::explorer::coin_item> items;

public:
    enum datarole {
        IdRole = Qt::UserRole + 100,
        SymbolRole = Qt::UserRole + 101,
        NameRole = Qt::UserRole + 102,
        TypeRole = Qt::UserRole + 103,
        AvatarUrlRole = Qt::UserRole + 104
    };

    int rowCount(const QModelIndex&) const override {
        //        qDebug() << "Getting row count: " << items.size();
        return items.size();
    }
    int columnCount(const QModelIndex&) const override {
        return 4;
    }

    //    void sort(int column, Qt::SortOrder order) override {
    ////        QAbstractItemModel::sort(column, order);
    //    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) {
            qDebug() << "Unable to get data by index " << index << " as index is invalid";
            return QVariant();
        }

        if (index.row() < 0 || index.row() >= ((int) items.size())) {
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
            case 0:
                return QString::fromStdString(minter::utils::to_string(items.at(index.row()).id));
            case 1:
                return QString::fromStdString(items.at(index.row()).symbol);
            case 2:
                return items.at(index.row()).type;
            case 3:
                return QString("%1%2").arg(QString(MINTER_COIN_AVATAR_URL), QString::fromStdString(items.at(index.row()).symbol));
            }
        } else if (role == IdRole) {
            return QString::fromStdString(minter::utils::to_string(items.at(index.row()).id));
        } else if (role == SymbolRole) {
            return QString::fromStdString(items.at(index.row()).symbol);
        } else if (role == NameRole) {
            return QString::fromStdString(items.at(index.row()).name);
        } else if (role == TypeRole) {
            return items.at(index.row()).type;
        } else if (role == AvatarUrlRole) {
            return QString("%1%2").arg(QString(MINTER_COIN_AVATAR_URL), QString::fromStdString(items.at(index.row()).symbol));
        }
        return QVariant();
    }

    void setItems(const std::vector<minter::explorer::coin_item>& coins) {
        beginInsertRows(QModelIndex(), 0, coins.size());
        //        insertRows(0, coins->size(), QModelIndex());
        items = coins;
        endInsertRows();
    }
};

#endif // MILEDGER_COIN_MODEL_H
