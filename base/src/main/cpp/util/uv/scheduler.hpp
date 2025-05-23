#include <uv.h>

#include <atomic>
#include <thread>

#include "../scheduler.hpp"

namespace base::util {

class UvMainLoopScheduler final : public util::Scheduler {
 public:
  UvMainLoopScheduler() : m_handle(std::make_unique<uv_async_t>()) {
    // This only supports running on the default loop, i.e. the main thread.
    // This suffices for node and for our tests, but in the future we may
    // need a way to pass in a target loop.
    int err = uv_async_init(
        uv_default_loop(), m_handle.get(), [](uv_async_t* handle) {
          if (!handle->data) {
            return;
          }
          auto& data = *static_cast<Data*>(handle->data);
          if (data.close_requested) {
            uv_close(reinterpret_cast<uv_handle_t*>(handle),
                     [](uv_handle_t* handle) {
                       delete reinterpret_cast<Data*>(handle->data);
                       delete reinterpret_cast<uv_async_t*>(handle);
                     });
          } else {
            data.queue.invoke_all();
          }
        });
    if (err < 0) {
      throw std::runtime_error("uv_async_init failed: " +
                               std::string(uv_strerror(err)));
    }
    m_handle->data = new Data;
  }

  ~UvMainLoopScheduler() {
    if (m_handle && m_handle->data) {
      static_cast<Data*>(m_handle->data)->close_requested = true;
      uv_async_send(m_handle.get());
      // Don't delete anything here as we need to delete it from within the
      // event loop instead
      m_handle.release();
    }
  }

  bool is_on_thread() const noexcept override {
    return m_id == std::this_thread::get_id();
  }
  bool is_same_as(const Scheduler* other) const noexcept override {
    auto o = dynamic_cast<const UvMainLoopScheduler*>(other);
    return (o && (o->m_id == m_id));
  }
  bool can_invoke() const noexcept override { return true; }

  void invoke(util::UniqueFunction<void()>&& fn) override {
    auto& data = *static_cast<Data*>(m_handle->data);
    data.queue.push(std::move(fn));
    uv_async_send(m_handle.get());
  }

 private:
  struct Data {
    InvocationQueue queue;
    std::atomic<bool> close_requested = {false};
  };
  std::unique_ptr<uv_async_t> m_handle;
  std::thread::id m_id = std::this_thread::get_id();
};

}  // namespace base::util
