#include "../../include/factories/DeviceFactory.h"

Device DeviceFactory::loadDevice(const nlohmann::basic_json<>& obj)
{
	return {0, "", "", 0};
}

std::unordered_map<int, Device*> DeviceFactory::loadFromJson(const nlohmann::basic_json<>& obj)
{
	return {};
}
