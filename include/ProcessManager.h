#pragma once

#include <vector>
#include <mutex>
#include <functional>


class ProcessManager {
private:
	static inline std::vector<std::function<void(void)>> processRequests;
	static inline std::mutex processRequestsMutex;

	static inline std::function<void(void)> idleFunction;
	static inline std::mutex idleFunctionMutex;


public:
	template<typename Func, typename... Bindables>
	static void requestProcess(Func func, Bindables... bindables) {
		// Bind the arguments to make a simple void(void) function call; doing this here because most uses of this function will force the use of bind anyway.
		auto callbackFunction = std::bind(func, bindables...);
		
		// Store the request process.
		processRequestsMutex.lock(); 	//Anything dealing with 'processRequests' is protected by a mutex.
		processRequests.push_back(callbackFunction);
		processRequestsMutex.unlock();
	}

	static void handleProcessRequests() {
		// Handle all process requests.
		processRequestsMutex.lock(); 	//anything dealing with 'processRequests' is protected by a mutex.
		while (!processRequests.empty()) {
			// Make a copy so we don't have to deal with the "what if a called process calls requestProcess"-case (cause it most likely will occur a lot).
			std::vector<std::function<void(void)>> _processRequests;
			std::swap(processRequests, _processRequests);
			processRequestsMutex.unlock();

			// Call the process requests.
			for (auto & _processRequest : _processRequests) {
				_processRequest();
			}
			processRequestsMutex.lock(); 	//anything dealing with 'processRequests' is protected by a mutex.
		}
		processRequestsMutex.unlock();
	}

	static void callIdleFunction() {
		//nothing to do; run the idle function
		idleFunctionMutex.lock(); 	//I doubt the assignment operator of a function is atomic, so mutex it.
		std::function<void(void)> _idleFunction = idleFunction; 	//make a copy so 'idleFunction' can't change in between the check whether it is good and running it.
		idleFunctionMutex.unlock();
		if (_idleFunction) {
			_idleFunction();
		}
	}

	static void run() {
		handleProcessRequests();
		callIdleFunction();
	}

	static void setIdleFunction(std::function<void(void)> idleFunction) {
		idleFunctionMutex.lock(); 	//I doubt the assignment operator of a function is atomic, so mutex it.
		ProcessManager::idleFunction = idleFunction;
		idleFunctionMutex.unlock();
	}

};
