#include "EventManager.h"

#include <iostream>
#include <string>

class InputEvent {
private:
	std::string inputString;

public:
	InputEvent(const std::string & inputString) :
			inputString(inputString)
	{
		// do nothing
	}

	const std::string & getInputString() const {
		return inputString;
	}
};



class InputEventReceiver {
private:
	SubscriptionHandle<InputEvent> subscriberHandle_inputEvent;
public:
	InputEventReceiver() :
			subscriberHandle_inputEvent(EventManager<InputEvent>::subscribe(&InputEventReceiver::receiveEvent, this))
	{

	}

	void receiveEvent(InputEvent event) {
		std::cout << "IER:" << event.getInputString() << std::endl;
	}

	void unsubscribe() {
		subscriberHandle_inputEvent.unsubscribe();
	}

	void resubscribe() {
		subscriberHandle_inputEvent.resubscribe();
	}
};

class KeyedInputEventReceiver {
private:
	SubscriptionHandle<InputEvent> subscriberHandle_inputEvent;
	int x;
public:
	KeyedInputEventReceiver(int x) :
			subscriberHandle_inputEvent(EventManager<InputEvent>::keyedSubscribe(&KeyedInputEventReceiver::receiveEvent, x, this)),
			x(x)
	{

	}

	void receiveEvent(InputEvent event) {
		std::cout << "KIER[" << x << "]:" << event.getInputString() << std::endl;
	}

	void unsubscribe() {
		subscriberHandle_inputEvent.unsubscribe();
	}

	void resubscribe() {
		subscriberHandle_inputEvent.resubscribe();
	}
};



int main() {
	
	{
	InputEventReceiver ier;

		EventManager<InputEvent>::addEvent("Hello World1!");

		ProcessManager::run();
		EventManager<InputEvent>::addEvent("Hello World2!");
		EventManager<InputEvent>::addEvent("Hello World3!");
		ProcessManager::run();
		ier.unsubscribe();
		EventManager<InputEvent>::addEvent("Hello World - shouldn't print 1!");
		ProcessManager::run();
		ier.resubscribe();
		EventManager<InputEvent>::addEvent("Hello World4!");
		ProcessManager::run();
	}
	EventManager<InputEvent>::addEvent("Hello World - shouldn't print 2!");
	ProcessManager::run();


	std::cout << "------------" << std::endl;

	{
		KeyedInputEventReceiver kier(1);
		KeyedInputEventReceiver kier2(2);

		EventManager<InputEvent>::addKeyedEvent(1, "Hello World1!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World1!");

		ProcessManager::run();
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World2!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World2!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World3!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World3!");
		ProcessManager::run();
		kier.unsubscribe();
		kier2.unsubscribe();
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World - shouldn't print 1!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World - shouldn't print 1!");
		ProcessManager::run();
		kier.resubscribe();
		kier2.resubscribe();
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World4!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World4!");
		ProcessManager::run();
	}

	EventManager<InputEvent>::addKeyedEvent(1, "Hello World - shouldn't print 2!");
	EventManager<InputEvent>::addKeyedEvent(2, "~Hello World - shouldn't print 2!");
	ProcessManager::run();

	return 0;
}