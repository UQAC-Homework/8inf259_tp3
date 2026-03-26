#include "../Team.h"

#include <filesystem>
#include <fstream>

#include "../include/factories/TeamFactory.h"
#include "../include/library/json.hpp"

Team::Team(const std::string& name, const std::vector<std::string>& capabilities, int maxCapacity)
{
	this->name = name;
	this->capabilities = capabilities;
	this->maxCapacity = maxCapacity;
}

std::vector<Team*> Team::BuildTeamsFromJsonFile(const std::string& filename)
{
	if (!std::filesystem::exists(filename))
		throw std::runtime_error("File at '" + filename + "' does not exist.");

	std::ifstream file(filename);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	auto json = nlohmann::json::parse(content);

	return TeamFactory::loadFromJson(json);
}
