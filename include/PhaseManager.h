#pragma once

#include "Phase.h"
#include "ProcessManager.h"

#include <unordered_map>
#include <queue>
#include <functional>


class DelayedPhaseEvent {
private:
	PhaseID phaseID;
	unsigned int offset; 	//TODO: refactor this.
	std::function<void(void)> eventManagementFunction;
public:
	DelayedPhaseEvent(PhaseID phaseID, unsigned int offset, std::function<void(void)> eventManagementFunction) :
			phaseID(phaseID),
			offset(offset),
			eventManagementFunction(eventManagementFunction)
	{
	}

	PhaseID getPhaseID() const {
		return phaseID;
	}

	std::function<void(void)> getEventManagementFunction() const {
		return eventManagementFunction;
	}

	unsigned int getOffset() const {
		return offset;
	}

	void decrementOffset() {
		offset--;
	}
};


class PhaseManager {
private:
	static inline std::unordered_map<PhaseID, Phase> phaseMap;
	static inline std::queue<PhaseID> phaseQueue;

	static inline std::unordered_map<PhaseID, std::vector<DelayedPhaseEvent>> delayedPhaseEventMap;

	static inline std::function<void(void)> phaseQueueEmptyCallback;

public:
	static void managePhases() {
		// If nothing to do; then don't do it.
		if (phaseQueue.empty()) {
			return;
		}

		// Take the phaseID to execute from the queue.
		PhaseID phaseID = phaseQueue.front();
		phaseQueue.pop();

		// Execute the corresponding phase.
		phaseMap[phaseID].run();

		// Update the delayed phase events; and schedule them if necessary.
		if(delayedPhaseEventMap.contains(phaseID)) {
			// Get the appropriate delayed phase events.
			auto & delayedPhaseEvents = delayedPhaseEventMap[phaseID];

			// Decrement the countdown/offset of the delayed phase events; and if that is 0, schedule them.
			for (DelayedPhaseEvent & delayedPhaseEvent : delayedPhaseEvents) {
				// Decrement the countdown/offset.
				delayedPhaseEvent.decrementOffset();

				// Check if they can be scheduled.
				if (delayedPhaseEvent.getOffset() == 0) {
					// Schedule.
					phaseMap[phaseID].addToQueue(delayedPhaseEvent.getEventManagementFunction());
				}
			}

			// Clear the scheduled delayed phase events.
			delayedPhaseEvents.erase(
				std::remove_if(
					delayedPhaseEvents.begin(),
					delayedPhaseEvents.end(),
					[](DelayedPhaseEvent & delayedPhaseEvent) {
						return delayedPhaseEvent.getOffset() == 0; 	// If it is 0, then it was scheduled above.
					}
				),
				delayedPhaseEvents.end()
			);
			
			// If there are no more delayed phase events, remove the empty vector from the 'delayedPhaseEventMap'.
			if (delayedPhaseEvents.empty()) {
				delayedPhaseEventMap.erase(phaseID);
			}
		}

		// If there's more phases to execute, schedule their execution.
		// NOTE: Scheduling the next phase execution here gives non-phased events priority over phased events.
		if (!phaseQueue.empty()) {
			requestManagingProcessPhases();
		} else {
			if (phaseQueueEmptyCallback) {
				phaseQueueEmptyCallback();
			}
		}
	}

	static void queuePhase(PhaseID phaseID) {
		phaseQueue.push(phaseID);
		if (phaseQueue.size() == 1) {
			requestManagingProcessPhases();
		}
	}

	static void runUntilEmpty() {
		requestManagingProcessPhases();
	}

	static void setPhaseQueueEmptyCallback(std::function<void(void)> phaseQueueEmptyCallback) {
		PhaseManager::phaseQueueEmptyCallback = phaseQueueEmptyCallback;
		// if (phaseQueue.empty()) {
		// 	if (phaseQueueEmptyCallback) {
		// 		phaseQueueEmptyCallback();
		// 	}
		// }
	}

	static void setPhaseStartCallback(PhaseID phaseID, std::function<void(void)> phaseStartCallback) {
		phaseMap[phaseID].setPhaseStartCallback(phaseStartCallback);
	}

	static void setPhaseEndCallback(PhaseID phaseID, std::function<void(void)> phaseEndCallback) {
		phaseMap[phaseID].setPhaseEndCallback(phaseEndCallback);
	}

	static void registerEventCallback(PhaseID phaseID, unsigned int offset, std::function<void(void)> eventManagementFunction) {
		if (offset > 0) {
			delayedPhaseEventMap[phaseID].emplace_back(phaseID, offset, eventManagementFunction);
		} else {
			phaseMap[phaseID].addToQueue(eventManagementFunction);
		}
	}

private:
	static void requestManagingProcessPhases() {
		ProcessManager::requestProcess(&PhaseManager::managePhases);
	}
};
