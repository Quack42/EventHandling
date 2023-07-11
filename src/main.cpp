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
		std::cout << event.getInputString() << std::endl;
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

		EventManager<InputEvent>::addEvent("Hello World!");

		ProcessManager::run();
		EventManager<InputEvent>::addEvent("Hello World2!");
		EventManager<InputEvent>::addEvent("Hello World3!");
		ProcessManager::run();
		ier.unsubscribe();
		EventManager<InputEvent>::addEvent("Hello World4!");
		ProcessManager::run();
		ier.resubscribe();
		EventManager<InputEvent>::addEvent("Hello World5!");
		ProcessManager::run();
	}

	EventManager<InputEvent>::addEvent("Hello WorldX!");
	ProcessManager::run();

	return 0;
}