#pragma once

#include <string_view>

namespace erts {

class EmbeddedSystemX;

// Concrete states override only the events that they accept.
// The context will forward each public event to its current State.
class State {
public:
    virtual ~State() = default;
    virtual std::string_view name() const noexcept = 0;
    virtual void onEntry(EmbeddedSystemX&) {}

    virtual void SelftestOk(EmbeddedSystemX&);
    virtual void SelfTestFailed(EmbeddedSystemX&, int errorNo);
    virtual void Initialized(EmbeddedSystemX&);
    virtual void Restart(EmbeddedSystemX&);
    virtual void Exit(EmbeddedSystemX&);
    virtual void Configure(EmbeddedSystemX&);
    virtual void ConfigX(EmbeddedSystemX&);
    virtual void ConfigurationEnded(EmbeddedSystemX&);
    virtual void Start(EmbeddedSystemX&);
    virtual void Stop(EmbeddedSystemX&);
    virtual void chMode1(EmbeddedSystemX&);
    virtual void chMode2(EmbeddedSystemX&);
    virtual void eventX(EmbeddedSystemX&);
    virtual void eventY(EmbeddedSystemX&);

protected:
    State() = default;
    State(const State&) = delete;
    State& operator=(const State&) = delete;

private:
    void ignored(EmbeddedSystemX&, std::string_view event) const;
};

} // namespace erts
