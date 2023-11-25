/**
 * A generic interface to dispatch callbacks.
 *
 * All APIs that take a callback as argument also take a
 * CallbackHandler* which is used to dispatch the
 * callback: CallbackHandler::post() method is called from a service thread as
 * soon as possible (this will NEVER be the main thread),
 * CallbackHandler::post() is  responsible for scheduling the callback onto the
 * thread the user desires.
 *
 * This is intended to make callbacks interoperate with
 * the platform/OS's own messaging system.
 *
 * CallbackHandler* can always be nullptr in which case the default handler is
 * used. The default handler always dispatches callbacks on filament's main
 * thread opportunistically.
 *
 * Life time:
 * ---------
 *
 * Filament make no attempts to manage the life time of the CallbackHandler* and
 * never takes ownership. In particular, this means that the CallbackHandler
 * instance must stay valid until all pending callbacks are been dispatched.
 *
 * Similarly, when shutting down filament, care must be taken to ensure that all
 * pending callbacks that might access filament's state have been dispatched.
 * Filament can no longer ensure this because callback execution is the
 * responsibility of the CallbackHandler, which is external to filament.
 * Typically, the concrete CallbackHandler would have a mechanism to drain
 * and/or wait for all callbacks to be processed.
 *
 */
#pragma once
class CallbackHandler {
 public:
  using Callback = void (*)(void* user);

  /**
   * Schedules the callback to be called onto the appropriate thread.
   * Typically this will be the application's main thead.
   *
   * Must be thread-safe.
   */
  virtual void post(void* user, Callback callback) = 0;

 protected:
  virtual ~CallbackHandler();
};