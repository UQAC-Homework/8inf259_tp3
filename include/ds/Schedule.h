#ifndef INC_8INF259_TP3_SCHEDULE_H
#define INC_8INF259_TP3_SCHEDULE_H
#include <unordered_map>
#include <unordered_set>

#include "../../Device.h"
#include "../../Machine.h"
#include "../../Team.h"

namespace ds
{
	struct ScheduleEntry
	{
	public:
		Device* device;
		std::size_t startTime;
	};

	struct Schedule
	{
	private:
		/// List of every device locked
		std::unordered_set<const Device*> _lockedDevices;

		/// List of every entry for each team
		std::unordered_map<const Team*, std::vector<ScheduleEntry>> _teamRecordEntries;

		/// List of every device skipped by a given machine
		std::unordered_map<const Machine*, std::vector<const Device*>> _skippedDevices;

	public:
		/// Checks if the given device is locked
		[[nodiscard]] bool isDeviceLocked(const Device* device) const;

		/// Locks the given device by the given machine
		void lockDevice(const Team* team, Device* device, size_t time);

		/// Records that the given machine has skipped the given device
		void recordSkippedDevice(const Machine* machine, const Device* device);

		/// Gets the amount of devices locked
		[[nodiscard]] std::size_t getLockedCount() const;

		/// Gets the amount of devices locking skipped
		[[nodiscard]] std::size_t getSkippedCount() const;

		/// Gets the devices skipped by the given machine
		[[nodiscard]] std::vector<const Device*> getSkippedDevices(const Machine* machine) const;

		/// Gets the amount of locks from the given team
		[[nodiscard]] std::size_t getTeamLockCount(const Team* team) const;

		/// Gets the total duration of this schedule
		[[nodiscard]] std::size_t getTotalDuration() const;

		/// Gets the total duration of the given from
		[[nodiscard]] std::size_t getTeamTotalDuration(const Team* team) const;

		/// Gets the record entry from the given team recorded within the given time
		const ScheduleEntry* getRecordEntry(const Team* team, std::size_t time) const;

		/// Gets the percentage of the total duration that was not used on a task
		double getEfficiency() const;
	};
}


#endif //INC_8INF259_TP3_SCHEDULE_H
