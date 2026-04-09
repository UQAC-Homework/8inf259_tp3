#include "../../include/factories/TeamFactory.h"

Team TeamFactory::loadTeam(const nlohmann::basic_json<>& obj)
{
	constexpr auto CAPABILITIES_KEY = "capabilities";

	auto name = obj.at("name").get<std::string>();
	std::vector<std::string> capabilities = {};
	auto maxCapacity = obj.value<int>("max_capacity", NO_LIMIT_CAPACITY);

	const auto& capabilitiesProp = obj.at(CAPABILITIES_KEY);

	if (!capabilitiesProp.is_array())
		throw std::invalid_argument("The property '" + std::string(CAPABILITIES_KEY) + "' must be an array.");

	for (const auto& capabilityItem : capabilitiesProp.items())
	{
		auto capability = capabilityItem.value().get<std::string>();
		capabilities.emplace_back(capability);
	}

	return {name, capabilities, maxCapacity};
}

std::vector<Team*> TeamFactory::loadFromJson(const nlohmann::basic_json<>& obj)
{
	constexpr auto TEAMS_KEY = "teams";

	const auto& teamsProp = obj.at(TEAMS_KEY);

	if (!teamsProp.is_array())
		throw std::invalid_argument("The property '" + std::string(TEAMS_KEY) + "' must be an array.");

	std::vector<Team*> teams;

	for (const auto& teamItem : teamsProp.items())
	{
		const auto team = loadTeam(teamItem.value());

		auto teamPtr = new Team(team);
		teams.emplace_back(teamPtr);
	}

	return teams;
}
