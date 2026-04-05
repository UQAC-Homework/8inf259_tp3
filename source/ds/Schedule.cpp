#include "../../include/ds/Schedule.h"

#include <format>
#include <ranges>

bool ds::Schedule::isDeviceLocked(const Device* device) const
{
	return this->_lockedDevices.contains(device);
}

void ds::Schedule::lockDevice(const Team* team, Device* device, size_t time)
{
	this->_lockedDevices.insert(device);
	this->_teamRecordEntries[team].emplace_back(device, time);
}

void ds::Schedule::recordSkippedDevice(const Machine* machine, const Device* device)
{
	this->_skippedDevices[machine].emplace_back(device);
}

std::size_t ds::Schedule::getLockedCount() const
{
	return this->_lockedDevices.size();
}

std::size_t ds::Schedule::getSkippedCount() const
{
	std::size_t count = 0;

	for (const auto& devices : this->_skippedDevices | std::views::values)
		count += devices.size();

	return count;
}

std::vector<const Device*> ds::Schedule::getSkippedDevices(const Machine* machine) const
{
	const auto skippedIt = this->_skippedDevices.find(machine);

	if (skippedIt == this->_skippedDevices.end())
		return {};

	return skippedIt->second;
}

std::size_t ds::Schedule::getTeamLockCount(const Team* team) const
{
	const auto recordIt = this->_teamRecordEntries.find(team);

	if (recordIt == this->_teamRecordEntries.end())
		return 0;

	return recordIt->second.size();
}

std::size_t ds::Schedule::getTotalDuration() const
{
	std::size_t highestTime = 0;

	for (const auto team : this->_teamRecordEntries | std::views::keys)
	{
		const auto currentTime = this->getTeamTotalDuration(team);

		if (currentTime <= highestTime)
			continue;

		highestTime = currentTime;
	}

	return highestTime;
}

std::size_t ds::Schedule::getTeamTotalDuration(const Team* team) const
{
	const auto entriesIt = this->_teamRecordEntries.find(team);

	if (entriesIt == this->_teamRecordEntries.end())
		return 0;

	std::size_t highestTime = 0;

	for (const auto [device, startTime] : entriesIt->second)
	{
		const auto currentTime = startTime + device->getLockTime();

		if (currentTime <= highestTime)
			continue;

		highestTime = currentTime;
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
	double totalUsedTime = 0;

	for (const auto& entries : this->_teamRecordEntries | std::views::values)
	{
		for (const auto [device, startTime] : entries)
			totalUsedTime += device->getLockTime();
	}

	return totalUsedTime / static_cast<double>(totalDuration) * 100.0;
}
