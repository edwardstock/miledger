/*!
 * miledger.
 * validators.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/validators.hpp"

const QString miledger::IpAddressValidator::m_ipRangeRegex = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
const QRegularExpression miledger::IpAddressValidator::m_regexp = QRegularExpression("^" + m_ipRangeRegex + "(\\." + m_ipRangeRegex + ")" + "(\\." + m_ipRangeRegex + ")" + "(\\." + m_ipRangeRegex + ")$");