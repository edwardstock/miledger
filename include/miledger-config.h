/*!
 * miledger-qtforms.
 * miledger-config.in.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_CONFIG_IN_H
#define MILEDGER_CONFIG_IN_H

#include <minter/minter_tx_config.h>

#define CMD_BIN "bash"
#define CMD_BIN_ARG "-c"
/* #undef PYTHON_BIN */
#define LEDGER_APP_VERSION "0.1.0"
#define LEDGER_APP_INSTALLER "loader"
#define MILEDGER_APPLE
#define MILEDGER_VERSION "0.1.0"
#define MILEDGER_VERSION_MAJOR "0"
#define MILEDGER_VERSION_MINOR "1"
#define MILEDGER_VERSION_PATCH "0"

#define MINTER_CHAIN_ID minter::chain_id::testnet
#define MINTER_EXPLORER_API_V "v2"
#define MINTER_GATE_API_V "v2"
#define MINTER_DEFAULT_COIN "MNT"
#define MINTER_EXPLORER_API "https://explorer-api.testnet.minter.network/api/v2/"
#define MINTER_EXPLORER_FRONT "https://explorer.testnet.minter.network"
#define MINTER_EXPLORER_STATIC_URL "https://explorer-static.testnet.minter.network"
#define MINTER_GATE_API "https://gate-api.testnet.minter.network/api/v2/"
#define MINTER_WS_URL "wss://explorer-rtm.testnet.minter.network/connection/websocket"
#define MINTER_COIN_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/coin/"
#define MINTER_ADDRESS_AVATAR_URL "https://my.apps.minter.network/api/v1/avatar/by/address/"

#endif // MILEDGER_CONFIG_IN_H
