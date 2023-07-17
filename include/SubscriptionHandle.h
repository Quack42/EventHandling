#pragma once

#include "Subscription.h"

template <typename T>
class SubscriptionHandle {
private:
	Subscription<T> & subscription;
public:
	SubscriptionHandle() :
			subscription(Subscription<T>::DummySubscription)
	{
		//This creates a handle for an invalid subscription.
	}

	virtual ~SubscriptionHandle() {
		subscription.decrementSubscriptionHandles();
	}

	template<typename Func, typename... Args>
	SubscriptionHandle(Func func, Args... args); 	//defined in EventManager.h

	SubscriptionHandle(Subscription<T> & subscription) :
			subscription(subscription)
	{
		subscription.incrementSubscriptionHandles();
	}

	SubscriptionHandle(SubscriptionHandle<T> & other) :
			subscription(other.subscription)
	{
		subscription.incrementSubscriptionHandles();
	}

	SubscriptionHandle<T> & operator=(SubscriptionHandle<T> & rhs) {
		this->~SubscriptionHandle<T>();
		new (this) SubscriptionHandle<T>(rhs);
		return *this;
	}

	void unsubscribe() {
		subscription.unsubscribe();
	}

	void resubscribe() {
		subscription.resubscribe();
	}
};

template <typename T>
class KeyedSubscriptionHandle { 	//Literally just a wrapper for SubscriptionHandle with one slightly different constructor ()
private:
	SubscriptionHandle<T> subscriptionHandle;
public:
	KeyedSubscriptionHandle(){} 	// An invalid subscription.

	template<typename Func, typename KeyInputType, typename... Args>
	KeyedSubscriptionHandle(Func func, KeyInputType keyInput, Args... args); 	//defined in EventManager.h

	KeyedSubscriptionHandle(Subscription<T> & subscription) :
			subscriptionHandle(subscription)
	{
	}

	KeyedSubscriptionHandle(KeyedSubscriptionHandle<T> & other) :
			subscriptionHandle(other)
	{
	}

	KeyedSubscriptionHandle<T> & operator=(KeyedSubscriptionHandle<T> & rhs) {
		this->subscriptionHandle = rhs.subscriptionHandle;
		return *this;
	}

	void unsubscribe() {
		subscriptionHandle.unsubscribe();
	}

	void resubscribe() {
		subscriptionHandle.resubscribe();
	}
};
