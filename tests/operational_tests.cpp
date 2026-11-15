#include "EmbeddedSystemX.hpp"
#include "OperationalStates.hpp"
#include "RealTimeLoopActiveObject.hpp"

#include <atomic>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void ready(erts::EmbeddedSystemX& system) {
    system.SelftestOk();
    system.Initialized();
}

class TrackingBuffer final : public std::stringbuf {
public:
    bool workerWrote = false;
    const std::thread::id caller = std::this_thread::get_id();
private:
    std::streamsize xsputn(const char* text, std::streamsize count) override {
        if (std::this_thread::get_id() != caller) workerWrote = true;
        return std::stringbuf::xsputn(text, count);
    }
};

void operationalBehavior() {
    TrackingBuffer buffer;
    std::ostream output(&buffer);
    erts::EmbeddedSystemX system(output);
    ready(system);
    system.Configure();
    require(system.stateName() == "Operational/Configuration", "Configure enters Configuration");
    require(buffer.str().find("readConfigurationInfo") != std::string::npos,
            "Configuration must run entry action");
    system.ConfigX();
    require(system.stateName() == "Operational/Configuration", "ConfigX is an internal action");
    require(buffer.str().find("PerformConfigurationX") != std::string::npos,
            "ConfigX must execute configuration action");
    system.Start();
    require(system.stateName() == "Operational/Configuration", "Cannot Start in Configuration");
    system.ConfigurationEnded();
    require(system.stateName() == "Operational/Ready", "ConfigurationEnded returns to Ready");
    system.Start();
    require(system.stateName() == "Operational/RealTimeLoop/Mode1", "Start enters Mode1");
    system.eventX();
    require(buffer.str().find("responseM1eventX") != std::string::npos, "Mode1 response required");

    require(!buffer.workerWrote, "Ordinary startup/actions should run on caller");
    system.chMode2();
    require(buffer.workerWrote, "Mode-change handler must actually execute on worker");
    require(system.stateName() == "Operational/RealTimeLoop/Mode2",
            "Mode-change call must wait until resulting state is visible");
    system.eventX();
    require(buffer.str().find("responseM2eventX") != std::string::npos, "Mode2 response required");
    const auto ignoredStart = buffer.str().size();
    system.chMode2();
    require(system.stateName() == "Operational/RealTimeLoop/Mode2", "Same-mode request is ignored");
    require(buffer.str().substr(ignoredStart).find("ignored") != std::string::npos,
            "Same-mode request must report ignored event");
    system.chMode1();
    require(system.stateName() == "Operational/RealTimeLoop/Mode1", "chMode1 returns to Mode1");
    system.chMode2();
    system.Stop();
    require(system.stateName() == "Operational/Ready", "Stop returns to Ready");
    system.Start();
    require(system.stateName() == "Operational/RealTimeLoop/Mode1", "Reentry resets to Mode1");
    system.eventY();
    require(system.stateName() == "Operational/RealTimeLoop/Mode1", "Undefined eventY is ignored");
    system.Restart();
    require(system.stateName() == "PowerOnSelfTest", "RealTimeLoop inherits Operational Restart");
    system.chMode2();
    require(system.stateName() == "PowerOnSelfTest", "No mode changes outside RealTimeLoop");
    ready(system);
    system.Configure();
    system.Restart();
    require(system.stateName() == "PowerOnSelfTest", "Configuration inherits Operational Restart");
}

void independentModes() {
    std::ostringstream firstOutput, secondOutput;
    erts::EmbeddedSystemX first(firstOutput), second(secondOutput);
    ready(first); ready(second);
    first.Start(); second.Start();
    first.chMode2();
    require(first.stateName() == "Operational/RealTimeLoop/Mode2" &&
            second.stateName() == "Operational/RealTimeLoop/Mode1",
            "Singletons must not share a context's active mode");
}

void concurrentCallers() {
    std::ostringstream output;
    erts::EmbeddedSystemX system(output);
    ready(system);
    system.Start();
    std::atomic<bool> badObservation{false};
    auto changeModes = [&system] {
        for (int i = 0; i < 100; ++i) {
            system.chMode2();
            system.chMode1();
        }
    };
    std::thread first(changeModes), second(changeModes);
    std::thread observer([&] {
        for (int i = 0; i < 400; ++i) {
            auto name = system.stateName();
            if (name != "Operational/RealTimeLoop/Mode1" &&
                name != "Operational/RealTimeLoop/Mode2") badObservation = true;
        }
    });
    first.join(); second.join(); observer.join();
    require(!badObservation, "Readers must see a valid mode during worker activity");
    system.chMode1();
    require(system.stateName() == "Operational/RealTimeLoop/Mode1", "Concurrent requests must complete");
}

class FailingRequest final : public erts::Request {
    void invoke() override { throw std::runtime_error("request failure"); }
};

void requestErrors() {
    FailingRequest request;
    auto done = request.completion();
    request.execute();
    bool propagated = false;
    try { done.get(); }
    catch (const std::runtime_error& error) { propagated = std::string(error.what()) == "request failure"; }
    require(propagated, "Request errors must reach waiting caller instead of hanging");
}
} // namespace

int main() {
    try {
        operationalBehavior();
        independentModes();
        concurrentCallers();
        requestErrors();
        std::cout << "Configuration, modes, worker execution, blocking completion, restart, concurrency PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
