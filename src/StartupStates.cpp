#include "StartupStates.hpp"
#include "EmbeddedSystemX.hpp"

namespace erts {

PowerOnSelfTest& PowerOnSelfTest::instance() {
    static PowerOnSelfTest state;
    return state;
}

void PowerOnSelfTest::onEntry(EmbeddedSystemX& context) {
    context.reportAction("systemSelftest; awaiting SelftestOk or SelfTestFailed");
}

void PowerOnSelfTest::SelftestOk(EmbeddedSystemX& context) {
    context.transitionTo(Initializing::instance());
}

void PowerOnSelfTest::SelfTestFailed(EmbeddedSystemX& context, int errorNo) {
    context.recordFailure(errorNo);
    context.transitionTo(Failure::instance());
}

Initializing& Initializing::instance() {
    static Initializing state;
    return state;
}

void Initializing::onEntry(EmbeddedSystemX& context) {
    context.reportAction("startInitializing; awaiting Initialized");
}

void Initializing::Initialized(EmbeddedSystemX& context) {
    context.transitionTo(Operational::instance());
}

Failure& Failure::instance() {
    static Failure state;
    return state;
}

void Failure::onEntry(EmbeddedSystemX& context) { context.displayError(); }
void Failure::Restart(EmbeddedSystemX& context) {
    context.transitionTo(PowerOnSelfTest::instance());
}
void Failure::Exit(EmbeddedSystemX& context) { context.terminate(); }

Operational& Operational::instance() {
    static Operational state;
    return state;
}

void Operational::onEntry(EmbeddedSystemX& context) {
    context.transitionTo(Ready::instance());
}

void Operational::Restart(EmbeddedSystemX& context) {
    context.transitionTo(PowerOnSelfTest::instance());
}

Ready& Ready::instance() {
    static Ready state;
    return state;
}

} // namespace erts
