#include "EmbeddedSystemX.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void startupBehavior() {
    std::ostringstream output;
    erts::EmbeddedSystemX system(output);
    require(system.stateName() == "PowerOnSelfTest", "Must wait for self-test result");
    system.Initialized();
    require(system.stateName() == "PowerOnSelfTest", "Cannot skip self-test");
    system.SelftestOk();
    require(system.stateName() == "Initializing", "Must wait for initialization result");
    system.SelfTestFailed(99);
    require(system.stateName() == "Initializing", "Self-test failure invalid after self-test");
    require(system.errorNo() == 0, "Ignored failure must not alter error data");
    system.Initialized();
    require(system.stateName() == "Operational/Ready", "Initialization enters Ready");
    system.Exit();
    require(!system.terminated(), "Exit only valid in Failure");
    system.Restart();
    require(system.stateName() == "PowerOnSelfTest", "Operational restart reruns self-test");

    system.SelfTestFailed(42);
    require(system.stateName() == "Failure", "Failed self-test enters Failure");
    require(system.errorNo() == 42, "Failure retains supplied error");
    require(output.str().find("Error: 42") != std::string::npos, "Failure displays error");
    const auto beforeInvalid = output.str().size();
    system.Start();
    require(system.stateName() == "Failure", "Start in Failure must preserve state");
    require(output.str().substr(beforeInvalid) ==
                "Event 'Start' ignored in Failure; state unchanged.\n",
            "Ignored event must explain unchanged state");
    system.Restart();
    require(system.stateName() == "PowerOnSelfTest", "Failure restart enters self-test");
    system.SelfTestFailed(7);
    system.Exit();
    require(system.terminated(), "Exit in Failure terminates");
    system.SelftestOk();
    system.Restart();
    system.SelfTestFailed(123);
    require(system.terminated(), "Events cannot revive terminated system");
    require(system.errorNo() == 7, "Events after termination cannot alter error");
}

void separateContexts() {
    std::ostringstream firstOutput, secondOutput;
    erts::EmbeddedSystemX first(firstOutput), second(secondOutput);
    first.SelfTestFailed(10);
    second.SelfTestFailed(20);
    require(first.errorNo() == 10 && second.errorNo() == 20,
            "Singleton states must not share mutable errors between contexts");
    first.Restart();
    require(second.stateName() == "Failure", "Contexts must transition independently");
}
} // namespace

int main() {
    try {
        startupBehavior();
        separateContexts();
        std::cout << "Startup, failure, restart, ignored events, termination, and context isolation PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
