#include "../../include/factories/TeamFactory.h"

Team TeamFactory::loadTeam(const nlohmann::basic_json<>& obj)
{
	constexpr auto NAME_KEY = "name";
	constexpr auto CAPABILITIES_KEY = "capabilities";
	constexpr auto MAX_CAPACITY_KEY = "max_capacity";

	const auto& capabilitiesProp = obj.at(CAPABILITIES_KEY);

	if (!capabilitiesProp.is_array())
		throw std::invalid_argument("The property '" + std::string(CAPABILITIES_KEY) + "' must be an array.");

	auto name = obj.at(NAME_KEY).get<std::string>();
	std::vector<std::string> capabilities = {};
	int maxCapacity = NO_LIMIT_CAPACITY;

	for (const auto& capabilityItem : capabilitiesProp.items())
	{
		auto capability = capabilityItem.value().get<std::string>();
		capabilities.emplace_back(capability);
	}

	if (obj.contains(MAX_CAPACITY_KEY))
		maxCapacity = obj.at(MAX_CAPACITY_KEY).get<int>();

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
