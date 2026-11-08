#pragma once

#include "OperationalStates.h"
#include "ModeStates.h"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>


class RealTimeLoop;



class MethodRequest {
public:
    virtual ~MethodRequest() = default;
    virtual void call() = 0;        
};

class ChMode1Request : public MethodRequest {
public:
    explicit ChMode1Request(RealTimeLoop& loop);
    void call() override;
private:
    RealTimeLoop& loop_;
};

class ChMode2Request : public MethodRequest {
public:
    explicit ChMode2Request(RealTimeLoop& loop);
    void call() override;
private:
    RealTimeLoop& loop_;
};


class RealTimeLoop : public OperationalState {
public:
    static RealTimeLoop& getInstance();

    // State interface (outer machine)
    void onEntry(EmbeddedSystemX& system) override;   
    void stop(EmbeddedSystemX& system) override;      
    void restart(EmbeddedSystemX& system) override;   
    void chMode1(EmbeddedSystemX& system) override;  
    void chMode2(EmbeddedSystemX& system) override;   
    void eventX(EmbeddedSystemX& system) override;    
    const char* name() const override;

    
    void setMode(ModeState& m);
    ModeState& getMode() const;

private:
    RealTimeLoop();
    ~RealTimeLoop();
    RealTimeLoop(const RealTimeLoop&) = delete;
    RealTimeLoop& operator=(const RealTimeLoop&) = delete;
    void startWorker();
    void stopWorker();
    void enqueue(std::unique_ptr<MethodRequest> req);
    void schedulerLoop();                          

    ModeState*  currentMode_;                      
    std::queue<std::unique_ptr<MethodRequest>> queue_;
    std::mutex  mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    bool        running_;
};
