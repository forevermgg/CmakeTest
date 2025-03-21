#include "sync_switch.h"

namespace base {

SyncSwitch::Handlers& SyncSwitch::Handlers::SetIfTrue(
    const std::function<void()>& handler) {
  true_handler = handler;
  return *this;
}

SyncSwitch::Handlers& SyncSwitch::Handlers::SetIfFalse(
    const std::function<void()>& handler) {
  false_handler = handler;
  return *this;
}

SyncSwitch::SyncSwitch(bool value)
    : mutex_(std::unique_ptr<base::SharedMutex>(base::SharedMutex::Create())),
      value_(value) {}

void SyncSwitch::Execute(const SyncSwitch::Handlers& handlers) const {
  base::SharedLock lock(*mutex_);
  if (value_) {
    handlers.true_handler();
  } else {
    handlers.false_handler();
  }
}

void SyncSwitch::SetSwitch(bool value) {
  {
    base::UniqueLock lock(*mutex_);
    value_ = value;
  }
  for (Observer* observer : observers_) {
    observer->OnSyncSwitchUpdate(value);
  }
}

void SyncSwitch::AddObserver(Observer* observer) const {
  base::UniqueLock lock(*mutex_);
  if (std::find(observers_.begin(), observers_.end(), observer) ==
      observers_.end()) {
    observers_.push_back(observer);
  }
}

void SyncSwitch::RemoveObserver(Observer* observer) const {
  base::UniqueLock lock(*mutex_);
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer),
                   observers_.end());
}
}  // namespace base
