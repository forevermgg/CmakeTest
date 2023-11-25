#include "event_loop.hpp"

#include <uv.h>

#include <mutex>
#include <stdexcept>
#include <vector>

#include "event_loop_dispatcher.hpp"
#include "scope_exit.hpp"

using namespace base::util;

struct EventLoop::Impl {
  // Returns the main event loop.
  static std::unique_ptr<Impl> main();

  // Run the event loop until the given return predicate returns true
  void run_until(util::FunctionRef<bool()> predicate);

  // Schedule execution of the given function on the event loop.
  void perform(util::UniqueFunction<void()>);

  // Run the event loop until all currently pending work has been run.
  void run_pending();

  ~Impl();

 private:
  Impl(uv_loop_t* loop);

  std::vector<util::UniqueFunction<void()>> m_pending_work;
  std::mutex m_mutex;
  uv_loop_t* m_loop;
  uv_async_t m_perform_work;
};

EventLoop& EventLoop::main() {
  static EventLoop main(Impl::main());
  return main;
}

EventLoop::EventLoop(std::unique_ptr<Impl> impl) : m_impl(std::move(impl)) {}

EventLoop::~EventLoop() = default;

void EventLoop::run_until(util::FunctionRef<bool()> predicate) {
  return m_impl->run_until(predicate);
}

void EventLoop::perform(util::UniqueFunction<void()> function) {
  return m_impl->perform(std::move(function));
}

void EventLoop::run_pending() { return m_impl->run_pending(); }

bool EventLoop::has_implementation() { return true; }

std::unique_ptr<EventLoop::Impl> EventLoop::Impl::main() {
  return std::unique_ptr<Impl>(new Impl(uv_default_loop()));
}

EventLoop::Impl::Impl(uv_loop_t* loop) : m_loop(loop) {
  m_perform_work.data = this;
  uv_async_init(uv_default_loop(), &m_perform_work, [](uv_async_t* handle) {
    std::vector<util::UniqueFunction<void()>> pending_work;
    {
      Impl& self = *static_cast<Impl*>(handle->data);
      std::lock_guard<std::mutex> lock(self.m_mutex);
      std::swap(pending_work, self.m_pending_work);
    }

    for (auto& f : pending_work) f();
  });
}

EventLoop::Impl::~Impl() {
  uv_close((uv_handle_t*)&m_perform_work, [](uv_handle_t*) {});
  uv_loop_close(m_loop);
}

struct IdleHandler {
  uv_idle_t* idle = new uv_idle_t;

  IdleHandler(uv_loop_t* loop) { uv_idle_init(loop, idle); }
  ~IdleHandler() {
    uv_close(reinterpret_cast<uv_handle_t*>(idle), [](uv_handle_t* handle) {
      delete reinterpret_cast<uv_idle_t*>(handle);
    });
  }
};

void EventLoop::Impl::run_until(util::FunctionRef<bool()> predicate) {
  if (predicate()) return;

  IdleHandler observer(m_loop);
  observer.idle->data = &predicate;

  uv_idle_start(observer.idle, [](uv_idle_t* handle) {
    auto& predicate = *static_cast<util::FunctionRef<bool()>*>(handle->data);
    if (predicate()) {
      uv_stop(handle->loop);
    }
  });

  auto cleanup =
      make_scope_exit([&]() noexcept { uv_idle_stop(observer.idle); });
  uv_run(m_loop, UV_RUN_DEFAULT);
}

void EventLoop::Impl::perform(util::UniqueFunction<void()> f) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending_work.push_back(std::move(f));
  }
  uv_async_send(&m_perform_work);
}

void EventLoop::Impl::run_pending() { uv_run(m_loop, UV_RUN_NOWAIT); }