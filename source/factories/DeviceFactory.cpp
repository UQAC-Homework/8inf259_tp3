#include "../../include/factories/DeviceFactory.h"

Device DeviceFactory::loadDevice(const nlohmann::basic_json<>& obj)
{
	constexpr auto ID_KEY = "id";
	constexpr auto TYPE_KEY = "type";
	constexpr auto NAME_KEY = "name";
	constexpr auto LOCK_TIME_KEY = "lock_time";

	if (!obj.contains(ID_KEY))
		throw std::invalid_argument("The property '" + std::string(ID_KEY) + "' does not exist.");

	if (!obj.contains(TYPE_KEY))
		throw std::invalid_argument("The property '" + std::string(TYPE_KEY) + "' does not exist.");

	if (!obj.contains(NAME_KEY))
		throw std::invalid_argument("The property '" + std::string(NAME_KEY) + "' does not exist.");

	auto id = obj.at(ID_KEY).get<int>();
	auto type = obj.at(TYPE_KEY).get<std::string>();
	auto name = obj.at(NAME_KEY).get<std::string>();
	auto lockTime = 1;

	if (obj.contains(LOCK_TIME_KEY))
		lockTime = obj.at(LOCK_TIME_KEY).get<int>();

	return {id, type, name, lockTime};
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

		auto devicePtr = new Device(device);
		devices.insert({id, devicePtr});
	}

	return devices;
}
