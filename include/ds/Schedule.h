#ifndef INC_8INF259_TP3_SCHEDULE_H
#define INC_8INF259_TP3_SCHEDULE_H
#include <unordered_map>

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
		/// List of every device locked by the following machine 
		std::unordered_map<const Device*, const Machine*> _lockedDevices;

		/// List of every entry for each team
		std::unordered_map<const Team*, std::vector<ScheduleEntry>> _teamRecordEntries;

	public:
		/// Checks if the given device is locked
		[[nodiscard]] bool isDeviceLocked(const Device* device) const;

		/// Locks the given device by the given machine
		void lockDevice(const Team* team, Device* device, const Machine* machine, std::size_t time);

		/// Gets the machine responsible for the lock of the given device
		[[nodiscard]] const Machine* getLockResponsible(const Device* device) const;

		/// Gets the amount of devices locked
		[[nodiscard]] std::size_t getLockedCount() const;

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
