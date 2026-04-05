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
		for (const auto dependencyId : machine->getDependsOn())
		{
			const auto dependency = this->_machines.at(dependencyId);

			this->_dependencyGraph[dependency].insert(machine);
		}
	}
}

bool Scheduler::hasCycle() const
{
	// Algorithm from: https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm
	std::unordered_map<Machine*, std::size_t> machinesInDegree;

	for (auto machine : this->_machines | std::views::values)
		machinesInDegree[machine] = machine->getDependsOn().size();

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

	while (!machineToExplore.empty())
	{
		auto currentMachine = machineToExplore.front();
		machineToExplore.pop();

		auto dependenciesIt = this->_dependencyGraph.find(currentMachine);

		if (dependenciesIt == this->_dependencyGraph.end())
			continue;

		for (auto dependency : dependenciesIt->second)
		{
			const auto inDegreeIt = machinesInDegree.find(dependency);

			if (inDegreeIt == machinesInDegree.cend())
				continue;

			inDegreeIt->second--;

			if (inDegreeIt->second == 0)
			{
				machinesInDegree.erase(inDegreeIt);
				machineToExplore.push(dependency);
			}
		}
	}

	return !machinesInDegree.empty();
}

std::vector<int> Scheduler::topologicalSort(const std::function<bool(const Machine*, const Machine*)>& tieBreaker)
{
	std::unordered_map<Machine*, std::size_t> machinesInDegree;

	for (auto machine : this->_machines | std::views::values)
		machinesInDegree[machine] = machine->getDependsOn().size();

	std::priority_queue<
		Machine*,
		std::vector<Machine*>,
		decltype(tieBreaker)
	> machinesToProcess(tieBreaker);

	for (auto it = machinesInDegree.cbegin(); it != machinesInDegree.cend();)
	{
		if (it->second > 0)
		{
			++it;
			continue;
		}

		machinesToProcess.push(it->first);
		it = machinesInDegree.erase(it);
	}

	std::vector<Machine*> machineOrder;

	while (!machinesToProcess.empty())
	{
		auto current = machinesToProcess.top();
		machinesToProcess.pop();

		machineOrder.push_back(current);

		auto it = this->_dependencyGraph.find(current);

		if (it == this->_dependencyGraph.cend())
			continue;

		for (Machine* dependent : it->second)
		{
			auto& deg = machinesInDegree[dependent];
			deg--;

			if (deg > 0)
				continue;

			machinesToProcess.push(dependent);
		}
	}

	std::vector<int> result(machineOrder.size());

	std::transform(
		machineOrder.begin(),
		machineOrder.end(),
		result.begin(),
		[](const Machine* machine)
		{
			return machine->getId();
		}
	);

	// std::priority_queue<Machine*, std::vector<Machine*>, decltype(tieBreaker)>
	return result;
}

void Scheduler::schedule()
{
	// std::unordered_set<Device*>
}
