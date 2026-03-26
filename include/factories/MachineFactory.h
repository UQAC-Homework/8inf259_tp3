#ifndef INC_8INF259_TP3_MACHINEFACTORY_H
#define INC_8INF259_TP3_MACHINEFACTORY_H
#include <unordered_map>

#include "../../Machine.h"
#include "../library/json.hpp"

/// Class that handles the creation of machines
class MachineFactory
{
	/// Loads a single machine from the given JSON object
	static Machine loadMachine(
		const nlohmann::basic_json<>& obj,
		const std::unordered_map<int, Device*>& devices
	);

public:
	/// Loads every machine in the given JSON object
	static std::unordered_map<int, Machine*> loadFromJson(
		const nlohmann::basic_json<>& obj,
		const std::unordered_map<int, Device*>& devices
	);
};


#endif //INC_8INF259_TP3_MACHINEFACTORY_H
