#pragma once


class EmbeddedSystemX;   // forward declaration

class State {
public:
    virtual ~State() = default;

    virtual void onEntry(EmbeddedSystemX& system);

    // Events from the state diagram
    virtual void selftestOk(EmbeddedSystemX& system);
    virtual void selfTestFailed(EmbeddedSystemX& system, int errorNo);
    virtual void initialized(EmbeddedSystemX& system);
    virtual void restart(EmbeddedSystemX& system);
    virtual void exit(EmbeddedSystemX& system);
    virtual void configure(EmbeddedSystemX& system);
    virtual void configX(EmbeddedSystemX& system);
    virtual void configurationEnded(EmbeddedSystemX& system);
    virtual void start(EmbeddedSystemX& system);
    virtual void stop(EmbeddedSystemX& system);

    // Events handled inside RealTimeLoop
    virtual void chMode1(EmbeddedSystemX& system);
    virtual void chMode2(EmbeddedSystemX& system);
    virtual void eventX(EmbeddedSystemX& system);
    virtual void eventY(EmbeddedSystemX& system);

    // For the test/display operation
    virtual const char* name() const = 0;
};
