#include "../Machine.h"

#include <filesystem>
#include <fstream>

#include "../include/factories/MachineFactory.h"
#include "../include/library/json.hpp"

Machine::Machine(int id, const std::string& name) : Machine(id, name, {}, {})
{
}

Machine::Machine(
	int id,
	const std::string& name,
	const std::vector<Device*>& relatedDevices,
	const std::vector<int>& dependsOn
)
{
	this->id = id;
	this->name = name;
	this->dependsOn = dependsOn;
	this->relatedDevices = relatedDevices;
}

std::unordered_map<int, Machine*> Machine::BuildMachinesFromJsonFile(
	const std::string& path,
	const std::unordered_map<int, Device*>& devices
)
{
	if (!std::filesystem::exists(path))
		throw std::runtime_error("File at '" + path + "' does not exist.");

	std::ifstream file(path);
	std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

	auto json = nlohmann::json::parse(content);

	return MachineFactory::loadFromJson(json, devices);
}
