#include "StartupStates.h"
#include "OperationalStates.h"
#include "EmbeddedSystemX.h"
#include <iostream>
#include <thread>
#include <chrono>

// Creates the instance (object)
PowerOnSelfTest& PowerOnSelfTest::getInstance() { static PowerOnSelfTest inst; return inst; }

// do system self test
void PowerOnSelfTest::onEntry(EmbeddedSystemX& system) {

    std::cout << "Running system self test..." << std::endl;
    //bool testPassed = true; // Replace with your actual self-test check
    //if (testPassed) {
    //    selftestOk(system);
    //}
    //else {
    //    selfTestFailed(system, 101); // Pass relevant error code
    //}


}

void PowerOnSelfTest::selftestOk(EmbeddedSystemX& system) { system.setState(Initializing::getInstance()); }

void PowerOnSelfTest::selfTestFailed(EmbeddedSystemX& system, int errorNo) {
    system.setErrorNo(errorNo);
    system.setState(Failure::getInstance());
}
const char* PowerOnSelfTest::name() const { return "PowerOnSelfTest"; }



Initializing& Initializing::getInstance() { static Initializing inst; return inst; }

void Initializing::onEntry(EmbeddedSystemX& system) {

    std::cout << "starting initializising" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "initializising done" << std::endl;
   
}

void Initializing::initialized(EmbeddedSystemX& system) {
   
    system.setState(Ready::getInstance()); 
}

const char* Initializing::name() const { return "Initializing"; }


Failure& Failure::getInstance() { static Failure inst; return inst; }

void Failure::onEntry(EmbeddedSystemX& system) {
    int error = 0;
    error = system.getErrorNo();
    std::cout << "Error: " << error << std::endl;
    
}

void Failure::restart(EmbeddedSystemX& system) {
    system.setState(PowerOnSelfTest::getInstance());
}

void Failure::exit(EmbeddedSystemX& system) {
    std::cout << "System shutting down" << std::endl;
    
}

const char* Failure::name() const { return "Failure"; }