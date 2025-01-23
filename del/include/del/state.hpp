#pragma once

#include <vector>

#include "del/types.hpp"

#include "del/util/bitset.hpp"


namespace del
{
	class action;

	/*
		W
		R: A -> 2^(W*W) The accessibility relation represents what worlds are possible from the perspective of an agent.
		V: P -> 2^W (equivalently W -> 2^P) The valuation function specifies which propositions are true in each world. It tells us, for a given world w, what atomic propositions hold.
		S: A*A -> 2^W 
	*/
	class state {
		friend class domain; // TODO: For setting initial state?
		friend class formula;

	public:
		state(size_type num_agents, size_type num_worlds, util::bitset<>::common_state proposition_bitset_state);

		// TODO: Need any of these?
		state(state const &) = delete;
		state & operator=(state const &) = delete;
		state(state &&) = default;
		state & operator=(state &&) = default;

		state product_update(action const & a, size_type num_agents, util::bitset<>::common_state proposition_bitset_state) const;

		size_type get_num_worlds() const;

		bool get_prop_valuation_actual_world(proposition_id prop, util::bitset<>::common_state proposition_bitset_state) const;

		bool get_reachable_world_boolean(size_type w) const;
	private:
		size_type num_worlds; 

		// TODO: Please end this std::vector hell for storing collections which are static after construction.
		// TODO: Replace vectors with unique_ptr to array, probably using uninitialized allocation and placement new to construct bitsets at offsets.
		// TODO: Bitsets should take their working memory as an argument instead of doing their own individual allocations; makes collections of bitsets more efficient.
		// TODO: Rcs is only really parameterized on num_worlds, which many states might share; we could extract it out one step.
		util::bitset<>::common_state Rcs;
		std::vector<util::bitset<>> R;
		std::vector<util::bitset<>> V;

		// TODO: Hack to eliminate unreachable worlds propagating by ignoring them in the next product update. Replace by bisimulation contraction or similar model reduction.
		util::bitset<>::common_state reachable_worlds_cs; // DOUBT: still not sure what it exactly is
		util::bitset<> reachable_worlds; // DOUBT: still not sure what it exactly is

		bool get_valuation(world_id w, proposition_id p, util::bitset<>::common_state proposition_bitset_state) const;
		void set_valuation(world_id w, proposition_id p, bool v, util::bitset<>::common_state proposition_bitset_state);

		bool get_accessible(agent_id a, world_id w1, world_id w2) const;
		void set_accessible(agent_id a, world_id w1, world_id w2, bool v);
	};
}
