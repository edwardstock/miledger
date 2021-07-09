/*!
 * miledger.
 * device_looper.cpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "include/device_server.h"

#include "include/settings.h"

#include <QDebug>
#include <iostream>
#include <minter/tx/utils.h>
#include <rxcpp/rx-observable.hpp>

const std::unordered_map<dev_state, std::string> BaseDeviceServer::state_string_map = {
    {dev_state::DISCONNECTED, "DISCONNECTED"},
    {dev_state::APP_NOT_OPENED, "APP_NOT_OPENED"},
    {dev_state::PERMISSION_ERROR, "PERMISSION_ERROR"},
    {dev_state::APP_OPENED, "APP_OPENED"},
};

std::string BaseDeviceServer::stateToString(dev_state state) {
    if (state_string_map.count(state)) {
        return DeviceServer::state_string_map.at(state);
    }
    return "UNKNOWN_STATE";
}

BaseDeviceServer::BaseDeviceServer(QObject* parent)
    : QObject(parent)
    , running(true)
    , skipConnectionCheck(false)
    , infiniteEmitting(true) {
}

bool BaseDeviceServer::isRunning() const {
    return BaseDeviceServer::running;
}
void BaseDeviceServer::setInfiniteEmitting(bool state) {
    BaseDeviceServer::infiniteEmitting = state;
}
bool BaseDeviceServer::isInfiniteEmitting() const {
    return BaseDeviceServer::infiniteEmitting;
}
bool BaseDeviceServer::canInteract() const {
    return BaseDeviceServer::lastState == dev_state::APP_OPENED;
}
dev_state BaseDeviceServer::getState() const {
    return BaseDeviceServer::lastState;
}
void BaseDeviceServer::setState(dev_state state) {
    BaseDeviceServer::lastState = state;
}
std::string BaseDeviceServer::getStateString() const {
    return stateToString(BaseDeviceServer::lastState);
}

void BaseDeviceServer::setSkipCheck(bool skip) {
    BaseDeviceServer::skipConnectionCheck = skip;
}
bool BaseDeviceServer::isSkipCheck() const {
    return BaseDeviceServer::skipConnectionCheck;
}

void BaseDeviceServer::setIsRunning(bool running) {
    BaseDeviceServer::running = running;
}

// Common looper

DeviceServer::DeviceServer(BaseDeviceServer* impl, QObject* parent)
    : BaseDeviceServer(parent)
    , m_looperImpl(impl) {

    connect(m_looperImpl, &BaseDeviceServer::deviceStateChanged, [this](dev_state s) {
        emit deviceStateChanged(s);
    });

    connect(m_looperImpl, &BaseDeviceServer::finished, [this] {
        emit finished();
    });
}

DeviceServer::~DeviceServer() {
    m_looperImpl->stop();
    delete m_looperImpl;
}

void DeviceServer::run() {
    m_looperImpl->run();
}

void DeviceServer::stop() {
    m_looperImpl->stop();
}
rxcpp::observable<minter::address_t> DeviceServer::getAddress(bool silent) {
    return m_looperImpl->getAddress(silent);
}
minter::signature DeviceServer::signTx(tb::bytes_data txHash, uint32_t deriveIndex) {
    return m_looperImpl->signTx(txHash, deriveIndex);
}
rxcpp::observable<bool> DeviceServer::checkMinterAppInstalled() {
    return m_looperImpl->checkMinterAppInstalled();
}

rxcpp::observable<bool> DeviceServer::openMinterApp() {
    return m_looperImpl->openMinterApp();
}
bool DeviceServer::isRunning() const {
    return m_looperImpl->isRunning();
}
void DeviceServer::setInfiniteEmitting(bool state) {
    m_looperImpl->setInfiniteEmitting(state);
}
bool DeviceServer::isInfiniteEmitting() const {
    return m_looperImpl->isInfiniteEmitting();
}
bool DeviceServer::canInteract() const {
    return m_looperImpl->canInteract();
}
dev_state DeviceServer::getState() const {
    return m_looperImpl->getState();
}
std::string DeviceServer::getStateString() const {
    return m_looperImpl->getStateString();
}
void DeviceServer::setSkipCheck(bool skip) {
    m_looperImpl->setSkipCheck(skip);
}
bool DeviceServer::isSkipCheck() const {
    return m_looperImpl->isSkipCheck();
}
void DeviceServer::setState(dev_state state) {
    m_looperImpl->setState(state);
}
void DeviceServer::setIsRunning(bool running) {
    m_looperImpl->setIsRunning(running);
}
