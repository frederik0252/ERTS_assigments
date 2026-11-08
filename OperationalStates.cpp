#include "OperationalStates.h"
#include "StartupStates.h"
#include "RealTimeLoop.h"
#include "EmbeddedSystemX.h"
#include <iostream>

//OperationalState (abstract)
void OperationalState::restart(EmbeddedSystemX& system) { system.setState(PowerOnSelfTest::getInstance()); }

//Ready
Ready& Ready::getInstance() { static Ready inst; return inst; }
void Ready::configure(EmbeddedSystemX& system) { system.setState(Configuration::getInstance()); }
void Ready::start(EmbeddedSystemX& system) { system.setState(RealTimeLoop::getInstance()); }
const char* Ready::name() const { return "Ready"; }

//Configuration
Configuration& Configuration::getInstance() { static Configuration inst; return inst; }
void Configuration::onEntry(EmbeddedSystemX& system) { std::cout << "read configuration" << std::endl; }
void Configuration::configX(EmbeddedSystemX& system) { std::cout << "perform configuration X" << std::endl; }
void Configuration::configurationEnded(EmbeddedSystemX& system) { system.setState(Ready::getInstance()); }
const char* Configuration::name() const { return "Configuration"; }
