#include "Schedule.h"

#include <ranges>

bool ds::Schedule::isDeviceLocked(Device* device) const
{
	return this->lockedDevices.contains(device);
}

void ds::Schedule::lockDevice(const Team* team, Device* device, Machine* machine, std::size_t time)
{
	this->lockedDevices.insert({device, machine});
	this->gantt[team].emplace_back(device, time);
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

const ds::ScheduleEntry* ds::Schedule::getRecordEntry(const Team* team, const std::size_t time) const
{
	const auto teamIt = this->gantt.find(team);

	if (teamIt == this->gantt.end())
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

std::size_t ds::Schedule::getTotalDuration() const
{
	std::size_t highestTime = 0;

	for (const auto& entries : this->gantt | std::views::values)
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
