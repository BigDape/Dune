#include "event_loop.h"
#include "log/logger.h"

namespace dune {

EventLoop::EventLoop() {
    base_ = event_base_new();
    if (!base_) {
        LOG_FATAL("Failed to create event_base");
        throw std::runtime_error("event_base_new failed");
    }
    LOG_DEBUG("EventLoop created");
}

EventLoop::~EventLoop() {
    if (base_) {
        event_base_free(base_);
        base_ = nullptr;
        LOG_DEBUG("EventLoop destroyed");
    }
}

int EventLoop::run() {
    LOG_INFO("EventLoop running...");
    int ret = event_base_dispatch(base_);
    if (ret == 0) {
        LOG_INFO("EventLoop exited normally");
    } else if (ret == -1) {
        LOG_ERROR("EventLoop exited with error");
    }
    return ret;
}

void EventLoop::stop() {
    LOG_INFO("EventLoop stop requested");
    event_base_loopbreak(base_);
}

} // namespace dune
