#pragma once

#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

namespace erts {

class EmbeddedSystemX;
class State;

class Request {
public:
    virtual ~Request() = default;
    std::future<void> completion() { return completed_.get_future(); }
    void execute();
private:
    virtual void invoke() = 0;
    std::promise<void> completed_;
};

// Every call creates its own request, capturing the singleton source state
// and the handler to invoke. The context outlives its requests and worker.
class ModeChangeRequest final : public Request {
public:
    using Handler = void (State::*)(EmbeddedSystemX&);
    ModeChangeRequest(EmbeddedSystemX& context, State& source, Handler handler);
private:
    void invoke() override;
    EmbeddedSystemX& context_;
    State& source_;
    Handler handler_;
};

// The singleton RealTimeLoop state uses this per-context active object.
// Queue, synchronization, and thread belong to a system, not a shared state.
class RealTimeLoopActiveObject {
public:
    RealTimeLoopActiveObject();
    ~RealTimeLoopActiveObject();
    RealTimeLoopActiveObject(const RealTimeLoopActiveObject&) = delete;
    RealTimeLoopActiveObject& operator=(const RealTimeLoopActiveObject&) = delete;

    std::future<void> chMode1(EmbeddedSystemX&, State& source);
    std::future<void> chMode2(EmbeddedSystemX&, State& source);

private:
    std::future<void> enqueue(std::unique_ptr<Request> request);
    void run();

    std::mutex mutex_;
    std::condition_variable available_;
    std::deque<std::unique_ptr<Request>> requests_;
    bool stopping_ = false;
    std::thread worker_;
};

} // namespace erts
