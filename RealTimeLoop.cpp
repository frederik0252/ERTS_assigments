#include "RealTimeLoop.h"
#include "OperationalStates.h"
#include "EmbeddedSystemX.h"
#include <iostream>

//Requests
ChMode1Request::ChMode1Request(RealTimeLoop& loop) : loop_(loop) {}
void ChMode1Request::call() { loop_.getMode().chMode1(loop_); }

ChMode2Request::ChMode2Request(RealTimeLoop& loop) : loop_(loop) {}
void ChMode2Request::call() { loop_.getMode().chMode2(loop_); }

//RealTimeLoop
RealTimeLoop& RealTimeLoop::getInstance() { static RealTimeLoop inst; return inst; }

RealTimeLoop::RealTimeLoop() : currentMode_(&Mode1::getInstance()), running_(false) {}
RealTimeLoop::~RealTimeLoop() { stopWorker(); }

void RealTimeLoop::onEntry(EmbeddedSystemX&) {
    std::cout << "RealTimeLoop started" << std::endl;
    currentMode_ = &Mode1::getInstance();           
    startWorker();
}

void RealTimeLoop::stop(EmbeddedSystemX& system) {
    stopWorker();
    system.setState(Ready::getInstance());
}

void RealTimeLoop::restart(EmbeddedSystemX& system) {
    stopWorker();                                   
    OperationalState::restart(system);                 
}
void RealTimeLoop::chMode1(EmbeddedSystemX&) { enqueue(std::make_unique<ChMode1Request>(*this)); }
void RealTimeLoop::chMode2(EmbeddedSystemX&) { enqueue(std::make_unique<ChMode2Request>(*this)); }
void RealTimeLoop::eventX(EmbeddedSystemX&)  { currentMode_->eventX(*this); }
const char* RealTimeLoop::name() const { return "RealTimeLoop"; }

void RealTimeLoop::setMode(ModeState& m) { currentMode_ = &m; }
ModeState& RealTimeLoop::getMode() const { return *currentMode_; }

void RealTimeLoop::enqueue(std::unique_ptr<MethodRequest> req) {
    {
        std::lock_guard<std::mutex> lock(mutex_);   
        queue_.push(std::move(req));
    }
    cv_.notify_one();
}
void RealTimeLoop::schedulerLoop() {
    while (true) {
        std::unique_ptr<MethodRequest> req;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || !running_; });
            if (!running_) return;                  
            req = std::move(queue_.front());
            queue_.pop();
        }                                           
        req->call();
    }
}

void RealTimeLoop::startWorker() {
    if (running_) return;
    running_ = true;
    worker_ = std::thread(&RealTimeLoop::schedulerLoop, this);
}

void RealTimeLoop::stopWorker() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();
    if (worker_.joinable()) worker_.join();         // wait for the thread to finish
    std::queue<std::unique_ptr<MethodRequest>> empty;
    std::swap(queue_, empty);                       // drop leftover requests
}