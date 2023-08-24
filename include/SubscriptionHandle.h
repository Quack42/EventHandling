#pragma once

#include "Subscription.h"

#include <memory>

template <typename T>
class SubscriptionHandle {
private:
	// Subscription<T> & subscription;
	std::weak_ptr<Subscription<T>> subscription_wp;
public:
	SubscriptionHandle() :
			subscription_wp()
	{
		//This creates a handle for an invalid subscription.
	}

	virtual ~SubscriptionHandle() {
		std::shared_ptr<Subscription<T>> subscription_sp = subscription_wp.lock();
		if (subscription_sp) {
			subscription_sp->decrementSubscriptionHandles();
		}
	}

	template<typename Func, typename... Args>
	SubscriptionHandle(Func func, Args... args); 	//defined in EventManager.h

	SubscriptionHandle(std::weak_ptr<Subscription<T>> & subscription_wp) :
			subscription_wp(subscription_wp)
	{
		std::shared_ptr<Subscription<T>> subscription_sp = subscription_wp.lock();
		if (subscription_sp) {
			subscription_sp->incrementSubscriptionHandles();
		}
	}

	SubscriptionHandle(const SubscriptionHandle<T> & other) :
			subscription_wp(other.subscription_wp)
	{
		std::shared_ptr<Subscription<T>> subscription_sp = subscription_wp.lock();
		if (subscription_sp) {
			subscription_sp->incrementSubscriptionHandles();
		}
	}

	SubscriptionHandle<T> & operator=(const SubscriptionHandle<T> & rhs) {
		this->~SubscriptionHandle<T>();
		new (this) SubscriptionHandle<T>(rhs);
		return *this;
	}

	void unsubscribe() {
		std::shared_ptr<Subscription<T>> subscription_sp = subscription_wp.lock();
		if (subscription_sp) {
			subscription_sp->unsubscribe();
		}
	}

	void resubscribe() {
		std::shared_ptr<Subscription<T>> subscription_sp = subscription_wp.lock();
		if (subscription_sp) {
			subscription_sp->resubscribe();
		}
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

	KeyedSubscriptionHandle(const KeyedSubscriptionHandle<T> & other) :
			subscriptionHandle(other)
	{
	}

	KeyedSubscriptionHandle<T> & operator=(const KeyedSubscriptionHandle<T> & rhs) {
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
