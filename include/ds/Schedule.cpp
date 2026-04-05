#include "Schedule.h"

#include <format>
#include <ranges>

ds::Schedule::Schedule() = default;

bool ds::Schedule::isDeviceLocked(const Device* device) const
{
	return this->lockedDevices.contains(device);
}

void ds::Schedule::lockDevice(const Team* team, Device* device, Machine* machine, std::size_t time)
{
	this->lockedDevices.insert({device, machine});
	this->_teamRecordEntries[team].emplace_back(device, time);
}

const Machine* ds::Schedule::getLockResponsible(const Device* device) const
{
	const auto lockedDeviceIt = this->lockedDevices.find(device);

	if (lockedDeviceIt == this->lockedDevices.end())
		return nullptr;

	return lockedDeviceIt->second;
}

std::size_t ds::Schedule::getLockedCount() const
{
	return this->lockedDevices.size();
}

std::size_t ds::Schedule::getTotalDuration() const
{
	std::size_t highestTime = 0;

	for (const auto& entries : this->_teamRecordEntries | std::views::values)
	{
		for (const auto [device, startTime] : entries)
		{
			const auto currentTime = startTime + device->getLockTime();

			if (currentTime <= highestTime)
				continue;

			highestTime = currentTime;
		}
	}

	return highestTime;
}

const ds::ScheduleEntry* ds::Schedule::getRecordEntry(const Team* team, const std::size_t time) const
{
	const auto teamIt = this->_teamRecordEntries.find(team);

	if (teamIt == this->_teamRecordEntries.end())
		return nullptr;

	for (const auto& entry : teamIt->second)
	{
		const auto endTime = entry.startTime + entry.device->getLockTime();

		if (time < entry.startTime || time >= endTime)
			continue;

		return &entry;
	}

	return nullptr;
}

double ds::Schedule::getEfficiency() const
{
	const auto totalDuration = this->getTotalDuration() * this->_teamRecordEntries.size();
	auto totalUsedTime = 0;

	for (const auto& entries : this->_teamRecordEntries | std::views::values)
	{
		for (const auto [device, startTime] : entries)
			totalUsedTime += device->getLockTime();
	}

	return totalUsedTime / static_cast<double>(totalDuration) * 100.0;
}

ds::Schedule ds::Schedule::createSchedule(const std::vector<Machine*>& machines, const std::vector<Team*>& teams)
{
	Schedule schedule;
	std::unordered_map<Team*, std::unordered_map<Machine*, std::size_t>> devicesAssignedPerTeamPerMachine;
	std::unordered_map<Team*, std::size_t> teamNextFreeAt;

	for (const auto machine : machines)
	{
		for (const auto device : machine->getRelatedDevices())
		{
			if (schedule.isDeviceLocked(device))
				continue;

			const auto deviceType = device->getType();
			Team* bestTeam = nullptr;
			std::size_t bestTeamLoad = __SIZE_MAX__;

			for (const auto team : teams)
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

			schedule.lockDevice(bestTeam, device, machine, startTime);
		}
	}

	return schedule;
}
