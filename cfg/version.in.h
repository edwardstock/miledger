/*!
 * miledger.
 * version.in.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_VERSION_IN_H
#define MILEDGER_VERSION_IN_H

#define VER_FILEVERSION             ${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH},0
#define VER_FILEVERSION_STR         "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}.0\0"

#define VER_PRODUCTVERSION          ${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH},0
#define VER_PRODUCTVERSION_STR      "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}.0\0"

#define VER_COMPANYNAME_STR         "Eduard Maximovich"
#define VER_FILEDESCRIPTION_STR     "MiLedger"
#define VER_INTERNALNAME_STR        "MiLedger - Minter Ledger GUI Client"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2021 Eduard Maximovich"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "miledger.exe"
#define VER_PRODUCTNAME_STR         "miledger"

#define VER_COMPANYDOMAIN_STR       "https://github.com/edwardstock/miledger"

#endif //MILEDGER_VERSION_IN_H