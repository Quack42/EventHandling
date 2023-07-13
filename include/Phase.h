#pragma once

#include <queue>
#include <functional>
#include <mutex>

typedef unsigned int PhaseID;

#define NOW (0)
#define NEXT (1)

class Phase {
private:
	std::queue<std::function<void(void)>> eventManagementFunctionCalls;
	std::recursive_mutex eventManagementFunctionCallsMutex;

	std::function<void(void)> phaseStartCallback;
	std::recursive_mutex phaseStartCallbackMutex;
	std::function<void(void)> phaseEndCallback;
	std::recursive_mutex phaseEndCallbackMutex;

public:
	void addToQueue(std::function<void(void)> eventManagementFunctionCall) {
		eventManagementFunctionCallsMutex.lock();
		eventManagementFunctionCalls.push(eventManagementFunctionCall);
		eventManagementFunctionCallsMutex.unlock();
	}

	bool hasEventsInQueue() const {
		return !eventManagementFunctionCalls.empty();
	}

	void run() {
		// Call phase start callback.
		phaseStartCallbackMutex.lock();
		if (phaseStartCallback) {
			phaseStartCallback();
		}
		phaseStartCallbackMutex.unlock();

		// Call events until there are no more events.
		eventManagementFunctionCallsMutex.lock();
		while(!eventManagementFunctionCalls.empty()) {
			// Copy the function to call.
			std::function<void(void)> eventManagementFunctionCall = eventManagementFunctionCalls.front();
			// Remove the function from the queue
			eventManagementFunctionCalls.pop();
			eventManagementFunctionCallsMutex.unlock();

			// Call the function
			if (eventManagementFunctionCall) {
				eventManagementFunctionCall();
			}
			eventManagementFunctionCallsMutex.lock();
		}
		//Any events registered at this point and after will be executed next cycle. "Too late!"
		eventManagementFunctionCallsMutex.unlock();

		// Call phase end callback.
		phaseEndCallbackMutex.lock();
		if (phaseEndCallback) {
			phaseEndCallback();
		}
		phaseEndCallbackMutex.unlock();
	}

	void setPhaseStartCallback(std::function<void(void)> phaseStartCallback) {
		phaseStartCallbackMutex.lock();
		this->phaseStartCallback = phaseStartCallback;
		phaseStartCallbackMutex.unlock();
	}

	void setPhaseEndCallback(std::function<void(void)> phaseEndCallback) {
		phaseEndCallbackMutex.lock();
		this->phaseEndCallback = phaseEndCallback;
		phaseEndCallbackMutex.unlock();
	}
};
