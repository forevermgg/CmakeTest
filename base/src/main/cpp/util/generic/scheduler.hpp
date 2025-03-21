#include <thread>

#include "../scheduler.hpp"

namespace base::util {
class GenericScheduler : public base::util::Scheduler {
 public:
  GenericScheduler() = default;

  bool is_on_thread() const noexcept override {
    return m_id == std::this_thread::get_id();
  }
  bool is_same_as(const Scheduler* other) const noexcept override {
    auto o = dynamic_cast<const GenericScheduler*>(other);
    return (o && (o->m_id == m_id));
  }
  bool can_invoke() const noexcept override { return false; }

  void invoke(UniqueFunction<void()>&&) override {}

 private:
  std::thread::id m_id = std::this_thread::get_id();
};
}  // namespace base::util
