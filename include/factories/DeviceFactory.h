#ifndef INC_8INF259_TP3_DEVICEFACTORY_H
#define INC_8INF259_TP3_DEVICEFACTORY_H
#include <unordered_map>

#include "../../Device.h"
#include "../library/json.hpp"

/// Class that handles the creation of devices
class DeviceFactory
{
	/// Loads a single device from the given JSON object
	static Device loadDevice(const nlohmann::basic_json<>& obj);
public:
	/// Loads every device in the given JSON object
	static std::unordered_map<int, Device*> loadFromJson(const nlohmann::basic_json<>& obj);
};

#endif //INC_8INF259_TP3_DEVICEFACTORY_H
