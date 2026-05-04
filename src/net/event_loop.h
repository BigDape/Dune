#ifndef DUNE_NET_EVENT_LOOP_H_
#define DUNE_NET_EVENT_LOOP_H_

#include <event2/event.h>

namespace dune {

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    // 非拷贝
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // 启动事件循环（阻塞），返回 0 正常退出，-1 出错
    int run();

    // 停止事件循环（线程安全）
    void stop();

    // 获取底层 event_base，供其他模块使用
    struct event_base* base() const { return base_; }

private:
    struct event_base* base_ = nullptr;
};

} // namespace dune

#endif // DUNE_NET_EVENT_LOOP_H_
