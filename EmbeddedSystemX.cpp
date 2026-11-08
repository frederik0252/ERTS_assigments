#include "EmbeddedSystemX.h"
#include "StartupStates.h"
#include <iostream>

EmbeddedSystemX::EmbeddedSystemX()
    : currentState_(nullptr), versionNo_(1), name_("EmbeddedSystemX"), errorNo_(0)
{
    setState(PowerOnSelfTest::getInstance());
}

// No logic, just set currentstate_ to the state
void EmbeddedSystemX::selftestOk() {currentState_->selftestOk(*this); }
void EmbeddedSystemX::selfTestFailed(int error) { currentState_->selfTestFailed(*this, error); }
void EmbeddedSystemX::initialized() { currentState_->initialized(*this); }
void EmbeddedSystemX::restart() { currentState_->restart(*this); }
void EmbeddedSystemX::exit() { currentState_->exit(*this); }
void EmbeddedSystemX::configure() { currentState_->configure(*this); }
void EmbeddedSystemX::configX() { currentState_->configX(*this); }
void EmbeddedSystemX::configurationEnded() { currentState_->configurationEnded(*this); }
void EmbeddedSystemX::start() { currentState_->start(*this); }
void EmbeddedSystemX::stop() { currentState_->stop(*this); }
void EmbeddedSystemX::chMode1() { currentState_->chMode1(*this); }
void EmbeddedSystemX::chMode2() { currentState_->chMode2(*this); }
void EmbeddedSystemX::eventX() { currentState_->eventX(*this); }
void EmbeddedSystemX::eventY() { currentState_->eventY(*this); }

void EmbeddedSystemX::setState(State& s) 
{
    currentState_ = &s;
    currentState_->onEntry(*this);
}
State& EmbeddedSystemX::getState() const { return *currentState_; }
void EmbeddedSystemX::displayState() const {
    std::cout << name_ << " v" << versionNo_ << " - Current state: "
              << currentState_->name() << std::endl;
}
int  EmbeddedSystemX::getErrorNo() const { return errorNo_; }
void EmbeddedSystemX::setErrorNo(int e) { errorNo_ = e; }
