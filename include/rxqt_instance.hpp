/*!
 * miledger.
 * rxqt_instance.hpp
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_RXQT_INSTANCE_HPP
#define MILEDGER_RXQT_INSTANCE_HPP
#include <QObject>
#include <rxcpp/rx.hpp>
#include <rxqt_run_loop.hpp>

class RxQt {
private:
    rxqt::run_loop m_mainLoop;
    RxQt()
        : m_mainLoop() {
    }

public:
    static RxQt& get() {
        static RxQt inst;
        return inst;
    }

    rxcpp::observe_on_one_worker uiThread() {
        return m_mainLoop.observe_on_run_loop();
    }

    rxcpp::observe_on_one_worker ioThread() {
        return rxcpp::observe_on_event_loop();
    }
};

#endif // MILEDGER_RXQT_INSTANCE_HPP
