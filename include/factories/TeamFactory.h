#ifndef INC_8INF259_TP3_TEAMFACTORY_H
#define INC_8INF259_TP3_TEAMFACTORY_H
#include <vector>

#include "../../Team.h"
#include "../library/json.hpp"

/// Class that handles the creation of teams
class TeamFactory
{
private:
	/// Loads a single team from the given JSON object
	static Team loadTeam(const nlohmann::basic_json<>& obj);
public:
	/// Loads every team in the given JSON object
	static std::vector<Team*> loadFromJson(const nlohmann::basic_json<>& obj);
};

#endif //INC_8INF259_TP3_TEAMFACTORY_H
