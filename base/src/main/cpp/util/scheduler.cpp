#include "scheduler.hpp"

#include "android/scheduler.hpp"
#include "functional.hpp"
#include "generic/scheduler.hpp"
#include "uv/scheduler.hpp"

namespace base::util {
namespace {

util::UniqueFunction<std::shared_ptr<Scheduler>()> s_factory =
    &Scheduler::make_platform_default;

class FrozenScheduler : public util::Scheduler {
 public:
  FrozenScheduler(VersionID version) : m_version(version) {}

  void invoke(UniqueFunction<void()>&&) override {}
  bool is_on_thread() const noexcept override { return true; }
  bool is_same_as(const Scheduler* other) const noexcept override {
    auto o = dynamic_cast<const FrozenScheduler*>(other);
    return (o && (o->m_version == m_version));
  }
  bool can_invoke() const noexcept override { return false; }

 private:
  VersionID m_version;
};

class DummyScheduler : public base::util::Scheduler {
 public:
  bool is_on_thread() const noexcept override { return true; }
  bool is_same_as(const Scheduler* other) const noexcept override {
    auto o = dynamic_cast<const DummyScheduler*>(other);
    return (o != nullptr);
  }
  bool can_invoke() const noexcept override { return false; }
  void invoke(UniqueFunction<void()>&&) override {}
};
}  // anonymous namespace

void InvocationQueue::push(util::UniqueFunction<void()>&& fn) {
  std::lock_guard lock(m_mutex);
  m_functions.push_back(std::move(fn));
}

void InvocationQueue::invoke_all() {
  std::vector<util::UniqueFunction<void()>> functions;
  {
    std::lock_guard lock(m_mutex);
    functions.swap(m_functions);
  }
  for (auto&& fn : functions) {
    fn();
  }
}

Scheduler::~Scheduler() = default;

void Scheduler::set_default_factory(
    util::UniqueFunction<std::shared_ptr<Scheduler>()> factory) {
  s_factory = std::move(factory);
}

std::shared_ptr<Scheduler> Scheduler::make_default() { return s_factory(); }

std::shared_ptr<Scheduler> Scheduler::make_platform_default() {
  return make_uv();
}

std::shared_ptr<Scheduler> Scheduler::make_generic() {
  return std::make_shared<GenericScheduler>();
}

std::shared_ptr<Scheduler> Scheduler::make_frozen(VersionID version) {
  return std::make_shared<FrozenScheduler>(version);
}

std::shared_ptr<Scheduler> Scheduler::make_dummy() {
  return std::make_shared<DummyScheduler>();
}

std::shared_ptr<Scheduler> Scheduler::make_alooper() {
  return std::make_shared<ALooperScheduler>();
}

std::shared_ptr<Scheduler> Scheduler::make_uv() {
  return std::make_shared<UvMainLoopScheduler>();
}

}  // namespace base::util
