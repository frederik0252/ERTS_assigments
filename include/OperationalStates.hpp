#pragma once

#include "StartupStates.hpp"

namespace erts {

class Configuration final : public Operational {
public:
    static Configuration& instance();
    std::string_view name() const noexcept override { return "Operational/Configuration"; }
    void onEntry(EmbeddedSystemX&) override;
    void ConfigX(EmbeddedSystemX&) override;
    void ConfigurationEnded(EmbeddedSystemX&) override;
private:
    Configuration() = default;
};

class RealTimeLoop final : public Operational {
public:
    static RealTimeLoop& instance();
    std::string_view name() const noexcept override { return "Operational/RealTimeLoop"; }
    void onEntry(EmbeddedSystemX&) override;
    void Stop(EmbeddedSystemX&) override;
    void chMode1(EmbeddedSystemX&) override;
    void chMode2(EmbeddedSystemX&) override;
    void eventX(EmbeddedSystemX&) override;
private:
    RealTimeLoop() = default;
};

class Mode1 final : public State {
public:
    static Mode1& instance();
    std::string_view name() const noexcept override { return "Operational/RealTimeLoop/Mode1"; }
    void chMode2(EmbeddedSystemX&) override;
    void eventX(EmbeddedSystemX&) override;
private:
    Mode1() = default;
};

class Mode2 final : public State {
public:
    static Mode2& instance();
    std::string_view name() const noexcept override { return "Operational/RealTimeLoop/Mode2"; }
    void chMode1(EmbeddedSystemX&) override;
    void eventX(EmbeddedSystemX&) override;
private:
    Mode2() = default;
};

} // namespace erts
