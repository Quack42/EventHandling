#pragma once

#include "ProcessManager.h"
#include "Subscription.h"
#include "SubscriptionHandle.h"

#include <vector>
#include <functional>
#include <mutex>
#include <memory>


template <typename T>
class EventManager {
private:
	static inline std::vector<T> events;
	static inline std::mutex eventsMutex;

	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptions;
	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptionsToAdd;
	static inline std::mutex subscriptionsToAddMutex;

public:
	static void manage() { 	//NOTE: multi-thread unsafe function.

		// Swap out 'events' list for an empty copy; this way we can handle the events-so-far without worrying what happens if 'addEvents' is called in the midst of handling.
		std::vector<T> _events;
		eventsMutex.lock(); 	// Lock because all 'events' handlings should be locked.
		std::swap(events, _events);
		eventsMutex.unlock();


		// Handle the events.
		for (auto & _event : _events) {
			// Add subscriptions that are to be added.
			_addToAddSubscriptions();

			// Remove subscriptions if they are invalid
			_removeInvalidSubscriptions();

			// Call subscriptions with events
			for (auto & subscription : subscriptions) {
				subscription->call(_event);
			}
		}
	}

	template<typename Func, typename... Bindables>
	static SubscriptionHandle<T> subscribe(Func func, Bindables... bindables){
		// Bind the arguments to make a simple void(void) function call; doing this here because most uses of this function will force the use of bind anyway.
		auto callbackFunction = std::bind(func, bindables..., std::placeholders::_1); 	// Leave a spot open with std::placeholders::_1 for the event type.

		/// Add subscription to list of to-be-added subscriptions; return a SubscriptionHandle<> to the user.
		subscriptionsToAddMutex.lock(); 	// Lock because all interactions with subscriptionsToAdd are mutex protected.
		// Add subscription to list.
		subscriptionsToAdd.emplace_back(std::make_unique<Subscription<T>>(callbackFunction));
		// Get a reference to create a SubscriptionHandle<>
		std::unique_ptr<Subscription<T>> & subscriptionRef_up = subscriptionsToAdd.back();
		// Create a SubscriptionHandle<> to return.
		SubscriptionHandle<T> ret(*subscriptionRef_up);
		subscriptionsToAddMutex.unlock();

		return ret;
	}

	template<typename... Arguments>
	static void addEvent(Arguments... arguments) {
		// Add event to the list of events to handle.
		eventsMutex.lock(); 	// Lock because adding the event to the list and checking for 'needToRequestProcess' needs to happen "atomically".
		events.emplace_back(arguments...);
		bool needToRequestProcess = (events.size() == 1);
		eventsMutex.unlock();

		// If needed, request a process to handle the events that are queued up.
		if (needToRequestProcess) {
			requestManagingProcess();
		}
	}

private:
	static void requestManagingProcess() {
		ProcessManager::requestProcess(&EventManager<T>::manage);
	}

	static void _addToAddSubscriptions() {
		// Swap out 'subscriptionsToAdd' list for an empty copy; this feels more neat and faster.
		std::vector<std::unique_ptr<Subscription<T>>> _subscriptionsToAdd;
		subscriptionsToAddMutex.lock();
		std::swap(subscriptionsToAdd, _subscriptionsToAdd);
		subscriptionsToAddMutex.unlock();
		
		// Insert the to-be-added subscriptions to the subscriptions list.
		subscriptions.reserve(subscriptions.size() + _subscriptionsToAdd.size());
		std::move(_subscriptionsToAdd.begin(), _subscriptionsToAdd.end(), std::back_inserter(subscriptions));
	}

	static void _removeInvalidSubscriptions() {
		subscriptions.erase(
			std::remove_if(
				subscriptions.begin(),
				subscriptions.end(),
				[](std::unique_ptr<Subscription<T>> & subscription) {
					return !subscription->isValid();
				}
			),
			subscriptions.end()
		);
	}

};
