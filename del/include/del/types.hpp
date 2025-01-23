#pragma once

#include <cstddef>
#include <cstdint>


namespace del {
	using size_type = std::uint32_t;

	struct proposition_id
	{
		size_type id;

		// Equality operator for comparisons
    	bool operator==(const proposition_id& other) const {
        	return this->id == other.id;
    	}
		// Less-than operator for ordering
		bool operator<(const proposition_id& other) const {
			return this->id < other.id;
		}
	};

	struct agent_id
	{
		size_type id;
				// Equality operator for comparisons
    	bool operator==(const agent_id& other) const {
        	return this->id == other.id;
    	}
	};

	struct state_id
	{
		size_type id;
	};

	struct world_id
	{
		size_type id;
	};

	struct action_id
	{
		size_type id;
	};

	struct event_id
	{
		size_type id;
	};
}
