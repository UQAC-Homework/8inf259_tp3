#include "../../include/factories/MachineFactory.h"

Machine MachineFactory::loadMachine(
	const nlohmann::basic_json<>& obj,
	const std::unordered_map<int, Device*>& devices
)
{
	constexpr auto ID_KEY = "id";
	constexpr auto NAME_KEY = "name";
	constexpr auto RELATIVE_DEVICES_KEY = "related_devices";
	constexpr auto MACHINE_DEPENDENCIES_KEY = "depends_on";

	if (!obj.contains(ID_KEY))
		throw std::invalid_argument("The property '" + std::string(ID_KEY) + "' does not exist.");

	if (!obj.contains(NAME_KEY))
		throw std::invalid_argument("The property '" + std::string(NAME_KEY) + "' does not exist.");

	auto id = obj.at(ID_KEY).get<int>();
	auto name = obj.at(NAME_KEY).get<std::string>();
	std::vector<Device*> relativeDevices = {};
	std::vector<int> machineDependencies = {};

	if (obj.contains(RELATIVE_DEVICES_KEY))
	{
		const auto& relativeDevicesProp = obj.at(RELATIVE_DEVICES_KEY);

		if (!relativeDevicesProp.is_array())
			throw std::invalid_argument("The property '" + std::string(RELATIVE_DEVICES_KEY) + "' must be an array.");

		for (const auto& deviceItem : relativeDevicesProp.items())
		{
			auto deviceId = deviceItem.value().get<int>();

			if (!devices.contains(deviceId))
				throw std::out_of_range("Failed to find the device with the id '" + std::to_string(deviceId) + "'.");

			auto device = devices.at(deviceId);
			relativeDevices.emplace_back(device);
		}
	}

	if (obj.contains(MACHINE_DEPENDENCIES_KEY))
	{
		const auto& machineDependenciesProp = obj.at(MACHINE_DEPENDENCIES_KEY);

		if (!machineDependenciesProp.is_array())
			throw std::invalid_argument(
				"The property '" + std::string(MACHINE_DEPENDENCIES_KEY) + "' must be an array.");

		for (const auto& machineItem : machineDependenciesProp.items())
		{
			auto machineId = machineItem.value().get<int>();

			machineDependencies.emplace_back(machineId);
		}
	}

	return {id, name, relativeDevices, machineDependencies};
}

std::unordered_map<int, Machine*> MachineFactory::loadFromJson(
	const nlohmann::basic_json<>& obj,
	const std::unordered_map<int, Device*>& devices
)
{
	constexpr auto MACHINES_KEY = "machines";

	if (!obj.contains(MACHINES_KEY))
		throw std::invalid_argument("The property '" + std::string(MACHINES_KEY) + "' does not exist.");

	const auto& machinesProps = obj.at(MACHINES_KEY);

	if (!machinesProps.is_array())
		throw std::invalid_argument("The property '" + std::string(MACHINES_KEY) + "' must be an array.");

	std::unordered_map<int, Machine*> machines;

	for (const auto& machineItem : machinesProps.items())
	{
		auto machine = loadMachine(machineItem.value(), devices);
		const auto id = machine.getId();

		auto machinePtr = new Machine(machine);
		machines.insert({id, machinePtr});
	}

	for (const auto machine : machines | std::views::values)
	{
		auto dependencies = machine->getDependsOn();

		for (auto dependency : dependencies)
		{
			if (machines.contains(dependency))
				continue;

			throw std::invalid_argument(
				"A machine depends on '" + std::to_string(dependency) + "', but it does not exist.");
		}
	}

	return machines;
}
