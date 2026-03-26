#include "../../include/factories/TeamFactory.h"

Team TeamFactory::loadTeam(const nlohmann::basic_json<>& obj)
{
	return {"", {}, 0};
}

std::vector<Team*> TeamFactory::loadFromJson(const nlohmann::basic_json<>& obj)
{
	constexpr auto TEAMS_KEY = "teams";

	if (!obj.contains(TEAMS_KEY))
		throw std::invalid_argument("The property '" + std::string(TEAMS_KEY) + "' does not exist.");

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
