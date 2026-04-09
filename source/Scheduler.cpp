#include "../Scheduler.h"

#include <format>
#include <iostream>
#include <queue>
#include <ranges>
#include <unordered_set>

#include "../include/ds/Schedule.h"
#include "../include/library/string.h"

bool Scheduler::defaultTieBreaker(const Machine* a, const Machine* b)
{
	return a->getId() < b->getId();
}

void Scheduler::displayTopologicalOrder(std::ostream& output, const std::vector<int>& order) const
{
	std::unordered_map<int, int> machineOrder;

	for (int i = 0; i < order.size(); i++)
	{
		const auto id = order.at(i);

		machineOrder[id] = i + 1;
	}

	output << "=== TOPOLOGICAL ORDER ===" << std::endl;
	auto orderIndex = 0;

	for (auto machineId : order)
	{
		const auto machine = this->_machines.at(machineId);

		std::string dependencyMessage;

		if (!machine->getDependsOn().empty())
		{
			std::vector<int> dependencyOrder;
			
			for (const auto dependency : machine->getDependsOn())
			{
				const auto index = machineOrder.at(dependency);
				dependencyOrder.push_back(index);
			}

			dependencyMessage = std::format(
				"[after: {}]",
				library::string::join(
					dependencyOrder,
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
		std::vector<const Device*> skipped = this->lastSchedule.getSkippedDevices(machine);

		auto machineMessage = std::format(
			"Machine {} : {} -- {} locked, {} skipped",
			machine->getId(),
			machine->getName(),
			machine->getRelatedDevices().size() - skipped.size(),
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
	this->_devices = {};
	this->_machines = machines;
	this->_teams = teams;
	this->_dependencyGraph = {};

	this->_devices.reserve(devices.size());

	for (const auto device : devices | std::views::values)
		this->_devices.push_back(device);
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

	for (const auto device : this->_devices)
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

			this->_dependencyGraph[dependency].push_back(machine);
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

	return machineOrder;
}

void Scheduler::schedule()
{
	const auto machinesId = this->topologicalSort(defaultTieBreaker);
	ds::Schedule schedule;

	std::unordered_map<Team*, std::unordered_map<const Machine*, std::size_t>> devicesAssignedPerTeamPerMachine;
	std::unordered_map<Team*, std::size_t> teamNextFreeAt;

	for (const auto machineId : machinesId)
	{
		const auto machine = this->_machines.at(machineId);

		for (const auto device : machine->getRelatedDevices())
		{
			if (schedule.isDeviceLocked(device))
			{
				schedule.recordSkippedDevice(machine, device);
				continue;
			}

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

				if (maxCapacity != NO_LIMIT_CAPACITY)
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

			schedule.lockDevice(bestTeam, device, startTime);
		}
	}

	this->lastSchedule = schedule;
}

int Scheduler::getMakespan() const
{
	std::size_t highestTime = 0;

	for (const auto team : this->_teams)
	{
		const auto currentTime = this->lastSchedule.getTeamTotalDuration(team);

		if (currentTime <= highestTime)
			continue;

		highestTime = currentTime;
	}

	return static_cast<int>(highestTime);
}

void Scheduler::displaySummary()
{
	const auto order = this->topologicalSort(defaultTieBreaker);

	this->displayTopologicalOrder(std::cout, order);
	std::cout << std::endl;
	this->displayLockoutSummary(std::cout, order);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Scheduler::displayGantt()
{
	const auto maxTime = this->getMakespan();
	const auto teamCount = this->_teams.size();

	const auto columnCount = teamCount + 1;
	const auto rowCount = maxTime + 1;

	std::vector<std::string> cells;
	cells.reserve(rowCount * columnCount);

	cells.emplace_back("T");

	for (const auto team : this->_teams)
		cells.emplace_back(team->getName());

	for (int i = 0; i < maxTime; ++i)
	{
		cells.emplace_back(std::to_string(i));

		for (const auto team : this->_teams)
		{
			const auto currentEntry = this->lastSchedule.getRecordEntry(team, i);

			if (currentEntry == nullptr)
			{
				cells.emplace_back(" ");
				continue;
			}

			const auto currentDevice = currentEntry->device;

			if (i != currentEntry->startTime)
			{
				cells.emplace_back("\"");
				continue;
			}

			auto startEntryMessage = std::format(
				"{} ({})",
				currentDevice->getName(),
				currentDevice->getId()
			);

			cells.emplace_back(startEntryMessage);
		}
	}

	std::vector<std::size_t> maxWidth;
	maxWidth.reserve(columnCount);

	for (int i = 0; i < columnCount; ++i)
	{
		std::size_t biggestWidth = 0;

		for (int j = 0; j < rowCount; ++j)
		{
			const auto width = cells.at(j * columnCount + i).length();

			if (width <= biggestWidth)
				continue;

			biggestWidth = width;
		}

		maxWidth.emplace_back(biggestWidth);
	}

	for (int i = 0; i < cells.size(); ++i)
	{
		const auto& cell = cells.at(i);
		const auto cellColumnIndex = i % columnCount;
		const auto widthDifference = maxWidth.at(cellColumnIndex) - cell.length();

		std::cout << cell;

		if (widthDifference > 0)
			std::cout << std::string(widthDifference, ' ');

		std::cout << " | ";

		if (cellColumnIndex == columnCount - 1)
			std::cout << std::endl;
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Scheduler::displayStats()
{
	const auto maxTime = this->getMakespan();

	std::cout << "=== STATISTICS ===" << std::endl;
	std::cout << std::format(
		"MAKESPAN : {} time units",
		maxTime
	) << std::endl;

	for (const auto team : this->_teams)
	{
		const auto teamMaxTime = this->lastSchedule.getTeamTotalDuration(team);
		std::string bottleneckMessage;

		if (teamMaxTime == maxTime)
			bottleneckMessage = "*** BOTTLENECK ***";

		auto teamMessage = std::format(
			"Team {} : {} devices, total_time={}, max_capacity={} {}",
			team->getName(),
			this->lastSchedule.getTeamLockCount(team),
			teamMaxTime,
			team->getMaxCapacity(),
			bottleneckMessage
		);

		std::cout << "\t" << teamMessage << std::endl;
	}

	std::cout << std::format(
		"Devices locked : {} / {}",
		this->lastSchedule.getLockedCount(),
		this->_devices.size()
	) << std::endl;

	std::cout << std::format(
		"Already-locked skips : {}",
		this->lastSchedule.getSkippedCount()
	) << std::endl;

	std::size_t totalUsedTime = 0;

	for (const auto team : this->_teams)
		totalUsedTime += this->lastSchedule.getTeamTotalDuration(team);

	std::cout << std::format(
		"Efficiency : {:.1f}% (useful_time / total_available_time)",
		static_cast<double>(totalUsedTime) / static_cast<double>(maxTime * this->_teams.size()) * 100.0
	) << std::endl;
}
