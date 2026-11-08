#pragma once

#include "State.h"

class OperationalState : public State {
public:
    void restart(EmbeddedSystemX& system) override;   // common to all substates
protected:
    OperationalState() = default;
};

class Ready : public OperationalState {
public:
    static Ready& getInstance();
    void configure(EmbeddedSystemX& system) override;
    void start(EmbeddedSystemX& system) override;
    const char* name() const override;
private:
    Ready() = default;
    Ready(const Ready&) = delete;
    Ready& operator=(const Ready&) = delete;
};

class Configuration : public OperationalState {
public:
    static Configuration& getInstance();
    void onEntry(EmbeddedSystemX& system) override;   
    void configX(EmbeddedSystemX& system) override;   
    void configurationEnded(EmbeddedSystemX& system) override;
    const char* name() const override;
private:
    Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;
};
