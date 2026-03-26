#include "../Device.h"

#include <fstream>
#include <iostream>

#include "../include/factories/DeviceFactory.h"
#include "../include/library/json.hpp"

Device::Device(int id, const std::string& type, const std::string& name, int lockTime)
{
	this->id = id;
	this->type = type;
	this->name = name;
	this->lockTime = lockTime;
}

std::unordered_map<int, Device*> Device::BuildDevicesFromJsonFile(const std::string& path)
{
	if (!std::filesystem::exists(path))
		throw std::runtime_error("File at '" + path + "' does not exist.");

	std::ifstream file(path);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	auto json = nlohmann::json::parse(content);

	return DeviceFactory::loadFromJson(json);
}
