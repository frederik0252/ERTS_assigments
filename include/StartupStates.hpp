#pragma once

#include "State.hpp"

namespace erts {

class PowerOnSelfTest final : public State {
public:
    static PowerOnSelfTest& instance();
    std::string_view name() const noexcept override { return "PowerOnSelfTest"; }
    void onEntry(EmbeddedSystemX&) override;
    void SelftestOk(EmbeddedSystemX&) override;
    void SelfTestFailed(EmbeddedSystemX&, int errorNo) override;
private:
    PowerOnSelfTest() = default;
};

class Initializing final : public State {
public:
    static Initializing& instance();
    std::string_view name() const noexcept override { return "Initializing"; }
    void onEntry(EmbeddedSystemX&) override;
    void Initialized(EmbeddedSystemX&) override;
private:
    Initializing() = default;
};

class Failure final : public State {
public:
    static Failure& instance();
    std::string_view name() const noexcept override { return "Failure"; }
    void onEntry(EmbeddedSystemX&) override;
    void Restart(EmbeddedSystemX&) override;
    void Exit(EmbeddedSystemX&) override;
private:
    Failure() = default;
};

// Entering this composite state immediately enters its initial substate Ready.
// Operational substates inherit its Restart behavior.
class Operational : public State {
public:
    static Operational& instance();
    std::string_view name() const noexcept override { return "Operational"; }
    void onEntry(EmbeddedSystemX&) override;
    void Restart(EmbeddedSystemX&) override;
protected:
    Operational() = default;
};

class Ready final : public Operational {
public:
    static Ready& instance();
    std::string_view name() const noexcept override { return "Operational/Ready"; }
    void onEntry(EmbeddedSystemX&) override {}
    void Configure(EmbeddedSystemX&) override;
    void Start(EmbeddedSystemX&) override;
private:
    Ready() = default;
};

} // namespace erts
