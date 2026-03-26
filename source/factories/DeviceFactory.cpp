#include "../../include/factories/DeviceFactory.h"

Device DeviceFactory::loadDevice(const nlohmann::basic_json<>& obj)
{
	return {0, "", "", 0};
}

std::unordered_map<int, Device*> DeviceFactory::loadFromJson(const nlohmann::basic_json<>& obj)
{
	constexpr auto DEVICES_KEY = "devices";

	if (!obj.contains(DEVICES_KEY))
		throw std::invalid_argument("The property '" + std::string(DEVICES_KEY) + "' does not exist.");

	const auto& devicesProp = obj.at(DEVICES_KEY);

	if (!devicesProp.is_array())
		throw std::invalid_argument("The property '" + std::string(DEVICES_KEY) + "' must be an array.");

	std::unordered_map<int, Device*> devices;

	for (const auto& deviceItem : devicesProp.items())
	{
		auto device = loadDevice(deviceItem.value());
		const auto id = device.getId();

		devices.insert({id, &device});
	}

	return devices;
}
