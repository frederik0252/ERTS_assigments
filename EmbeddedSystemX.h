#pragma once

#include "State.h"

class EmbeddedSystemX {
public:
    EmbeddedSystemX(); //constructor

    void selftestOk();
    void selfTestFailed(int errorNo);
    void initialized();
    void restart();
    void exit();
    void configure();
    void configX();
    void configurationEnded();
    void start();      
    void stop();
    void chMode1();
    void chMode2();
    void eventX();
    void eventY();

    void setState(State& newState);  
    State& getState() const;
    void displayState() const;
    int  getErrorNo() const;
    void setErrorNo(int errorNo);

private:
    State* currentState_;   
    int    versionNo_;
    const char* name_;
    int    errorNo_;
};
