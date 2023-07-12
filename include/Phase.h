#pragma once

#include <queue>
#include <functional>
#include <mutex>

typedef unsigned int PhaseID;

#define NOW (0)
#define NEXT (1)

class Phase {
private:
	std::queue<std::function<void(void)>> eventCalls;
	std::recursive_mutex eventCallsMutex;

	std::queue<std::function<void(void)>> endEventCalls;
	std::recursive_mutex endEventCallsMutex;

	bool endPhase = false;

public:
	void addToQueue(std::function<void(void)> eventCall) {
		eventCallsMutex.lock();
		eventCalls.push(eventCall);
		eventCallsMutex.unlock();
	}

	void addToEndQueue(std::function<void(void)> endEventCall) {
		endEventCallsMutex.lock();
		endEventCalls.push(endEventCall);
		endEventCallsMutex.unlock();
	}

	void run() {
		eventCallsMutex.lock();
		while(!eventCalls.empty()) {
			// Copy the function to call.
			std::function<void(void)> functionToCall = eventCalls.front();
			// Remove the function from the queue
			eventCalls.pop();
			eventCallsMutex.unlock();

			// Call the function
			if (functionToCall) {
				functionToCall();
			}
			eventCallsMutex.lock();
		}
		eventCallsMutex.unlock();

		endPhase = true;

		endEventCallsMutex.lock();
		while(!endEventCalls.empty()) {
			// Copy the function to call.
			std::function<void(void)> functionToCall = endEventCalls.front();
			// Remove the function from the queue
			endEventCalls.pop();
			endEventCallsMutex.unlock();

			// Call the function
			if (functionToCall) {
				functionToCall();
			}
			endEventCallsMutex.lock();
		}
		endEventCallsMutex.unlock();

		//NOTE: all non-executed events should be rescheduled in the next phase.
	}

	bool isInEndPhase() {
		return true;
	}
};
