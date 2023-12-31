#pragma once

#include <functional>
#include <mutex>


template <typename T>
class Subscription {
private:
	std::function<void(T&)> subscriberFunction;
	unsigned int subscriptionHandles;
	std::mutex subscriptionHandlesMutex;

	std::recursive_mutex deletionDelayMutex;
	bool subscribed;

	Subscription() {
		//dummy subscription.
	}

public:
	static inline Subscription<T> DummySubscription;

	Subscription(std::function<void(T&)> subscriberFunction) :
			subscriberFunction(subscriberFunction),
			subscriptionHandles(0),
			subscribed(true)
	{

	}

	void incrementSubscriptionHandles() {
		subscriptionHandlesMutex.lock();
		subscriptionHandles++;
		subscriptionHandlesMutex.unlock();
	}

	void decrementSubscriptionHandles() {
		subscriptionHandlesMutex.lock();
		subscriptionHandles--;
		auto _subscriptionHandles = subscriptionHandles;
		subscriptionHandlesMutex.unlock();

		if (_subscriptionHandles == 0) {
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
		if (isValid() && subscribed) {
			subscriberFunction(event);
		}
		deletionDelayMutex.unlock();
	}

	void unsubscribe() {
		subscribed = false;
	}

	void resubscribe() {
		subscribed = true;
	}
};
