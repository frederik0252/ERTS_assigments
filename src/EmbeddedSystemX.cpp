#include "EmbeddedSystemX.hpp"
#include "StartupStates.hpp"
#include "OperationalStates.hpp"
#include "RealTimeLoopActiveObject.hpp"

#include <utility>

namespace erts {

EmbeddedSystemX::EmbeddedSystemX(std::ostream& output, std::string name, int versionNo)
    : output_(output), name_(std::move(name)), versionNo_(versionNo),
      modeWorker_(std::make_unique<RealTimeLoopActiveObject>()) {
    transitionTo(PowerOnSelfTest::instance());
}

EmbeddedSystemX::~EmbeddedSystemX() = default;

std::string_view EmbeddedSystemX::stateNameUnlocked() const noexcept {
    if (state_ == &RealTimeLoop::instance() && mode_) return mode_->name();
    return state_ ? state_->name() : "Terminated";
}

std::string_view EmbeddedSystemX::stateName() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    return stateNameUnlocked();
}

bool EmbeddedSystemX::terminated() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    return state_ == nullptr;
}

int EmbeddedSystemX::errorNo() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    return errorNo_;
}

void EmbeddedSystemX::recordFailure(int errorNo) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    errorNo_ = errorNo;
}

void EmbeddedSystemX::transitionTo(State& next) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    mode_ = nullptr;
    state_ = &next;
    state_->onEntry(*this);
}

void EmbeddedSystemX::terminate() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    state_ = nullptr;
    mode_ = nullptr;
    output_ << "System terminated.\n";
}

void EmbeddedSystemX::displayState() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    output_ << "State: " << stateNameUnlocked() << '\n';
}

void EmbeddedSystemX::reportIgnored(std::string_view event) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    output_ << "Event '" << event << "' ignored in " << stateNameUnlocked()
            << "; state unchanged.\n";
}

void EmbeddedSystemX::reportAction(std::string_view action) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    output_ << "Simulated action: " << action << '\n';
}

void EmbeddedSystemX::displayError() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    output_ << "Error: " << errorNo_ << '\n';
}

void EmbeddedSystemX::dispatch(void (State::*handler)(EmbeddedSystemX&),
                             std::string_view event) {
    std::lock_guard<std::mutex> eventLock(eventMutex_);
    std::future<void> completed;
    {
        std::lock_guard<std::recursive_mutex> stateLock(stateMutex_);
        if (state_) {
            (state_->*handler)(*this);
            completed = std::move(pendingModeChange_);
        } else {
            reportIgnored(event);
        }
    }
    // The worker can now lock stateMutex_. Holding eventMutex_ prevents Stop,
    // Restart or another event from changing the captured source in between.
    if (completed.valid()) completed.get();
}

void EmbeddedSystemX::SelftestOk() { dispatch(&State::SelftestOk, "SelftestOk"); }
void EmbeddedSystemX::Initialized() { dispatch(&State::Initialized, "Initialized"); }
void EmbeddedSystemX::Restart() { dispatch(&State::Restart, "Restart"); }
void EmbeddedSystemX::Exit() { dispatch(&State::Exit, "Exit"); }
void EmbeddedSystemX::Configure() { dispatch(&State::Configure, "Configure"); }
void EmbeddedSystemX::ConfigX() { dispatch(&State::ConfigX, "ConfigX"); }
void EmbeddedSystemX::ConfigurationEnded() {
    dispatch(&State::ConfigurationEnded, "ConfigurationEnded");
}
void EmbeddedSystemX::Start() { dispatch(&State::Start, "Start"); }
void EmbeddedSystemX::Stop() { dispatch(&State::Stop, "Stop"); }
void EmbeddedSystemX::chMode1() { dispatch(&State::chMode1, "chMode1"); }
void EmbeddedSystemX::chMode2() { dispatch(&State::chMode2, "chMode2"); }
void EmbeddedSystemX::eventX() { dispatch(&State::eventX, "eventX"); }
void EmbeddedSystemX::eventY() { dispatch(&State::eventY, "eventY"); }

void EmbeddedSystemX::SelfTestFailed(int errorNo) {
    std::lock_guard<std::mutex> eventLock(eventMutex_);
    std::lock_guard<std::recursive_mutex> stateLock(stateMutex_);
    if (state_) {
        state_->SelfTestFailed(*this, errorNo);
    } else {
        reportIgnored("SelfTestFailed");
    }
}

void EmbeddedSystemX::setMode(State& mode) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    mode_ = &mode;
    output_ << "Mode changed: " << mode.name() << '\n';
}

void EmbeddedSystemX::queueMode1() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    pendingModeChange_ = modeWorker_->chMode1(*this, *mode_);
}

void EmbeddedSystemX::queueMode2() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    pendingModeChange_ = modeWorker_->chMode2(*this, *mode_);
}

void EmbeddedSystemX::respondToModeEventX() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    mode_->eventX(*this);
}

void EmbeddedSystemX::executeModeRequest(State& source,
                                        void (State::*handler)(EmbeddedSystemX&)) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex_);
    (source.*handler)(*this);
}

} // namespace erts
