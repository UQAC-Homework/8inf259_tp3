#include "../Scheduler.h"

#include <format>
#include <iostream>
#include <queue>
#include <ranges>
#include <unordered_set>

#include "../include/library/string.h"

bool Scheduler::defaultTieBreaker(const Machine* a, const Machine* b)
{
	return a->getId() < b->getId();
}

void Scheduler::displayTopologicalOrder(std::ostream& output, const std::vector<int>& order) const
{
	output << "=== TOPOLOGICAL ORDER ===" << std::endl;
	auto orderIndex = 0;

	for (auto machineId : order)
	{
		const auto machine = this->_machines.at(machineId);

		std::string dependencyMessage;

		if (!machine->getDependsOn().empty())
		{
			dependencyMessage = std::format(
				"[after: {}]",
				library::string::join(
					machine->getDependsOn(),
					", "
				)
			);
		}

		auto message = std::format(
			" {} : {} {}",
			orderIndex + 1,
			machine->getName(),
			dependencyMessage
		);

		output << message << std::endl;
		orderIndex++;
	}
}

void Scheduler::displayLockoutSummary(std::ostream& output, const std::vector<int>& order) const
{
	output << "=== LOCKOUT SUMMARY (by Machine) ===" << std::endl;

	for (auto machineId : order)
	{
		const auto machine = this->_machines.at(machineId);
		std::vector<Device*> skipped;
		std::size_t locked = 0;

		for (auto device : machine->getRelatedDevices())
		{
			auto lockedDeviceIt = this->lockedDevices.find(device);

			if (lockedDeviceIt == this->lockedDevices.end())
				continue;

			locked++;

			if (lockedDeviceIt->second == machine)
				continue;

			skipped.push_back(device);
		}

		auto machineMessage = std::format(
			"Machine {} : {} -- {} locked, {} skipped",
			machine->getId(),
			machine->getName(),
			locked,
			skipped.size()
		);

		output << machineMessage << std::endl;

		for (const auto device : skipped)
		{
			auto deviceMessage = std::format(
				"\tSkipped: {} ({})",
				device->getId(),
				device->getName()
			);

			output << deviceMessage << std::endl;
		}
	}
}

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
	std::queue<Machine*> machineToExplore;

	for (const auto machine : this->_machines | std::views::values)
	{
		const auto dependencyCount = machine->getDependsOn().size();

		if (dependencyCount > 0)
		{
			machinesInDegree[machine] = dependencyCount;
			continue;
		}

		machineToExplore.push(machine);
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

			if (inDegreeIt->second > 0)
				continue;

			machinesInDegree.erase(inDegreeIt);
			machineToExplore.push(dependency);
		}
	}

	return !machinesInDegree.empty();
}

std::vector<int> Scheduler::topologicalSort(const std::function<bool(const Machine*, const Machine*)>& tieBreaker)
{
	std::unordered_map<Machine*, std::size_t> machinesInDegree;
	std::priority_queue<
		Machine*,
		std::vector<Machine*>,
		decltype(tieBreaker)
	> machinesToProcess(tieBreaker);

	for (const auto machine : this->_machines | std::views::values)
	{
		const auto dependencyCount = machine->getDependsOn().size();

		if (dependencyCount > 0)
		{
			machinesInDegree[machine] = dependencyCount;
			continue;
		}

		machinesToProcess.push(machine);
	}

	std::vector<int> machineOrder;

	while (!machinesToProcess.empty())
	{
		auto current = machinesToProcess.top();
		machinesToProcess.pop();

		machineOrder.push_back(current->getId());

		auto it = this->_dependencyGraph.find(current);

		if (it == this->_dependencyGraph.cend())
			continue;

		for (auto dependent : it->second)
		{
			const auto inDegreeIt = machinesInDegree.find(dependent);

			if (inDegreeIt == machinesInDegree.cend())
				continue;

			inDegreeIt->second--;

			if (inDegreeIt->second > 0)
				continue;

			machinesToProcess.push(dependent);
		}
	}

	// TODO: Check if the order is not reversed
	// The dependent should be locked before the dependency
	return machineOrder;
}

