#pragma once

#include "Key.h"
#include "Subscription.h"
#include "SubscriptionHandle.h"
#include "ProcessManager.h"
#include "Phase.h"
#include "PhaseManager.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>


template <typename T>
class EventManager {
private:
	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptions;
	static inline std::vector<std::unique_ptr<Subscription<T>>> subscriptionsToAdd;
	static inline std::mutex subscriptionsToAddMutex;

	static inline std::unordered_map<Key, std::vector<std::unique_ptr<Subscription<T>>>> keyedSubscriptionsMap;
	static inline std::vector<std::pair<Key, std::unique_ptr<Subscription<T>>>> keyedSubscriptionsToAdd;
	static inline std::mutex keyedSubscriptionsToAddMutex;

public:
	static void manageEvent(T event) {
		// Add subscriptions that are to be added.
		_addToAddSubscriptions();

		// Remove subscriptions if they are invalid
		_removeInvalidSubscriptions();

		// Call subscriptions with events
		for (auto & subscription : subscriptions) {
			subscription->call(event);
		}
	}

	static void manageKeyedEvent(Key key, T event) {
		// Add subscriptions that are to be added.
		_addToAddKeyedSubscriptions();

		// Remove subscriptions if they are invalid
		_removeInvalidKeyedSubscriptions();

		// Call keyed subscriptions with events
		try {
			auto & subscriptionsForKey = keyedSubscriptionsMap.at(key);
			for (auto & subscriptionForKey : subscriptionsForKey) {
				subscriptionForKey->call(event);
			}
		} catch(const std::out_of_range & oor) {
			//nothing here.
		}
	}


	template<typename Func, typename... Bindables>
	static Subscription<T> & subscribeRaw(Func func, Bindables... bindables){
		// Bind the arguments to make a simple void(void) function call; doing this here because most uses of this function will force the use of bind anyway.
		auto callbackFunction = std::bind(func, bindables..., std::placeholders::_1); 	// Leave a spot open with std::placeholders::_1 for the event type.

		/// Add subscription to list of to-be-added subscriptions; return a SubscriptionHandle<> to the user.
		subscriptionsToAddMutex.lock(); 	// Lock because all interactions with subscriptionsToAdd are mutex protected.
		// Add subscription to list.
		subscriptionsToAdd.emplace_back(std::make_unique<Subscription<T>>(callbackFunction));
		// Get a reference to return.
		std::unique_ptr<Subscription<T>> & subscriptionRef_up = subscriptionsToAdd.back();
		Subscription<T> & subscriptionRef = *subscriptionRef_up;
		subscriptionsToAddMutex.unlock();

		return subscriptionRef;
	}

	template<typename Func, typename... Bindables>
	static SubscriptionHandle<T> subscribe(Func func, Bindables... bindables){
		// Subscribe and get a reference create a SusbcriptionHandle<>.
		Subscription<T> & subscriptionRef = subscribeRaw(func, bindables...);

		// Create a SubscriptionHandle<> to return.
		SubscriptionHandle<T> ret(subscriptionRef);

		return ret;
	}

	template<typename Func, typename KeyInputType, typename... Bindables>
	static SubscriptionHandle<T> keyedSubscribe(Func func, KeyInputType keyInput, Bindables... bindables){
		// Bind the arguments to make a simple void(void) function call; doing this here because most uses of this function will force the use of bind anyway.
		auto callbackFunction = std::bind(func, bindables..., std::placeholders::_1); 	// Leave a spot open with std::placeholders::_1 for the event type.

		/// Add subscription to list of to-be-added subscriptions; return a SubscriptionHandle<> to the user.
		keyedSubscriptionsToAddMutex.lock(); 	// Lock because all interactions with subscriptionsToAdd are mutex protected.
		// Add subscription to list.
		keyedSubscriptionsToAdd.emplace_back(Key(keyInput), std::make_unique<Subscription<T>>(callbackFunction));
		// Get a reference to create a SubscriptionHandle<>
		std::unique_ptr<Subscription<T>> & keyedSubscriptionRef_up = std::get<1>(keyedSubscriptionsToAdd.back()); 	//get the second item in the pair
		// Create a SubscriptionHandle<> to return.
		SubscriptionHandle<T> ret(*keyedSubscriptionRef_up);
		keyedSubscriptionsToAddMutex.unlock();

		return ret;
	}

