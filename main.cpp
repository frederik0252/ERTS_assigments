#include "EmbeddedSystemX.h"
#include "RealTimeLoop.h"
#include <iostream>
#include <thread>
#include <chrono>


int main() {
    std::cout << "good path\n";
    EmbeddedSystemX sys;
    sys.displayState();                 // PowerOnSelfTest
    sys.selftestOk();      sys.displayState();   // Initializing
    sys.initialized();     sys.displayState();   // Ready
    sys.configure();       sys.displayState();   // Configuration
    sys.configX();         sys.displayState();   // still Configuration, no re-entry
    sys.configurationEnded(); sys.displayState(); // Ready

    std::cout << "\nRealTimeLoop / active object\n";
    sys.start();           sys.displayState();   // RealTimeLoop
    sys.eventX();                                // mode1
    sys.chMode2();                               // returns immediately
    sys.eventX();                                // probably still mode1 (race!)
    sys.eventX();                                // mode2
    sys.chMode1(); 
    sys.eventX();                                // mode1
    sys.stop();            sys.displayState();   // Ready

    std::cout << "\nInvalid events\n";
    sys.selftestOk();                            // not valid in Ready
    sys.initialized();                           // not valid in Ready
    sys.exit();                                  // not valid in Ready

    std::cout << "\nRestart from substates\n";
    sys.restart();         sys.displayState();   // PowerOnSelfTest
    sys.selftestOk(); sys.initialized(); sys.start();
    sys.restart();         sys.displayState();   // thread stopped, then PowerOnSelfTest

    std::cout << "\nFailure path\n";
    sys.selfTestFailed(101); sys.displayState(); // Failure, "Error: 101"
    sys.restart();         sys.displayState();   // PowerOnSelfTest
    sys.selfTestFailed(102);
    sys.exit();
    return 0;
}
