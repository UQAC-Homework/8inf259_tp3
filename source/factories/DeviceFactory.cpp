#include "../../include/factories/DeviceFactory.h"

Device DeviceFactory::loadDevice(const nlohmann::basic_json<>& obj)
{
	constexpr auto ID_KEY = "id";
	constexpr auto TYPE_KEY = "type";
	constexpr auto NAME_KEY = "name";
	constexpr auto LOCK_TIME_KEY = "lock_time";

	auto id = obj.at(ID_KEY).get<int>();
	auto type = obj.at(TYPE_KEY).get<std::string>();
	auto name = obj.at(NAME_KEY).get<std::string>();
	int lockTime;

	if (obj.contains(LOCK_TIME_KEY))
		lockTime = obj.at(LOCK_TIME_KEY).get<int>();
	else
		lockTime = 1;

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

		auto devicePtr = new Device(device);
		devices.insert({id, devicePtr});
	}

	return devices;
}
