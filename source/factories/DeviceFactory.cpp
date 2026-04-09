#include "../../include/factories/DeviceFactory.h"

Device DeviceFactory::loadDevice(const nlohmann::basic_json<>& obj)
{
	auto id = obj.at("id").get<int>();
	auto type = obj.at("type").get<std::string>();
	auto name = obj.at("name").get<std::string>();
	auto lockTime = obj.value<int>("lock_time", 1);

	return {id, type, name, lockTime};
}

std::unordered_map<int, Device*> DeviceFactory::loadFromJson(const nlohmann::basic_json<>& obj)
{
	constexpr auto DEVICES_KEY = "devices";

	const auto& devicesProp = obj.at(DEVICES_KEY);

	if (!devicesProp.is_array())
		throw std::invalid_argument("The property '" + std::string(DEVICES_KEY) + "' must be an array.");

	std::unordered_map<int, Device*> devices;

	for (const auto& deviceItem : devicesProp.items())
	{
		auto device = loadDevice(deviceItem.value());
		const auto id = device.getId();

		if (devices.contains(id))
			throw std::runtime_error("A device with ID '" + std::to_string(id) + "' is already loaded.");

		auto devicePtr = new Device(device);
		devices.insert({id, devicePtr});
	}

	return devices;
}
