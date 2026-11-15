#include "RealTimeLoopActiveObject.hpp"
#include "EmbeddedSystemX.hpp"
#include "State.hpp"

#include <stdexcept>

namespace erts {

void Request::execute() {
    try {
        invoke();
        completed_.set_value();
    } catch (...) {
        completed_.set_exception(std::current_exception());
    }
}

ModeChangeRequest::ModeChangeRequest(EmbeddedSystemX& context, State& source,
                                   Handler handler)
    : context_(context), source_(source), handler_(handler) {}

void ModeChangeRequest::invoke() {
    context_.executeModeRequest(source_, handler_);
}

RealTimeLoopActiveObject::RealTimeLoopActiveObject()
    : worker_(&RealTimeLoopActiveObject::run, this) {}

RealTimeLoopActiveObject::~RealTimeLoopActiveObject() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    available_.notify_one();
    worker_.join();
}

std::future<void> RealTimeLoopActiveObject::chMode1(EmbeddedSystemX& context,
                                                  State& source) {
    return enqueue(std::make_unique<ModeChangeRequest>(context, source, &State::chMode1));
}

std::future<void> RealTimeLoopActiveObject::chMode2(EmbeddedSystemX& context,
                                                  State& source) {
    return enqueue(std::make_unique<ModeChangeRequest>(context, source, &State::chMode2));
}

std::future<void> RealTimeLoopActiveObject::enqueue(std::unique_ptr<Request> request) {
    auto done = request->completion();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_) throw std::runtime_error("RealTimeLoop worker is stopping");
        requests_.push_back(std::move(request));
    }
    available_.notify_one();
    return done;
}

void RealTimeLoopActiveObject::run() {
    for (;;) {
        std::unique_ptr<Request> request;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            available_.wait(lock, [this] { return stopping_ || !requests_.empty(); });
            if (requests_.empty()) return;
            request = std::move(requests_.front());
            requests_.pop_front();
        }
        request->execute();
        // Unique ownership deletes the request after execution. On shutdown,
        // continue processing until all previously accepted requests finish.
    }
}

} // namespace erts
