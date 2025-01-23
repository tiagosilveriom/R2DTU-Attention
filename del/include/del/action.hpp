#pragma once

#include <vector>

#include "del/formula.hpp"
#include "del/types.hpp"

#include "del/util/bitset.hpp"


namespace del
{
	/*
		E
		Q: A -> (E*E -> L_epis)
		Pre: E -> L_epis
		Post E -> (P -> {true, false, inert})
	*/
	class action {
		friend class domain; // For creating actions.
		friend class state; // For product update.

	public:
		/*
			Construct an action model where:
				- All preconditions are TOP.
				- All postcondiitons are no change.
				- No accessibility between events.
		*/
		action(size_type num_agents, size_type num_events, util::bitset<>::common_state proposition_bitset_state);

		// Need any of these?
		action(action const &) = delete; // deletes the copy constructor
		action & operator=(action const &) = delete; // deletes the copy assignment operator
		action(action &&) = default; //defaults the move constructor
		action & operator=(action &&) = default; //defaults the move assignment operator

	private:
		size_type num_events;

		// Stores all formulas that the action uses, the internal structures just point to formula nodes in this collection.
		// NB! Q and pre uses formulas in member initialization, so declaration order is important.
		formula formulas;

		// TODO: Please end this std::vector hell for storing collections which are static after construction.
		// Q tracks how different agents perceive the relations between different events, which is key to modeling knowledge updates and belief changes in multi-agent systems.
		std::vector<formula::node_id> Q; // (A x E x E) -> phi 
		std::vector<formula::node_id> pre;
		std::vector<util::bitset<>> post_add;
		std::vector<util::bitset<>> post_del;

		void set_pre(event_id e, formula::node_id f);
		formula::node_id get_pre(event_id e) const;

		void set_post(event_id e, proposition_id p, bool v, util::bitset<>::common_state proposition_bitset_state);
		util::bitset<> const & get_post_del(event_id e) const;
		util::bitset<> const & get_post_add(event_id e) const;

		void set_accessible(agent_id a, event_id e1, event_id e2, formula::node_id f);
		formula::node_id get_accessible(agent_id a, event_id e1, event_id e2) const;
	};
}
