#include "../Scheduler.h"

#include <format>
#include <iostream>

#include "../include/library/string.h"

Scheduler::Scheduler(
	const std::unordered_map<int, Device*>& devices,
	const std::unordered_map<int, Machine*>& machines,
	const std::vector<Team*>& teams
)
{
	this->_teams = teams;
}

void Scheduler::displayNotConnectedDevices()
{
	// std::unordered_map<Device*, std::vector<Machine*>>
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Scheduler::displayTeams()
{
	std::cout << "=== TEAMS ===" << std::endl;

	for (const auto& team : this->_teams)
	{
		auto message = std::format(
			"- {} (capacity={}) : {}",
			team->getName(),
			team->getMaxCapacity(),
			library::string::join(
				team->getCapabilities(),
				", "
			)
		);
		
		std::cout << message << std::endl;
	}
}

void Scheduler::buildDependencyGraph()
{
	// ???
}

bool Scheduler::hasCycle() const
{
	// std::unordered_set<Machine*>
	// DFS
}

std::vector<int> Scheduler::topologicalSort(const std::function<bool(const Machine*, const Machine*)>& tieBreaker)
{
	// std::priority_queue<Machine*, std::vector<Machine*>, decltype(tieBreaker)>
	// BFS
}

void Scheduler::schedule()
{
	// std::unordered_set<Device*>
}
