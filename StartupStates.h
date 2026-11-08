#pragma once

#include "State.h"

class PowerOnSelfTest : public State {
public:
    static PowerOnSelfTest& getInstance();
    void onEntry(EmbeddedSystemX& system) override;           
    void selftestOk(EmbeddedSystemX& system) override;
    void selfTestFailed(EmbeddedSystemX& system, int errorNo) override;
    const char* name() const override;
private:
    PowerOnSelfTest() = default;
    PowerOnSelfTest(const PowerOnSelfTest&) = delete;
    PowerOnSelfTest& operator=(const PowerOnSelfTest&) = delete;
};

class Initializing : public State {
public:
    static Initializing& getInstance();
    void onEntry(EmbeddedSystemX& system) override;           
    void initialized(EmbeddedSystemX& system) override;
    const char* name() const override;
private:
    Initializing() = default; 
    Initializing(const Initializing&) = delete;
    Initializing& operator=(const Initializing&) = delete;
};

class Failure : public State {
public:
    static Failure& getInstance();
    void onEntry(EmbeddedSystemX& system) override;           
    void restart(EmbeddedSystemX& system) override;
    void exit(EmbeddedSystemX& system) override;
    const char* name() const override;
private:
    Failure() = default;
    Failure(const Failure&) = delete;
    Failure& operator=(const Failure&) = delete;
};
