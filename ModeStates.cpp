#include "ModeStates.h"
#include "RealTimeLoop.h"
#include <iostream>

static void notValid(const ModeState& m, const char* event) {
    std::cout << event << "() not valid in " << m.name() << std::endl;
}

void ModeState::chMode1(RealTimeLoop&) { notValid(*this, "chMode1"); }
void ModeState::chMode2(RealTimeLoop&) { notValid(*this, "chMode2"); }
void ModeState::eventX(RealTimeLoop&) { notValid(*this, "eventX"); }

//Mode1
Mode1& Mode1::getInstance() { static Mode1 inst; return inst; }
void Mode1::chMode2(RealTimeLoop& loop) { loop.setMode(Mode2::getInstance()); }

void Mode1::eventX(RealTimeLoop& system) { std::cout << "eventX:mode1 active" << std::endl; }
const char* Mode1::name() const { return "Mode1"; }

//Mode2 
Mode2& Mode2::getInstance() { static Mode2 inst; return inst; }
void Mode2::chMode1(RealTimeLoop& loop) { loop.setMode(Mode1::getInstance()); }
void Mode2::eventX(RealTimeLoop& system) { std::cout << "eventX:mode2 active" << std::endl; }
const char* Mode2::name() const { return "Mode2"; }
