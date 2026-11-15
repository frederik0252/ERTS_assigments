#pragma once

#include <iostream>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace erts {

class State;
class RealTimeLoopActiveObject;

class EmbeddedSystemX {
public:
    explicit EmbeddedSystemX(std::ostream& output = std::cout,
                             std::string name = "EmbeddedSystemX", int versionNo = 1);
    ~EmbeddedSystemX();
    EmbeddedSystemX(const EmbeddedSystemX&) = delete;
    EmbeddedSystemX& operator=(const EmbeddedSystemX&) = delete;

    void SelftestOk();
    void SelfTestFailed(int errorNo);
    void Initialized();
    void Restart();
    void Exit();
    void Configure();
    void ConfigX();
    void ConfigurationEnded();
    void Start();
    void Stop();
    void chMode1();
    void chMode2();
    void eventX();
    void eventY();

    std::string_view stateName() const;
    bool terminated() const;
    int errorNo() const;
    int versionNo() const noexcept { return versionNo_; }
    const std::string& name() const noexcept { return name_; }
    void displayState() const;

private:
    friend class State;
    friend class PowerOnSelfTest;
    friend class Initializing;
    friend class Failure;
    friend class Operational;
    friend class Ready;
    friend class Configuration;
    friend class RealTimeLoop;
    friend class Mode1;
    friend class Mode2;
    friend class ModeChangeRequest;

    // Operations used by the state objects. The context owns mutable data;
    // singleton states are shared and must not store per-system values.
    void transitionTo(State& next);
    void recordFailure(int errorNo);
    void terminate();
    void reportIgnored(std::string_view event) const;
    void reportAction(std::string_view action) const;
    void displayError() const;
    void setMode(State& mode);
    void queueMode1();
    void queueMode2();
    void respondToModeEventX();
    void executeModeRequest(State& source, void (State::*handler)(EmbeddedSystemX&));

    void dispatch(void (State::*handler)(EmbeddedSystemX&), std::string_view event);
    std::string_view stateNameUnlocked() const noexcept;

    std::ostream& output_;
    std::string name_;
    int versionNo_;
    int errorNo_ = 0;
    State* state_ = nullptr; // Non-owning: concrete states have static lifetime.
    State* mode_ = nullptr;
    // A public event keeps eventMutex_ until its request completes. The worker
    // only needs stateMutex_; the caller must release that lock before waiting.
    std::mutex eventMutex_;
    mutable std::recursive_mutex stateMutex_;
    std::future<void> pendingModeChange_;
    // Declared last so the worker drains and joins before context data dies.
    std::unique_ptr<RealTimeLoopActiveObject> modeWorker_;
};

} // namespace erts
