#pragma once

#include <thread>
#include <mutex>
#include <atomic>

namespace SL {
	namespace Screen_Capture {

		class InterruptableSleeper {
			std::timed_mutex mut_;
			std::atomic_bool lockedByCreator_; // track whether the mutex is locked
			void lock() { // lock mutex
				mut_.lock();
				lockedByCreator_ = true;
			}
			void unlock() { // unlock mutex
				lockedByCreator_ = false;
				mut_.unlock();
			}
		public:
			// lock on creation
			InterruptableSleeper() {
				lock();
			}
			// unlock on destruction, if wake was never called
			~InterruptableSleeper() {
				if (lockedByCreator_) {
					unlock();
				}
			}
			// called by any thread except the creator
			// waits until wake is called or the specified time passes
			template< class Rep, class Period >
			void sleepFor(const std::chrono::duration<Rep, Period>& timeout_duration) {
				if (mut_.try_lock_for(timeout_duration)) {
					// if successfully locked, 
					// remove the lock
					mut_.unlock();
				}
			}
			// unblock any waiting threads, handling a situation
			// where wake has already been called.
			// should only be called by the creating thread
			void wake() {
				if (lockedByCreator_) {
					unlock();
				}
			}
		};

	}
}
