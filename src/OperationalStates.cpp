#include "OperationalStates.hpp"
#include "EmbeddedSystemX.hpp"

namespace erts {

void Ready::Configure(EmbeddedSystemX& context) {
    context.transitionTo(Configuration::instance());
}

void Ready::Start(EmbeddedSystemX& context) {
    context.reportAction("run");
    context.transitionTo(RealTimeLoop::instance());
}

Configuration& Configuration::instance() {
    static Configuration state;
    return state;
}

void Configuration::onEntry(EmbeddedSystemX& context) {
    context.reportAction("readConfigurationInfo");
}
void Configuration::ConfigX(EmbeddedSystemX& context) {
    context.reportAction("PerformConfigurationX");
}
void Configuration::ConfigurationEnded(EmbeddedSystemX& context) {
    context.transitionTo(Ready::instance());
}

RealTimeLoop& RealTimeLoop::instance() {
    static RealTimeLoop state;
    return state;
}

void RealTimeLoop::onEntry(EmbeddedSystemX& context) {
    context.setMode(Mode1::instance());
}
void RealTimeLoop::Stop(EmbeddedSystemX& context) {
    context.transitionTo(Ready::instance());
}
void RealTimeLoop::chMode1(EmbeddedSystemX& context) { context.queueMode1(); }
void RealTimeLoop::chMode2(EmbeddedSystemX& context) { context.queueMode2(); }
void RealTimeLoop::eventX(EmbeddedSystemX& context) { context.respondToModeEventX(); }

Mode1& Mode1::instance() {
    static Mode1 state;
    return state;
}
void Mode1::chMode2(EmbeddedSystemX& context) { context.setMode(Mode2::instance()); }
void Mode1::eventX(EmbeddedSystemX& context) {
    context.reportAction("responseM1eventX");
}

Mode2& Mode2::instance() {
    static Mode2 state;
    return state;
}
void Mode2::chMode1(EmbeddedSystemX& context) { context.setMode(Mode1::instance()); }
void Mode2::eventX(EmbeddedSystemX& context) {
    context.reportAction("responseM2eventX");
}

} // namespace erts