void Scheduler::schedule()
{
	std::unordered_map<Team*, std::unordered_map<Machine*, std::size_t>> devicesAssignedPerTeamPerMachine;
	std::unordered_map<Team*, std::size_t> teamNextFreeAt;

	const auto machinesId = this->topologicalSort(defaultTieBreaker);

	for (const auto machineId : machinesId)
	{
		const auto machine = this->_machines.at(machineId);

		for (const auto device : machine->getRelatedDevices())
		{
			// This device has already been locked
			if (this->lockedDevices.contains(device))
				continue;

			const auto deviceType = device->getType();
			Team* bestTeam = nullptr;
			std::size_t bestTeamLoad = __SIZE_MAX__;

			for (const auto team : this->_teams)
			{
				// Skip if the team can't handle the device type
				if (!team->canHandle(deviceType))
					continue;

				// Skip if the team can't handle more devices from this machine
				const auto maxCapacity = team->getMaxCapacity();

				if (maxCapacity != -1)
				{
					const auto currentHandledDeviceCount = devicesAssignedPerTeamPerMachine[team][machine];

					if (currentHandledDeviceCount >= maxCapacity)
						continue;
				}

				// If the team is the least occupied
				const auto teamLoad = teamNextFreeAt[team];

				if (teamLoad >= bestTeamLoad)
					continue;

				bestTeamLoad = teamLoad;
				bestTeam = team;
			}

			if (bestTeam == nullptr)
			{
				const auto message = std::format(
					"No team is fitted to handle the device '{}'.",
					device->getId()
				);

				throw std::logic_error(message);
			}

			const auto startTime = teamNextFreeAt[bestTeam];
			teamNextFreeAt[bestTeam] = startTime + device->getLockTime();
			devicesAssignedPerTeamPerMachine[bestTeam][machine]++;
			this->lockedDevices.insert({device, machine});
			this->gantt[bestTeam].emplace_back(device, startTime);
		}
	}
}

int Scheduler::getMakespan() const
{
	int highestTime = 0;

	for (const auto& entries : this->gantt | std::views::values)
	{
		for (const auto [device, startTime] : entries)
		{
			const auto currentTime = startTime + device->getLockTime();

			if (currentTime <= highestTime)
				continue;

			highestTime = static_cast<int>(currentTime);
		}
	}

	return highestTime;
}

void Scheduler::displaySummary()
{
	const auto order = this->topologicalSort(defaultTieBreaker);

	this->displayTopologicalOrder(std::cout, order);
	std::cout << std::endl;
	this->displayLockoutSummary(std::cout, order);
}

void Scheduler::displayGantt()
{
	std::cout << "T | ";

	for (const auto team : this->_teams)
		std::cout << team->getName() << " | ";

	std::cout << std::endl;

	const auto maxTime = this->getMakespan();

	for (int i = 0; i < maxTime; ++i)
	{
		std::cout << std::to_string(i) << " | ";

		for (auto team : this->_teams)
		{
			const ScheduleEntry* currentEntry = nullptr;

			for (const auto& entry : this->gantt[team])
			{
				const auto endTime = entry.startTime + entry.device->getLockTime();

				if (i < entry.startTime || i >= endTime)
					continue;

				currentEntry = &entry;
				break;
			}

			if (currentEntry != nullptr)
			{
				const auto currentDevice = currentEntry->device;

				if (i == currentEntry->startTime)
				{
					auto startEntryMessage = std::format(
						"{} ({})",
						currentDevice->getName(),
						currentDevice->getId()
					);

					std::cout << startEntryMessage;
				}
				else
					std::cout << "\"";
			}
			else
				std::cout << " ";

			std::cout << " | ";
		}

		std::cout << std::endl;
	}
}

void Scheduler::displayStats()
{
	std::cout << "=== STATISTICS ===" << std::endl;
	std::cout << std::format(
		"MAKESPAN : {} time units",
		this->getMakespan()
	) << std::endl;

	//       - Le nombre de devices et le temps total par équipe
	//       - L'équipe "goulot d'étranglement" (celle qui finit en dernier)

	std::cout << std::format(
		"Devices locked : {} / {}",
		this->lockedDevices.size(),
		this->_devices.size()
	) << std::endl;

	//       - Le nombre de "already locked" skips
	//       - L'efficacité : temps utile / (makespan × nb équipes) × 100
	//         Une efficacité de 100% signifie que toutes les équipes ont
	//         travaillé sans interruption jusqu'à la fin. Une efficacité
	//         basse signifie que certaines équipes finissent bien avant
	//         les autres (déséquilibre de charge).
}
