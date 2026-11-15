#include "State.hpp"

#include "EmbeddedSystemX.hpp"

namespace erts {

void State::ignored(EmbeddedSystemX& context, std::string_view event) const {
    context.reportIgnored(event);
}

// These defaults do not modify the context or perform a transition.
// An accepted event gets its own override in the relevant concrete state.
void State::SelftestOk(EmbeddedSystemX& c) { ignored(c, "SelftestOk"); }
void State::SelfTestFailed(EmbeddedSystemX& c, int) { ignored(c, "SelfTestFailed"); }
void State::Initialized(EmbeddedSystemX& c) { ignored(c, "Initialized"); }
void State::Restart(EmbeddedSystemX& c) { ignored(c, "Restart"); }
void State::Exit(EmbeddedSystemX& c) { ignored(c, "Exit"); }
void State::Configure(EmbeddedSystemX& c) { ignored(c, "Configure"); }
void State::ConfigX(EmbeddedSystemX& c) { ignored(c, "ConfigX"); }
void State::ConfigurationEnded(EmbeddedSystemX& c) { ignored(c, "ConfigurationEnded"); }
void State::Start(EmbeddedSystemX& c) { ignored(c, "Start"); }
void State::Stop(EmbeddedSystemX& c) { ignored(c, "Stop"); }
void State::chMode1(EmbeddedSystemX& c) { ignored(c, "chMode1"); }
void State::chMode2(EmbeddedSystemX& c) { ignored(c, "chMode2"); }
void State::eventX(EmbeddedSystemX& c) { ignored(c, "eventX"); }
void State::eventY(EmbeddedSystemX& c) { ignored(c, "eventY"); }

} // namespace erts
