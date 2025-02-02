#include "Action_Event.hpp"

namespace del {

	Event_Id Action_Event::get_id() const {
		return id;
	}

	const std::unordered_set<std::string> Action_Event::get_add_list() const {
		return proposition_add;
	}

	const std::unordered_set<std::string> Action_Event::get_delete_list() const {
		return proposition_delete;
	}

	const Formula& Action_Event::get_preconditions() const {
		return precondition;
	}

	std::string Action_Event::to_string() const {
		std::string result = "Event " + std::to_string(id.id) + ": (Preconditions: " + precondition.to_string() + ") (Add list";
		for (auto add : proposition_add) {
			result += ", " + add;
		}
		result += ") (Delete list, ";
		for (auto delete_entry : proposition_delete) {
			result += ", " + delete_entry;
		}
		result += ")";
		return result;
	}
}