#pragma once

#include "Subscription.h"

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

	SubscriptionHandle(SubscriptionHandle<T> & other) :
			subscription(other.subscription)
	{
		subscription.incrementSubscriptionHandles();
	}


	SubscriptionHandle<T> & operator=(SubscriptionHandle<T> & rhs) {
		this->subscription.decrementSubscriptionHandles();
		this->subscription = rhs.subscription;
		rhs.subscription.incrementSubscriptionHandles();
		return *this;
	}

	void unsubscribe() {
		subscription.unsubscribe();
	}

	void resubscribe() {
		subscription.resubscribe();
	}
};
