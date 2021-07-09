#include "include/ui/testwindow.h"

#include "include/errors.h"
#include "include/image_cache.h"
#include "include/input_fields.hpp"
#include "include/net/explorer_repo.h"
#include "ui_testwindow.h"

#include <QDir>
#include <QStandardItem>
#include <QStandardItemModel>
#include <iostream>
#include <minter/api/explorer/explorer_results.h>
#include <rxcpp/operators/rx-observe_on.hpp>
#include <rxcpp/rx-observable.hpp>

TestWindow::TestWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::TestWindow),
      app(new miledger::ConsoleApp(this)),
      model(new CoinModel()) {
    ui->setupUi(this);

    mpd_setminalloc(4);

    std::vector<minter::explorer::coin_item> coins;
    //    minter::explorer::coin_item c0;
    //    c0.id = dev::bigint("0");
    //    c0.symbol = "BIP";
    //    c0.name = "Bistro I Prosto";
    //
    //    minter::explorer::coin_item c1;
    //    c1.id = dev::bigint("1993");
    //    c1.symbol = "USDTE";
    //    c1.name = "Tether USDT";
    //
    //    minter::explorer::coin_item c2;
    //    c2.id = dev::bigint("2065");
    //    c2.symbol = "ETH";
    //    c2.name = "Ethereum ETH";

    //    coins.push_back(c0);
    //    coins.push_back(c1);
    //    coins.push_back(c2);

    miledger::repo::explorer_repo explorerRepo;
    auto res = explorerRepo.get_coins()
                   .as_blocking();

    coins = res.first().data;

    CoinItemViewDelegate* delegate = new CoinItemViewDelegate(app);
    delegate->setIconUpdatedCallback([this](const QModelIndex& index) {
        emit model->dataChanged(index, index);
    });

    model->setItems(coins);
    //    ui->listView->setItemDelegate(delegate);
    //    ui->listView->setModel(model);

    QCompleter* comp = new QCompleter(this);
    comp->setModel(model);
    comp->popup()->setItemDelegate(delegate);
    comp->setCompletionRole(CoinModel::SymbolRole);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    comp->setMaxVisibleItems(10);

    ui->lineEdit->setCompleter(comp);
    //    ui->textEdit->setCompleter(comp);
}

TestWindow::~TestWindow() {
    delete ui;
    delete app;
    delete model;
}
