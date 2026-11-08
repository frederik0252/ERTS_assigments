#include "State.h"
#include "EmbeddedSystemX.h"
#include <iostream>




// Static function for easier validation 
static void notValid(const State& s, const char* event) 
{
    std::cout << event << "() is invalid state " << s.name() << std::endl;
}

void State::onEntry(EmbeddedSystemX&) 
{ 
     
}

void State::selftestOk(EmbeddedSystemX&) 
{ 
    notValid(*this, "Selftestok"); 
}

void State::selfTestFailed(EmbeddedSystemX&, int) 
{ 
    notValid(*this, "selfTestFailed"); 
}

void State::initialized(EmbeddedSystemX&) 
{
    notValid(*this, "Initialized");
}

void State::restart(EmbeddedSystemX&) 
{
    notValid(*this, "restart");
}

void State::exit(EmbeddedSystemX&) 
{
    notValid(*this, "exit");
}

void State::configure(EmbeddedSystemX&) 
{
    notValid(*this, "configure");
}

void State::configX(EmbeddedSystemX&) 
{
    notValid(*this, "configX");
}

void State::configurationEnded(EmbeddedSystemX&) 
{
    notValid(*this, "configurationEnded");
}

void State::start(EmbeddedSystemX&) 
{
    notValid(*this, "start");
}

void State::stop(EmbeddedSystemX&)
{
    notValid(*this, "stop");
}

void State::chMode1(EmbeddedSystemX&) 
{
    notValid(*this, "chmode1");
}

void State::chMode2(EmbeddedSystemX&) 
{
    notValid(*this, "chmode2");
}

void State::eventX(EmbeddedSystemX&) 
{
    notValid(*this, "eventX");
}

void State::eventY(EmbeddedSystemX&) 
{
    notValid(*this, "eventY");
}
