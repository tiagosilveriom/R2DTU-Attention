#include "DEL_Operations.hpp"

namespace del {

	// TODO - Check definition of applicable, does there have to be at least one designated world left?
	State perform_product_update(const State& state, const Action& action) {
		//Agent_Id agent = action.get_owner();
		std::vector<World_Entry> new_worlds;
		State result(state.get_number_of_agents());

		for (auto& world : state.get_worlds()) {
			for (const auto& event : action.get_events()) {
				if (world.valuate(event.get_preconditions())) {
					// TODO - Maybe handle unreachable worlds here

					World& updated_world = result.create_world();
					updated_world.add_true_propositions(world.get_true_propositions());
					updated_world.remove_true_propositions(event.get_delete_list());
					updated_world.add_true_propositions(event.get_add_list());

					if (state.is_world_designated(world.get_id()) && action.is_event_designated(event.get_id())) {
						result.add_designated_world(updated_world.get_id());
					}

					new_worlds.emplace_back(world.get_id(), event.get_id(), updated_world.get_id());
				}
			}
		}

		for (auto world1 : new_worlds) {
			for (auto world2 : new_worlds) {
				for (size_t i = 0; i < state.get_number_of_agents(); i++) {
					Agent_Id agent = Agent_Id{ i };
					if (state.is_one_reachable(agent, world1.old_world, world2.old_world) &&
						action.is_one_reachable(agent, world1.old_event, world2.old_event)) {
						result.add_indistinguishability_relation(agent, world1.new_world, world2.new_world);
					}
				}
			}
		}
		return std::move(result);
	}
	// Using definition: All states reachable by 'agent' from any designated world, 
	// and the resulting worlds must be closed under 'agent' (any world should be reachable from any other world by 'agent')
	State perform_perspective_shift(const State& state, Agent_Id agent_id) {

		std::vector<World_Id> frontier;
		// Using size_t instead of World_Id to avoid specifying custom hash function for World_Id
		std::unordered_set<size_t> visited;
		for (auto designated_world : state.get_designated_worlds()) {
			frontier.push_back(designated_world);
			//visited.push_back(designated_world);
			visited.insert(designated_world.id);
		}
		while (!frontier.empty()) {
			auto current = frontier.back();
			frontier.pop_back();
			for (auto relation : state.get_indistinguishability_relations(agent_id)) {
				if (relation.world_from == current &&
					std::find(visited.begin(), visited.end(), relation.world_to.id) == visited.end()) {

					frontier.push_back(relation.world_to);
					visited.insert(relation.world_to.id);
				}
			}
		}

		State result(state.get_number_of_agents());
		// Using size_t instead of World_Id to avoid specifying custom hash function for World_Id
		std::unordered_map<size_t, size_t> old_to_new_mapping;
		for (auto world : visited) {
			auto& new_world = result.create_world(state.get_world(World_Id{ world }));
			result.add_designated_world(new_world.get_id());
			old_to_new_mapping[world] = new_world.get_id().id;
		}
		for (size_t agent_number = 0; agent_number < state.get_number_of_agents(); agent_number++) {
			auto current_agent = Agent_Id{ agent_number };
			for (auto& relation : state.get_indistinguishability_relations(current_agent)) {
				if (visited.find(relation.world_from.id) != visited.end() && visited.find(relation.world_to.id) != visited.end()) {
					result.add_indistinguishability_relation(
						current_agent, 
						World_Id{ old_to_new_mapping[relation.world_from.id] }, 
						World_Id{ old_to_new_mapping[relation.world_to.id] });
				}
			}
		}

		// TODO - Maybe add indistinguishability for the perspective shifting agent, 
		// such that it may not distinguish between any of the new designated worlds

		return std::move(result);
	}

	std::vector<State> split_into_global_states(const State& state, const Agent_Id agent) {
		std::vector<State> result;
		for (auto designated_world : state.get_designated_worlds()) {
			State new_state = State(state);
			new_state.set_global_for_agent(agent, designated_world);
			result.push_back(std::move(new_state));
		}
		return result;
	}


	bool is_action_applicable(const State& state, const Action& action) {
		auto worlds = state.get_designated_world_reachables(action.get_owner());

		for (auto& world_id : worlds) {
			auto& world = state.get_world(world_id);
			bool found_applicable_event = false;
			for (auto event : action.get_events()) {
				if (world.valuate(event.get_preconditions())) {
					found_applicable_event = true;
				}
			}
			if (!found_applicable_event) {
				return false;
			}
		}
		return true;
	}

	bool are_states_bisimilar(const State& state1, const State& state2) {
		Bisimulation_Context bisimulation_context(state1, state2);
		return bisimulation_context.is_bisimilar();
	}
}