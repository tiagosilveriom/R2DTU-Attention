#include "del/action.hpp"


namespace del
{
	action::action(size_type num_agents, size_type num_events, util::bitset<>::common_state proposition_bitset_state) :
		num_events(num_events), formulas(),							//tautology for pre condition for every event
		Q(num_agents * num_events * num_events, formulas.new_bot()), pre(num_events, formulas.new_top()), post_add(), post_del()
		// NB! Q and pre uses formulas, so declaration order is important.
	{
		this->post_add.reserve(num_events); 
		this->post_del.reserve(num_events);
		for (size_type e = 0; e < num_events; ++e)
		{
			this->post_add.emplace_back(proposition_bitset_state); //every event representation is initialized with the current state proposition set 
			this->post_del.emplace_back(proposition_bitset_state);
		}
	}

	void action::set_pre(event_id e, formula::node_id f) 
	{
		this->pre[e.id] = f;
	} 

	formula::node_id action::get_pre(event_id e) const
	{
		return this->pre[e.id];
	}

	void action::set_post(event_id e, proposition_id p, bool v, util::bitset<>::common_state proposition_bitset_state)
	{
		if (v) this->post_add[e.id].set(proposition_bitset_state, p.id, true); //add if v true
		else this->post_del[e.id].set(proposition_bitset_state, p.id, true);  // delete if v false
	}

	util::bitset<> const & action::get_post_del(event_id e) const
	{
		return this->post_del[e.id];
	}

	util::bitset<> const & action::get_post_add(event_id e) const
	{
		return this->post_add[e.id];
	}

	void action::set_accessible(agent_id a, event_id e1, event_id e2, formula::node_id f)
	{
		this->Q[a.id * this->num_events * this->num_events + e1.id * this->num_events + e2.id] = f;
	}

	formula::node_id action::get_accessible(agent_id a, event_id e1, event_id e2) const
	{
		return this->Q[a.id * this->num_events * this->num_events + e1.id * this->num_events + e2.id];
	}
}