	template<typename... Arguments>
	static void addEvent(Arguments... arguments) {
		requestManagingProcessForEvent(T(arguments...));
	}


	template<typename KeyInputType, typename... Arguments>
	static void addKeyedEvent(KeyInputType keyInput, Arguments... arguments) {
		requestManagingProcessForEvent(T(arguments...));
		requestManagingProcessForKeyedEvent(Key(keyInput), T(arguments...));
	}

	template<typename... Arguments>
	static void addPhasedEvent(PhaseID phaseID, unsigned int offset, Arguments... arguments) {
		//offset: 0 -> NOW
		//offset: 1 -> NEXT RUN
		// etc.
		std::function<void(void)> eventManagementFunction = std::bind(&EventManager<T>::manageEvent, T(arguments...));
		PhaseManager::registerEventCallback(phaseID, offset, eventManagementFunction);
	}

private:
	static void requestManagingProcessForEvent(const T & event) {
		ProcessManager::requestProcess(&EventManager<T>::manageEvent, event);
	}

	static void requestManagingProcessForKeyedEvent(const Key & key, const T & event) {
		ProcessManager::requestProcess(&EventManager<T>::manageKeyedEvent, key, event);
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

	static void _addToAddKeyedSubscriptions() {
		// Swap out 'keyedSubscriptionsToAdd' list for an empty copy; this feels more neat and faster.
		std::vector<std::pair<Key, std::unique_ptr<Subscription<T>>>> _keyedSubscriptionsToAdd;
		keyedSubscriptionsToAddMutex.lock();
		std::swap(keyedSubscriptionsToAdd, _keyedSubscriptionsToAdd);
		keyedSubscriptionsToAddMutex.unlock();

		// Insert the to-be-added subscriptions to the subscriptions list.
		for (std::pair<Key, std::unique_ptr<Subscription<T>>> & _keyedSubscriptionToAdd : _keyedSubscriptionsToAdd) {
			// Get key.
			const Key & keyRef = std::get<0>(_keyedSubscriptionToAdd);
			// Get a reference to the vector we're adding the subscription to.
			auto & subscriptionVectorToAddTo = keyedSubscriptionsMap[keyRef];

			// Create a dummy element.
			subscriptionVectorToAddTo.push_back(std::unique_ptr<Subscription<T>>());
			// Switch the dummy element with the real thing.
			std::swap(std::get<1>(_keyedSubscriptionToAdd), subscriptionVectorToAddTo.back());
		}
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

	static void _removeInvalidKeyedSubscriptions() {
		std::vector<Key> keysToBeRemoved;

		for (auto & [key, subscriptionsForKey] : keyedSubscriptionsMap) {
			subscriptionsForKey.erase(
				std::remove_if(
					subscriptionsForKey.begin(),
					subscriptionsForKey.end(),
					[](std::unique_ptr<Subscription<T>> & subscription) {
						return !subscription->isValid();
					}
				),
				subscriptionsForKey.end()
			);

			if (subscriptionsForKey.empty()) {
				keysToBeRemoved.push_back(key);
			}
		}

		for (const Key & keyToBeRemoved : keysToBeRemoved) {
			keyedSubscriptionsMap.erase(keyToBeRemoved);
		}
	}
};

template<typename T>
template<typename Func, typename... Args>
SubscriptionHandle<T>::SubscriptionHandle(Func func, Args... args) :
		subscription(EventManager<T>::subscribeRaw(func, args...))
{
	subscription.incrementSubscriptionHandles();
}
