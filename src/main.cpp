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

class ComputeEvent {
private:
	int x;

public:
	ComputeEvent(const int & x) :
			x(x)
	{
		// do nothing
	}

	const int & getX() const {
		return x;
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

enum Phases_e {
	e_Tick,
	e_Graphics,
	e_UI
};



class PhasedEventReceiver {
private:
	SubscriptionHandle<InputEvent> subscriberHandle_inputEvent;
	SubscriptionHandle<ComputeEvent> subscriberHandle_computeEvent;
public:
	PhasedEventReceiver() :
			subscriberHandle_inputEvent(EventManager<InputEvent>::subscribe(&PhasedEventReceiver::receiveInputEvent, this)),
			subscriberHandle_computeEvent(EventManager<ComputeEvent>::subscribe(&PhasedEventReceiver::receiveComputeEvent, this))
	{

	}

	void receiveInputEvent(InputEvent event) {
		std::cout << "PIER-ie:" << event.getInputString() << std::endl;
		EventManager<ComputeEvent>::addPhasedEvent(e_Tick, NOW, 41);
		EventManager<ComputeEvent>::addPhasedEvent(e_Tick, NEXT, 40);
	}
	void receiveComputeEvent(ComputeEvent event) {
		std::cout << "PIER-ce:" << event.getX() << std::endl;
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
		KeyedInputEventReceiver kier1(1);
		KeyedInputEventReceiver kier2(2);

		EventManager<InputEvent>::addKeyedEvent(1, "Hello World1!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World1!");

		ProcessManager::run();
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World2!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World2!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World3!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World3!");
		ProcessManager::run();
		kier1.unsubscribe();
		kier2.unsubscribe();
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World - shouldn't print 1!");
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World - shouldn't print 1!");
		ProcessManager::run();
		kier1.resubscribe();
		kier2.resubscribe();
		EventManager<InputEvent>::addKeyedEvent(2, "~Hello World4!");
		EventManager<InputEvent>::addKeyedEvent(1, "Hello World4!");
		ProcessManager::run();
	}

	EventManager<InputEvent>::addKeyedEvent(1, "Hello World - shouldn't print 2!");
	EventManager<InputEvent>::addKeyedEvent(2, "~Hello World - shouldn't print 2!");
	ProcessManager::run();

	std::cout << "------------" << std::endl;

	{
		PhasedEventReceiver pier;

		PhaseManager::queuePhase(e_Tick);
		PhaseManager::queuePhase(e_Graphics);
		PhaseManager::queuePhase(e_UI);
		PhaseManager::queuePhase(e_Tick);
		PhaseManager::queuePhase(e_Graphics);
		PhaseManager::queuePhase(e_UI);

		{
			//Phase: tick
			EventManager<InputEvent>::addPhasedEvent(e_Tick, NOW, "First cycle tick");
			EventManager<InputEvent>::addPhasedEvent(e_Tick, NEXT, "Second cycle tick");
			EventManager<ComputeEvent>::addPhasedEvent(e_Tick, NOW, 42);
		}
		{
			//Phase: graphics
			EventManager<InputEvent>::addPhasedEvent(e_Graphics, NOW, "First cycle graphics");
			EventManager<InputEvent>::addPhasedEvent(e_Graphics, NEXT, "Second cycle graphics");
			EventManager<ComputeEvent>::addPhasedEvent(e_Graphics, NOW, 43);
		}
		{
			//Phase: UI
			EventManager<InputEvent>::addPhasedEvent(e_UI, NOW, "First cycle UI");
			EventManager<InputEvent>::addPhasedEvent(e_UI, NEXT, "Second cycle UI");
			EventManager<ComputeEvent>::addPhasedEvent(e_UI, NOW, 44);
		}

		ProcessManager::run();
	}

	return 0;
}
