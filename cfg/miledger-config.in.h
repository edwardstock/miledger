/*!
 * miledger.
 * miledger-config.in.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_CONFIG_IN_H
#define MILEDGER_CONFIG_IN_H

#include <minter/minter_tx_config.h>

// clang-format off

#cmakedefine CMD_BIN "${CMD_BIN}"
#cmakedefine CMD_BIN_ARG "${CMD_BIN_ARG}"
#cmakedefine PYTHON_BIN "${PYTHON_BIN}"
#cmakedefine LEDGER_APP_VERSION "${LEDGER_APP_VERSION}"
#cmakedefine LEDGER_APP_INSTALLER "${LEDGER_APP_INSTALLER}"
#cmakedefine MILEDGER_APPLE
#cmakedefine MILEDGER_LINUX
#define MILEDGER_VERSION "${CMAKE_PROJECT_VERSION}"
#define MILEDGER_VERSION_MAJOR "${CMAKE_PROJECT_VERSION_MAJOR}"
#define MILEDGER_VERSION_MINOR "${CMAKE_PROJECT_VERSION_MINOR}"
#define MILEDGER_VERSION_PATCH "${CMAKE_PROJECT_VERSION_PATCH}"

#cmakedefine MINTER_CHAIN_ID ${MINTER_CHAIN_ID}
#cmakedefine MINTER_EXPLORER_API_V "${MINTER_EXPLORER_API_V}"
#cmakedefine MINTER_GATE_API_V "${MINTER_GATE_API_V}"
#cmakedefine MINTER_DEFAULT_COIN "${MINTER_DEFAULT_COIN}"
#cmakedefine MINTER_EXPLORER_API "${MINTER_EXPLORER_API}"
#cmakedefine MINTER_EXPLORER_FRONT "${MINTER_EXPLORER_FRONT}"
#cmakedefine MINTER_EXPLORER_STATIC_URL "${MINTER_EXPLORER_STATIC_URL}"
#cmakedefine MINTER_GATE_API "${MINTER_GATE_API}"
#cmakedefine MINTER_WS_URL "${MINTER_WS_URL}"
#cmakedefine MINTER_COIN_AVATAR_URL "${MINTER_COIN_AVATAR_URL}"
#cmakedefine MINTER_ADDRESS_AVATAR_URL "${MINTER_ADDRESS_AVATAR_URL}"

#cmakedefine HAVE_CXX_OPTIONAL

#cmakedefine ENABLE_SERVER

// clang-format on

#include <QHash>
#include <QString>
#include <functional>

// namespace std {
// template<>
// struct hash<QString> {
//     std::size_t operator()(const QString& s) const noexcept {
//         return (size_t) qHash(s);
//     }
// };
// } // namespace std

#endif // MILEDGER_CONFIG_IN_H
