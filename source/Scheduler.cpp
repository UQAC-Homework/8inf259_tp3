#include "../Scheduler.h"

#include <format>
#include <iostream>
#include <queue>
#include <ranges>
#include <unordered_set>

#include "../include/library/string.h"

Scheduler::Scheduler(
	const std::unordered_map<int, Device*>& devices,
	const std::unordered_map<int, Machine*>& machines,
	const std::vector<Team*>& teams
)
{
	this->_devices = devices;
	this->_machines = machines;
	this->_teams = teams;
	this->_dependencyGraph = {};
}

void Scheduler::displayNotConnectedDevices()
{
	std::unordered_set<Device*> connectedDevices;

	for (const auto machine : this->_machines | std::views::values)
	{
		for (const auto device : machine->getRelatedDevices())
			connectedDevices.insert(device);
	}

	std::cout << "=== DEVICES NOT CONNECTED TO ANY MACHINE ===" << std::endl;

	for (const auto device : this->_devices | std::views::values)
	{
		if (connectedDevices.contains(device))
			continue;

		auto message = std::format(
			"- Device {} ( {} ) is not connected",
			device->getId(),
			device->getName()
		);

		std::cout << message << std::endl;
	}

	std::cout << std::endl;
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

	std::cout << std::endl;
}

void Scheduler::buildDependencyGraph()
{
	this->_dependencyGraph.clear();

	for (const auto machine : this->_machines | std::views::values)
	{
		const auto dependenciesId = machine->getDependsOn();

		if (dependenciesId.empty())
			continue;

		std::unordered_set<Machine*> dependencies;

		for (const auto dependencyId : dependenciesId)
		{
			const auto dependency = this->_machines.at(dependencyId);

			dependencies.insert(dependency);
		}

		this->_dependencyGraph.insert({machine, dependencies});
	}
}

bool Scheduler::hasCycle() const
{
	// TODO: Check if DFS is faster than Kahn's Algorithm 
	// Algorithm from: https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm
	std::unordered_map<Machine*, int> machinesInDegree;

	for (const auto& [machine, dependencies] : this->_dependencyGraph)
	{
		if (!machinesInDegree.contains(machine))
			machinesInDegree.insert({machine, 0});

		for (const auto dependency : dependencies)
		{
			if (!machinesInDegree.contains(dependency))
				machinesInDegree.insert({dependency, 0});

			machinesInDegree[dependency]++;
		}
	}

	std::queue<Machine*> machineToExplore;

	for (auto it = machinesInDegree.cbegin(); it != machinesInDegree.cend();)
	{
		if (it->second > 0)
		{
			++it;
			continue;
		}

		machineToExplore.push(it->first);
		it = machinesInDegree.erase(it);
	}

	if (machineToExplore.empty())
		return true;

	while (!machineToExplore.empty())
	{
		auto currentMachine = machineToExplore.front();
		machineToExplore.pop();

		if (!this->_dependencyGraph.contains(currentMachine))
			continue;

		for (auto dependency : this->_dependencyGraph.at(currentMachine))
		{
			const auto inDegreeIt = machinesInDegree.find(dependency);

			if (inDegreeIt == machinesInDegree.cend())
				return true;

			if (inDegreeIt->second == 1)
			{
				machinesInDegree.erase(inDegreeIt);
				machineToExplore.push(dependency);
				continue;
			}

			inDegreeIt->second--;
		}
	}

	return !machinesInDegree.empty();
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
