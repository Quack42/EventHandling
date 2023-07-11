#pragma once

#include <vector>
#include <functional>
#include <mutex>
#include <memory>

#include "ProcessManager.h"

template <typename T>
class Subscription {
private:
	std::function<void(T&)> subscriberFunction;
	unsigned int subscriptionHandles;

	std::recursive_mutex deletionDelayMutex;


public:
	Subscription(std::function<void(T&)> subscriberFunction) :
			subscriberFunction(subscriberFunction),
			subscriptionHandles(0)
	{

	}

	void incrementSubscriptionHandles() {
		subscriptionHandles++;
	}

	void decrementSubscriptionHandles() {
		subscriptionHandles--;
		if (subscriptionHandles == 0) {
			invalidate();
		}
	}

	void invalidate() {
		deletionDelayMutex.lock();
		subscriberFunction = std::function<void(T&)>();
		deletionDelayMutex.unlock();
	}

	bool isValid() {
		return !!subscriberFunction;
	}

	void call(T & event) {
		deletionDelayMutex.lock();
		if (isValid()) {
			subscriberFunction(event);
		}
		deletionDelayMutex.unlock();
	}
};



template <typename T>
class SubscriptionHandle {
private:
	Subscription<T> & subscription;
public:
	virtual ~SubscriptionHandle() {
		subscription.decrementSubscriptionHandles();
	}

	SubscriptionHandle(Subscription<T> & subscription) :
			subscription(subscription)
	{
		subscription.incrementSubscriptionHandles();
	}

	SubscriptionHandle(SubscriptionHandle & other) :
			subscription(other.subscription)
	{
		subscription.incrementSubscriptionHandles();
	}


	SubscriptionHandle & operator=(SubscriptionHandle & rhs) {
		this->subscription.decrementSubscriptionHandles();
		this->subscription = rhs.subscription;
		rhs.subscription.incrementSubscriptionHandles();
		return *this;
	}

	void deletingHost() {
		subscription.invalidate();
	}
};




template <typename T>
class EventManager {
private:
	static inline std::vector<T> events;
	static inline std::mutex eventsMutex;

	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptions; 	//TODO: make recursion (and thread) safe.
	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptionsToAdd; 	//TODO: make recursion (and thread) safe.
	static inline std::mutex subscriptionsToAddMutex;

public:
	static void manage() {
		// Swap out 'events' list for an empty copy; this way we can handle the events-so-far without worrying what happens if 'addEvents' is called in the midst of handling.
		std::vector<T> _events;
		eventsMutex.lock(); 	// Lock because all 'events' handlings should be locked.
		std::swap(events, _events);
		eventsMutex.unlock();


		// Handle the events.
		for (auto & _event : _events) {
			// Add subscriptions that are to be added.
			_addToAddSubscribers();
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
		std::unique_ptr<Subscription<T>> & subscriberRef_up = subscriptionsToAdd.back();
		// Create a SubscriptionHandle<> to return.
		SubscriptionHandle<T> ret(*subscriberRef_up);
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

	static void _addToAddSubscribers() {
		// Swap out 'subscriptionsToAdd' list for an empty copy; this feels more neat and faster.
		std::vector<std::unique_ptr<Subscription<T>>> _subscribersToAdd;
		subscriptionsToAddMutex.lock();
		std::swap(subscriptionsToAdd, _subscribersToAdd);
		subscriptionsToAddMutex.unlock();
		
		// Insert the to-be-added subscriptions to the subscriptions list.
		subscriptions.reserve(subscriptions.size() + _subscribersToAdd.size());
		std::move(_subscribersToAdd.begin(), _subscribersToAdd.end(), std::back_inserter(subscriptions));
	}
};


