#include "del/state.hpp"

#include "del/action.hpp"
#include "del/formula.hpp"

#include <iostream> 


namespace del
{
	state::state(size_type num_agents, size_type num_worlds, util::bitset<>::common_state proposition_bitset_state) :
		num_worlds(num_worlds),
		Rcs(num_worlds * num_worlds), R(), V() 
		// TODO: Hack to eliminate unreachable worlds propagating by ignoring them in the next product update. Replace by bisimulation contraction or similar model reduction.
		, reachable_worlds_cs(num_worlds), reachable_worlds(reachable_worlds_cs) //DOUBT: still not sure how these two work
	{

		/* Space for the Accessibility relations at the state */
		this->R.reserve(num_agents);
		for (size_type a = 0; a < num_agents; ++a)
		{
			this->R.emplace_back(this->Rcs);
		}

		/* Valuation Functions at the state */
		this->V.reserve(num_worlds);
		for (size_type w = 0; w < num_worlds; ++w)
		{
			this->V.emplace_back(proposition_bitset_state);
		}
	}

	state state::product_update(action const & a, size_type num_agents, util::bitset<>::common_state proposition_bitset_state) const
	{
		// current state worlds and action events 
		std::vector<std::pair<world_id, event_id>> new_worlds; // TODO: This vector workspace could/should be external if product update is in hot path.

		/* For each world at the current state, evaluate which events are possible
			and if it's, create world */
		for (size_type w = 0; w < this->num_worlds; ++w)
		{
			world_id w_id{ w }; // current state world
			//std::cout<< "reachable : " << this->reachable_worlds.get(this->reachable_worlds_cs, w) <<"\n";
			for (size_type e = 0; e < a.num_events; ++e)
			{
				event_id e_id{ e };

				// evaluate if this world at this current state fulfills preconditions for event e_id
				if (a.formulas.evaluate(*this, w_id, a.get_pre(e_id), proposition_bitset_state)) 
				{
					// TODO: Hack to eliminate unreachable worlds propagating by ignoring them in the next product update. Replace by bisimulation contraction or similar model reduction.
					//std::cout<< "World: " << w_id.id << "| Event: "<< e_id.id<< "\n";

					if (!this->reachable_worlds.get(this->reachable_worlds_cs, w))
					{
						continue; 
					}

					new_worlds.emplace_back(w_id, e_id); // add pairs <new world, event>
				}
			}
		}

		// TODO: New constructor which doesn't make empty bitsets first, but does the copy+del+add in one pass (constructor).
		// We could check if the optimizer is smart enough already, but it's probably not an automatically deducible optimization.
		state new_state(num_agents, static_cast<size_type>(new_worlds.size()), proposition_bitset_state);
		
		//std::cout << "\nNew Worlds size: " << new_worlds.size() <<" | Old Worlds Size: "<< this->num_worlds << " \n";

		
		for(auto world : new_worlds)
		{
			//std::cout<< "\nWorld: " << world.first.id << "| Event: "<< world.second.id;
		}
		

		for (size_type nw1 = 0; nw1 < new_state.num_worlds; ++nw1)
		{
			auto const &[w_id, e_id] = new_worlds[nw1]; //nw1 is result of w_id world from former state with e_id event consequences

			// Valuation.
			new_state.V[nw1]
				.copy(proposition_bitset_state, this->V[w_id.id]) // copies the proposition valuation from the current state's world w_id.id into the new state's world nw1
				.inplace_difference(proposition_bitset_state, a.post_del[e_id.id]) //  postcondition of the action that indicates which propositions should be set to false
				.inplace_union(proposition_bitset_state, a.post_add[e_id.id]); // postcondition of the action that indicates which propositions should be set to true

			// Accessibility.
			for (size_type agent = 0; agent < num_agents; ++agent)
			{
				agent_id a_id{ agent };
				
				/*
				std::cout<< "----------------------------";
				std::cout<< "AGENT : "<< agent <<" \n";
				std::cout<< "----------------------------";
				*/

				for (size_type nw2 = 0; nw2 < new_state.num_worlds; ++nw2)
				{
					auto const &[v_id, f_id] = new_worlds[nw2]; //nw2 is result of v_id world from former state with f_id event consequences
				
					//std::cout<< "\nWorld: " << w_id.id << "| Event: "<< e_id.id<< "-->" << "World: " << v_id.id << "| Event: "<< f_id.id;
					//std::cout<< "\nFormer worlds accessible :"<< this->get_accessible(a_id, w_id, v_id)<< "Events accessible: "<< a.formulas.evaluate(*this, w_id, a.get_accessible(a_id, e_id, f_id), proposition_bitset_state)<< "\n";
					
					/*
					std::cout << "Agent: " << a_id.id << ", World: " << w_id.id 
          			<< " -> World: " << v_id.id 
          			<< ", Event: " << e_id.id << " -> Event: " << f_id.id
          			<< ", Formula: " << a.formulas.to_string(d, a.get_accessible(a_id, e_id, f_id)) << "\n";
					std::cout<< "\nFormer worlds accessible :"<< this->get_accessible(a_id, w_id, v_id)<< "Events accessible: "
					<< a.formulas.evaluate(*this, w_id, a.get_accessible(a_id, e_id, f_id), proposition_bitset_state)<< " Sally is paying attention: "<<
					(get_valuation(w_id, d.get_proposition_id("sally_is_paying_attention_to_marble_in_table"), proposition_bitset_state) ? "true" : "false") << "\n";
					*/


					if (this->get_accessible(a_id, w_id, v_id) && a.formulas.evaluate(*this, w_id, a.get_accessible(a_id, e_id, f_id), proposition_bitset_state))
					{
						/* new world 1 (nw1) -> new world 2 (nw2) if:
							- respective worlds from former state also fulfill the access relation (w_id -> v_id)
							- required formula for accessibility between events is true
						*/
						
						//std::cout<< "\nWorld: " << w_id.id << "| Event: "<< e_id.id<< "-->" << "World: " << v_id.id << "| Event: "<< f_id.id  ;
						//std::cout<< "\nFormer worlds accessible :"<< this->get_accessible(a_id, w_id, v_id)<< "Events accessible: "<< a.formulas.evaluate(*this, w_id, a.get_accessible(a_id, e_id, f_id), proposition_bitset_state)<< "\n";
						//std::cout<< " TRUE !\n";
						//std::cout<<"##################################\n";

						new_state.set_accessible(a_id, world_id{ nw1 }, world_id{ nw2 }, true);
					}
				//	std::cout<<"##################################\n";
				}
			}
		}

		// TODO: Hack to eliminate unreachable worlds propagating by ignoring them in the next product update. Replace by bisimulation contraction or similar model reduction.
		{
			// Union of all accessibility relations.
			util::bitset<> joint_R(new_state.Rcs);
			for (size_type a = 0; a < num_agents; ++a)
			{
				joint_R.inplace_union(new_state.Rcs, new_state.R[a]);
			}

			// BFS from designated world.
			std::vector<size_type> queue;
			queue.reserve(new_state.num_worlds);
			std::vector<size_type> next_queue;
			next_queue.reserve(new_state.num_worlds);

			new_state.reachable_worlds.set(new_state.reachable_worlds_cs, 0, true);
			queue.push_back(0);

			while (!queue.empty())
			{
				for (size_type w : queue)
				{
					for (size_type v = 0; v < new_state.num_worlds; ++v)
					{
						if (joint_R.get(new_state.Rcs, w * new_state.num_worlds + v) && !new_state.reachable_worlds.get(new_state.reachable_worlds_cs, v))
						{
							new_state.reachable_worlds.set(new_state.reachable_worlds_cs, v, true);
							next_queue.emplace_back(v);
						}
					}
				}

				std::swap(queue, next_queue);
				next_queue.clear();
			}
		}

		return new_state;
	}
	bool state::get_prop_valuation_actual_world(proposition_id p, util::bitset<>::common_state proposition_bitset_state) const
	{
		world_id actual_w{0};
		return this->get_valuation(actual_w,p, proposition_bitset_state);
	}
	bool state::get_reachable_world_boolean(size_type w) const
	{
		return this->reachable_worlds.get(this->reachable_worlds_cs, w);
	}

	bool state::get_valuation(world_id w, proposition_id p, util::bitset<>::common_state proposition_bitset_state) const
	{
		return this->V[w.id].get(proposition_bitset_state, p.id);
	}

	void state::set_valuation(world_id w, proposition_id p, bool v, util::bitset<>::common_state proposition_bitset_state)
	{
		this->V[w.id].set(proposition_bitset_state, p.id, v);
	}

	bool state::get_accessible(agent_id a, world_id w1, world_id w2) const
	{
		return this->R[a.id].get(this->Rcs, w1.id * this->num_worlds + w2.id);
	}

	void state::set_accessible(agent_id a, world_id w1, world_id w2, bool v)
	{
		this->R[a.id].set(this->Rcs, w1.id * this->num_worlds + w2.id, v);
	}

	size_type state::get_num_worlds() const
	{
		return this->num_worlds;
	}

	
}
