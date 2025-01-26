#include "del/domain.hpp"
#include <iostream>
#include <algorithm>
#include <map>
#include <set>




namespace del {
	domain::domain(std::vector<std::string> const & agents, std::vector<std::string> const & propositions, std::vector<bool> const & default_values) :
		// NB! proposition_bitset_state uses num_propositions to initialize, so order is important.
		
		num_non_attention_propositions(static_cast<size_type>(propositions.size())),
		//propositions + attention propositions
		num_agents(static_cast<size_type>(agents.size())), num_propositions(static_cast<size_type>(propositions.size() + agents.size()*propositions.size())),
		proposition_bitset_state(num_propositions),
		states(), actions(),
		agents(agents), agent_name_to_id(),
		propositions(propositions),propositions_default(default_values), prop_name_to_id()
	{
		//fill agent_name_to_id mapping
		for (size_type a = 0; a < this->num_agents; ++a)
		{
			this->agent_name_to_id[this->agents[a]] = agent_id{ a };
		}

		//fill prop_name_to_id mapping with propositions
		for (size_type p = 0; p < propositions.size(); ++p)
		{
			this->prop_name_to_id[this->propositions[p]] = proposition_id{ p };
		}

		//add attention propositions to the domain propositions set 
		for (std::string const & a : this->agents)
		{
			for (std::string const & p : propositions)
			{
				agent_id a_id = this->get_agent_id(a);
				proposition_id p_id = this->get_proposition_id(p);
				this->propositions.push_back(this->get_attention_proposition_name(a_id, p_id));
				this->propositions_default.push_back(true);
			}
		}


		//fill prop_name_to_id mapping attention propositions
		for (size_type p = propositions.size(); p < this->num_propositions; ++p)
		{
			this->prop_name_to_id[this->propositions[p]] = proposition_id{ p };
		}

		std::cout << "Size non attention proposition set: "<< num_non_attention_propositions <<"\n";
		// Prints just to control the domain proposition set
		std::cout << "Size proposition set: "<< num_propositions <<"\n";
		
		std::cout << "Size proposition default values set: "<< propositions_default.size() <<"\n";

		for (size_type p = 0; p < this->num_propositions; ++p)
		{
			std::cout << this->propositions[p] <<"  ";
			std::cout << this->get_proposition_default_value(this->prop_name_to_id[this->propositions[p]]) <<"\n";
		}
	}

	size_type domain::get_num_agents() const
	{
		return this->num_agents;
	}

	agent_id domain::get_agent_id(std::string const & name) const
	{
		return this->agent_name_to_id.at(name);
	}

	std::string const & domain::get_agent_name(agent_id id) const
	{
		return this->agents[id.id];
	}

	size_type domain::get_num_propositions() const
	{
		return this->num_propositions;
	}

	proposition_id domain::get_proposition_id(std::string const& name) const 
	{
		auto it = this->prop_name_to_id.find(name);
		if (it != this->prop_name_to_id.end()) {
			return it->second; // Found
		}
		throw std::out_of_range("Proposition not found: " + name + "\n");
	}
/*
	proposition_id domain::get_proposition_id(std::string const & name) const
	{
		std::cout<<"Proposition Name: "<< name <<"\n";
		std::cout<<"Proposition id: "<< this->prop_name_to_id.at(name).id <<"\n";
		return this->prop_name_to_id.at(name);
	}
*/
	std::string const & domain::get_proposition_name(proposition_id id) const
	{
		return this->propositions[id.id];
	}

	proposition_id domain::get_sees_proposition_id(agent_id a1, agent_id a2) const
	{
		return this->get_proposition_id(this->get_sees_proposition_name(a1, a2));
	}

	proposition_id domain::get_attention_proposition_id(agent_id a, proposition_id p) const
	{
		return this->get_proposition_id(this->get_attention_proposition_name(a, p));
	}

	std::vector<proposition_id> domain::get_domain_non_attention_propositions_id() const
	{
		std::vector<proposition_id> propositions_id;
		for(size_t i=0; i< this->num_non_attention_propositions; i++)
		{

			propositions_id.push_back(this->get_proposition_id(this->propositions[i]));
		}
		
		return propositions_id;
	}

	state const & domain::get_state(state_id id) const
	{
		return this->states[id.id];
	}

	action const & domain::get_action(action_id id) const
	{
		return this->actions[id.id];
	}

	std::vector<proposition_id> domain::get_domain_propositions_id() const
	{
		std::vector<proposition_id> propositions_id;
		for (auto p: this->propositions)
			propositions_id.push_back(this->get_proposition_id(p));
		
		return propositions_id;
	}

	state_id domain::add_initial_state(std::vector<proposition_id> add)
	{
		state_id s_id{ static_cast<size_type>(this->states.size()) };


		/*
		for (size_type a = 0; a < this->num_agents; ++a)
		{
			agent_id a_id{ a };
			s.set_accessible(a_id, w0, w0, true);
		}
		*/

		//	|non attentional propositions|=|At|/(1+|A|) 
		// Attention propositions from index |non attentional propositions|
		size_type num_non_attention_propositions= this->num_non_attention_propositions;

		std::map<std::vector<proposition_id>, std::vector<agent_id>> false_attention_map;

		// TODO: if time allows, improve false attention map code
		false_attention_map[{}].push_back(agent_id{ static_cast<size_type>(-1) } ); //agents with no false attention beliefs (pay attention to every prop) are stored here (-1 is omniscient agent)

		for ( size_type a=0; a<this->num_agents; a++)
		{
			agent_id a_id {a};

			std::vector<proposition_id> false_attention_set; // Each agent has a false attention set (a_id believes he is paying attention to p, but it's not)

			for(size_type i=0; i<num_non_attention_propositions;i++)
			{
				proposition_id p { i };
				proposition_id attention_p = this->get_attention_proposition_id(a_id,p);
				std::cout << "Attention p :"<<  this->get_proposition_name(attention_p)<< "\n";

				//confirm if attention_p is in add
				if (std::find(add.begin(), add.end(), attention_p) == add.end()) 
				{
            		false_attention_set.push_back(p);
        		}
			}
			/*
			std::cout << "False Attention Sets :" ;
			for (auto p : false_attention_set)
				std::cout << "\n"<< this->get_attention_proposition_name(a_id,p);
			*/
			
			false_attention_map[false_attention_set].push_back(a_id);
		}

		std::cout << "False Attention Sets :" << false_attention_map.size() << "\n";

		/*
		size_type num_states = (false_attention_map.size() == 1) 
    	? false_attention_map.size() 
    	: false_attention_map.size() + 1;
		*/
		size_type num_states = false_attention_map.size();

		std::cout << "Num states :" << num_states<< "\n";

		state & s = this->states.emplace_back(num_agents, num_states, this->proposition_bitset_state);

		// TODO: Hack to eliminate unreachable worlds propagating by ignoring them in the next product update. Replace by bisimulation contraction or similar model reduction.


		world_id w0 = world_id{ 0 };

		//set value of propositions contained in add to true at the initial world 
		/*
		for (auto p : add)
		{
			s.set_valuation(w0, p, true, this->proposition_bitset_state); //set value of proposition p in initial world to true
		}
		*/

		// Each false attention set create one world. This way, each agent beliefs he is paying attention to every object.
		// None of the propostions that belong to false attention set will be updated
		// The accessibility is defined by the agents mapped to each false attention set

		size_type w=0;
		// If agents have the same false_attention_set, they believe in the same world
		for (const auto& pair : false_attention_map) //Detail: This implementation assumes the agents falsely belief they pay attention to everything, and belif the same about the other agents
		{
			// Valuation
			world_id w_id{ w++ };
			for (auto p : add)
			{
				s.set_valuation(w_id, p, true, this->proposition_bitset_state); // Copy proposition true propositions from current world
			}


			std::vector<proposition_id> false_attention_set =pair.first; 
			std::cout << "Propositions Agents Falsely Believe To Be Paying Attention\n";
			for (auto p : false_attention_set) // Propositions Agents Falsely Believe To Be Paying Attention
			{
				std::cout << " p :"<<  this->get_proposition_name(p)<< " \n";

				s.set_valuation(w_id, p, this->get_proposition_default_value(p), this->proposition_bitset_state); 

				//s.set_valuation(w_id, p, false, this->proposition_bitset_state); //TODO: default value considered for every proposition is false, for now

				for(size_type a=0; a<this->num_agents ;a++) 
				{
					agent_id a_id{ a };
					s.set_valuation(w_id, this->get_attention_proposition_id(a_id,p), this->get_proposition_default_value(this->get_attention_proposition_id(a_id,p)), this->proposition_bitset_state); // Agents Falsely Believe To Be Paying Attention and all other agents
					//s.set_valuation(w_id, this->get_attention_proposition_id(a_id,p), true, this->proposition_bitset_state); // Agents Falsely Believe To Be Paying Attention and all other agents
				}
			}
/* //Uncomment this makes agents believing other agents are fully attentive also
			if(w_id.id != static_cast<size_type>(0))
			{ // Each agent also falsely beliefs all other agents are fully attentive

				for(size_type a=0; a<this->num_agents ;a++) 
				{
					agent_id a_id{ a };
					for( auto p : this->propositions)
					{
						try
						{
							proposition_id p_id = this->get_proposition_id(p);
							auto attention_p = this->get_attention_proposition_id(a_id, p_id);
							s.set_valuation(w_id, this->get_attention_proposition_id(a_id,p_id), this->get_proposition_default_value(this->get_attention_proposition_id(a_id,p_id)), this->proposition_bitset_state); // Agents Falsely Believe To Be Paying Attention and all other agents
						}
						catch(const std::out_of_range&)
						{
							continue;
						}
						
					}
				}
			}*/

			// Accessibility			
			std::vector<agent_id> agents_world_access = pair.second; 
			
			for( auto a_id : agents_world_access )
			{
				if(a_id.id==static_cast<size_type>(-1) ) continue; // -1 represents the "omniscient agent"

				s.set_accessible(a_id, w0, w_id, true);
			} 

			if(w_id.id != static_cast<size_type>(0))
			{
				for(size_type a=0; a<this->num_agents ;a++){
					agent_id a_id{ a };
					s.set_accessible(a_id, w_id, w_id, true);
				}
			}
		}
		
		for(size_type w=0; w<s.num_worlds;w++)
		{
			s.reachable_worlds.set(s.reachable_worlds_cs, w, true);  //set the world index  to reachable
		}

		//std::cout << "Number of worlds: " << s.get_num_worlds();
		return s_id;
	}
	
	std::pair<action_id, state_id> domain::perform_do(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id> del)
	{
		action_id do_action_id = { static_cast<size_type>(this->actions.size()) };
		

		//number of events in each action is not static anymore, it depends from the number of atoms involved in the post action (2^n)
		size_type num_events= 1 << (add.size()+del.size());

		action & do_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);


		event_id e0{ 0 };
		event_id e1{ 1 };

		// Preconditions: Always applicable by default.

		// Postconditions: No change by default.
		// e0 : event representing the "expected" changes after the action 
		// e1 : event representing no changes after the action
		for (auto p : add)
		{
			do_action.set_post(e0, p, true, this->proposition_bitset_state);
		}

		for (auto p : del)
		{
			do_action.set_post(e0, p, false, this->proposition_bitset_state);
		}
		
		std::vector<std::vector<proposition_id>> set_subsets_add = util::generate_subsets(add);
		std::vector<std::vector<proposition_id>> set_subsets_del = util::generate_subsets(del);

		// First and last subsets of each set are respectively empty and full

		std::vector<std::vector<proposition_id>> set_subsets_propositions; // contains the propostions each event pays attention

		std::vector<proposition_id>  subset_propositions=add;
		subset_propositions.insert(subset_propositions.end(), del.begin(), del.end()); 
    	
		set_subsets_propositions.push_back(subset_propositions);  // First element (e0)
		set_subsets_propositions.push_back({}); // Second element (e1)
		
		// set_post for remaining $num_event - 2$ events 
		for (auto subset_add : set_subsets_add)
		{
			for (auto subset_del : set_subsets_del)
			{
				if(subset_del.size()==del.size() && subset_add.size()==add.size()) continue; //e0
				if(subset_del.empty() && subset_add.empty()) continue; //e1

				subset_propositions.clear();
				subset_propositions.insert(subset_propositions.end(), subset_add.begin(), subset_add.end()); 
				subset_propositions.insert(subset_propositions.end(), subset_del.begin(), subset_del.end());
				
				set_subsets_propositions.push_back(subset_propositions); //size updated 
				 
				event_id current_e{ static_cast<del::size_type>(set_subsets_propositions.size() - 1)}; 

				for (auto p : subset_add)
				{
					do_action.set_post(current_e, p, true, this->proposition_bitset_state);
				}

				
				for (auto p : subset_del)
				{
					do_action.set_post(current_e, p, false, this->proposition_bitset_state);
				}				
			}
		}
		
		// Accessibility: Nothing accessible by default.
		formula & f = do_action.formulas;
		formula::node_id f_TOP = f.new_top();
		for (size_type j = 0; j < this->num_agents; ++j)
		{
			agent_id j_id{ j };	
			
			std::vector<formula::node_id>  action_all_attention_propositions, action_no_attention_propositions;
			subset_propositions.clear();
			subset_propositions=set_subsets_propositions[0]; //full set
			for(proposition_id p : subset_propositions)
			{
				action_all_attention_propositions.emplace_back(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p))));
				action_no_attention_propositions.emplace_back(f.new_not(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p)))));
			}
			
			formula::node_id j_pays_attention_to_all_p= f.new_and(action_all_attention_propositions);
			formula::node_id j_pays_no_attention= f.new_and(action_no_attention_propositions);

			do_action.set_accessible(j_id, e0, e0, j_pays_attention_to_all_p);

			//j_id accesses e0->e1 if j_id is not paying attention to any of the changed propositions in this action
			do_action.set_accessible(j_id, e0, e1, j_pays_no_attention);
			do_action.set_accessible(j_id, e1, e1, f_TOP);

			// e0 inaccessible from e1 by default.

			// All other events are accessible from e0, with the corresponding attention propositions as conditions.
			// Each event is accessible to itself with a tautology (always true) condition.
			
			for (size_t h = 2; h < set_subsets_propositions.size(); ++h) // Not Permanent: to avoid e0 and e1 propositions subsets
			{
				subset_propositions.clear();
    			subset_propositions = set_subsets_propositions[h];

				std::vector<formula::node_id> subset_attention_conditions;

				for(proposition_id p : subset_propositions)
				{
					subset_attention_conditions.emplace_back(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p))));
				}

				// set_subsets_propositions[0] is add + del propositions
				std::vector<proposition_id> inattention_propositions = this->get_remaining_non_attention_propositions(subset_propositions, set_subsets_propositions[0]);


				for (auto prop : inattention_propositions) {
					subset_attention_conditions.push_back(f.new_not(f.new_prop(this->get_attention_proposition_id(j_id, prop))));
				}

				formula::node_id j_attentive_to_subset = f.new_and(subset_attention_conditions);

				event_id current_e{ static_cast<del::size_type>(h) }; 
				
				//e0 -> current_e
				do_action.set_accessible(j_id, e0, current_e, j_attentive_to_subset);

				//do_action.set_pre(current_e, j_pays_attention_to_p);

				//current_e -> current_e
				do_action.set_accessible(j_id, current_e, current_e, f_TOP);
				 
			}
		}  

		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(do_action, this->num_agents, this->proposition_bitset_state));

		return { do_action_id, new_state_id };
	}

// Bottom up attention shift - version 1
	std::pair<action_id, state_id> domain::perform_minimal_bottom_up( std::vector<agent_id> agents_attention_shifter, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{

		size_type num_events= 1;

		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };

    	action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		event_id e0{0};

		for (auto i:agents_attention_shifter)
		{
			for (auto prop : del) {
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
				ac_action.set_post(e0, attention_agent_prop, false, this->proposition_bitset_state); // Remove attention
			}

			for (auto prop : add) {
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);

				ac_action.set_post(e0, attention_agent_prop, true, this->proposition_bitset_state); // Add attention
			}
		}

		// Accesibility: Nothing accessible by default.
		formula & f = ac_action.formulas;
		formula::node_id f_TOP = f.new_top();
		for (size_type j = 0; j < this->num_agents; ++j)
		{
			agent_id j_id{ j };

			ac_action.set_accessible(j_id, e0, e0, f_TOP);
		}

		// Apply the product update
		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}


// Bottom up attention shift - version 2

	std::pair<action_id, state_id> domain::perform_expanded_bottom_up( std::vector<agent_id> agents_attention_shifter, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{
		//DOUBT: Should exist an argument present_agents? So, only the present ones are updated?
		// Some events just include some agents' Bottom Up attention shifts (an agent appears on scenario, all other agents are aware that he is paying attention to certain propositions)
		// Other events include every agents' Bottom Up attention shifts (robot points to some box), all agents pay attention to that box and all other agents are aware of that)

		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };

		size_type num_events = 1 << add.size();  // 2^(|add|) possible subsets of add propositions

		action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		event_id e0{ 0 }; //e0 represents the event for the agents that performed the public attention shift (attention and prop value updated)

		//Temporary
		state_id last_state_id = { static_cast<size_type>(this->states.size() - 1) };
    	state const & last_state = this->get_state(last_state_id);

		//Valuation
		for(auto i : agents_attention_shifter){
			for (auto prop : del) {
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);

				for(size_type e_id=0; e_id< num_events; e_id++)
				{
					ac_action.set_post(event_id{e_id}, attention_agent_prop, false, this->proposition_bitset_state); // Remove attention
				}

			}
			for (auto prop : add) 
			{
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
				bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);

				for(size_type e_id=0; e_id< num_events; e_id++)
				{
					ac_action.set_post(event_id{e_id}, attention_agent_prop, true, this->proposition_bitset_state); // Add attention
				}

				ac_action.set_post(e0, prop, prop_actual_valuation, this->proposition_bitset_state); //e0 updates all propositions to the true value
			}
		}

    	formula & f = ac_action.formulas;
		formula::node_id f_TOP = f.new_top();

		//Accessibility

		for (size_type j = 0; j < this->num_agents; ++j) 
		{
			agent_id j_id{ j };
			std::vector<formula::node_id> attention_conditions;
			for (auto prop : add) 
			{
				attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
			}
			formula::node_id j_attentive_to_all_add = f.new_and(attention_conditions);

			//std::cout << "\nFormula for j_attentive_to_all_add: "<< f.to_string(*this, j_attentive_to_all_add) << "\n";
		    ac_action.set_accessible(j_id, e0, e0, j_attentive_to_all_add); // `e0` is accessible for agents paying attention to all added prop
		}

		// TODO: Remove because in good working, all the attention shifters must follow the condition j_attentive_to_all_add
		for (auto i : agents_attention_shifter) {
    		ac_action.set_accessible(i, e0, e0, f_TOP); // `e0` is accessible for the performing agents.
		}

		std::vector<std::vector<proposition_id>> add_subsets = util::generate_subsets(add);
		add_subsets.pop_back(); // Eliminate full set

		size_type event_counter=1;

		for (const auto &add_subset : add_subsets) {
			event_id current_event{ event_counter ++};
			// Set postconditions for add_subset: setting attention and proposition values
			for (auto prop : add_subset) {
				for (auto i : agents_attention_shifter) {
					bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state); 
					ac_action.set_post(current_event, prop, prop_actual_valuation, this->proposition_bitset_state); // TODO: Change for equivalent to Default Value later
				}
			}

			// Define accessibility conditions for each agent
			for (size_type j = 0; j < this->num_agents; ++j) 
			{
				agent_id j_id{ j };

				std::vector<formula::node_id> attention_conditions;
				for (auto prop : add_subset) {
					attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
				}
				
				// The agent is not attentive to the remaining propositions
				std::vector<proposition_id> inattention_propositions = this->get_remaining_non_attention_propositions(add_subset, add);
				for (auto prop : inattention_propositions) {
					attention_conditions.push_back(f.new_not(f.new_prop(this->get_attention_proposition_id(j_id, prop))));
				}

				// Combine the attention conditions for this agent on the current add subset
				formula::node_id j_attentive_to_subset = f.new_and(attention_conditions);

				// Set accessibility rules:
				// - The performing agent accesses the event unconditionally
				// - Other attentive agents access the event based on `j_attentive_to_subset`

				//TODO: (**) And to also make this condiiton for everyone, for agent i to not be aware of his attention shift
				//for (auto i : agents_attention_shifter) // TODO: MOST PROBABLY THE ISSUE IS HERE
				//{
					//if(!(j_id == i)) 
				//std::cout << "\nFormula for j_attentive_to_subset: "<< f.to_string(*this, j_attentive_to_subset) << "\n";

				for (auto i : agents_attention_shifter) 
				{
					if (!(j_id==i))ac_action.set_accessible(j_id, e0, current_event, j_attentive_to_subset);
				}	
				
				ac_action.set_accessible(j_id, current_event, current_event, f_TOP);

			}
    	}

		// Apply the product update
		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}


// Top down attention shift - version 1

	std::pair<action_id, state_id> domain::perform_private_top_down(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{

		size_type num_events= 2;

		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };

    	action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		state_id last_state_id = { static_cast<size_type>(this->states.size() - 1) };
    	state const & last_state = this->get_state(last_state_id);

		event_id agent_performer_event{0};
		event_id others_event{1};


		for (auto prop : del) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			ac_action.set_post(agent_performer_event, attention_agent_prop, false, this->proposition_bitset_state); // Remove attention
		}

		for (auto prop : add) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);

			ac_action.set_post(agent_performer_event, attention_agent_prop, true, this->proposition_bitset_state); // Add attention
			//ac_action.set_post(agent_performer_event, prop, prop_actual_valuation, this->proposition_bitset_state); // Add prop current valuation
		}

		// Accesibility: Nothing accessible by default.
		formula & f = ac_action.formulas;
		formula::node_id f_TOP = f.new_top();
		for (size_type j = 0; j < this->num_agents; ++j)
		{
			agent_id j_id{ j };

			if(j_id == i)
			{
				ac_action.set_accessible(j_id, agent_performer_event, agent_performer_event, f_TOP);

			}
			else{
				ac_action.set_accessible(j_id, agent_performer_event, others_event, f_TOP);
			}

			ac_action.set_accessible(j_id, others_event, others_event, f_TOP);

		}

		// Apply the product update
		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}


// Top Down attention shift - version 2.1 (in sync with the property if every agent believes all other agents also pay attention to everything)
/*
	std::pair<action_id, state_id> domain::perform_ac_top_down(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{

		size_type num_add_events = 1 << add.size();  // 2^(|add|) possible subsets of add propositions

		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };

		size_type num_events ;

		// If there are delete attention propositions, there is 1 more possible event, because agent i is the only aware of delete
		if(del.empty()) num_events=num_add_events;
		else num_events=num_add_events + 1;

    	action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		state_id last_state_id = { static_cast<size_type>(this->states.size() - 1) };
    	state const & last_state = this->get_state(last_state_id);


		// **Delete Event**: Only the performing agent knows about his attention shift
		event_id agent_performer_event{0};

		for (auto prop : del) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			ac_action.set_post(agent_performer_event, attention_agent_prop, false, this->proposition_bitset_state); // Remove attention
		}

		for (auto prop : add) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);

			ac_action.set_post(agent_performer_event, attention_agent_prop, true, this->proposition_bitset_state); // Add attention
			ac_action.set_post(agent_performer_event, prop, prop_actual_valuation, this->proposition_bitset_state); // Add prop current valuation
		}

    	formula & f = ac_action.formulas;
    	formula::node_id f_TOP = f.new_top();

		//TODO: (**) Add attention change with possibility for the agent to no be aware of delete
		// It might be to just have a boolean set this accessible or not. Not sure and 

		// Set accessibility: Only the performing agent has access to this event
		ac_action.set_accessible(i, agent_performer_event, agent_performer_event, f_TOP);


		// **Add Events**: Based on attention configurations
		std::vector<std::vector<proposition_id>> add_subsets = util::generate_subsets(add);
		size_type event_counter = 1;  // Start counting after delete_event
		
		// If there are no delete attentions, an agent who is aware of all the propositions, has access to the same event
		// If there are delete attentions, that does not happen because of the property **every agent beliefs the others are full attentive**
		if(del.empty()) // DOUBT: Does this work after public attention shifts ? (Bottom Up attention shift)
		{
			add_subsets.pop_back(); // Eliminate full set
			for (size_type j = 0; j < this->num_agents; ++j) {
				agent_id j_id{ j };

				if(j_id == i) continue;

				std::vector<formula::node_id> attention_conditions;
				for (auto prop : add) {
					attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
				}

				// Combine the attention conditions for this agent on the current add subset
				formula::node_id j_attentive_to_set = f.new_and(attention_conditions);

				// Set accessibility rules:
				ac_action.set_accessible(j_id, agent_performer_event, agent_performer_event, j_attentive_to_set);
			}
		}

    	for (const auto &add_subset : add_subsets) {
			event_id current_event{ event_counter++ };

			// Set postconditions for add_subset: setting attention and proposition values
			for (auto prop : add_subset) {
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
				bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);
				ac_action.set_post(current_event, attention_agent_prop, true, this->proposition_bitset_state);
				ac_action.set_post(current_event, prop, prop_actual_valuation, this->proposition_bitset_state); // TODO: Change for Default Value later
			}

			// Define accessibility conditions for each agent
			for (size_type j = 0; j < this->num_agents; ++j) 
			{
				agent_id j_id{ j };

				std::vector<formula::node_id> attention_conditions;
				for (auto prop : add_subset) {
					attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
				}
				
				// The agent is not attentive to the remaining propositions
				std::vector<proposition_id> inattention_propositions = this->get_remaining_non_attention_propositions(add_subset, add);
				for (auto prop : inattention_propositions) {
					attention_conditions.push_back(f.new_not(f.new_prop(this->get_attention_proposition_id(j_id, prop))));
				}

				// Combine the attention conditions for this agent on the current add subset
				formula::node_id j_attentive_to_subset = f.new_and(attention_conditions);

				// Set accessibility rules:
				// - The performing agent accesses the event unconditionally
				// - Other attentive agents access the event based on `j_attentive_to_subset`

				//TODO: (**) And to also make this condiiton for everyone, for agent i to not be aware of his attention shift
				if(!(j_id == i)) ac_action.set_accessible(j_id, agent_performer_event, current_event, j_attentive_to_subset);

				ac_action.set_accessible(j_id, current_event, current_event, f_TOP);

			}
    	}

		// Apply the product update
		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}
*/

// Top Down attention shift - version 2.2
	std::pair<action_id, state_id> domain::perform_conscious_top_down(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{
		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };
		

		//number of events in each action is not static anymore, it depends from the number of atoms involved in the post action (2^n)
		size_type num_events= 1 << (add.size()+del.size());

		action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		//Temporary
		state_id last_state_id = { static_cast<size_type>(this->states.size() - 1) };
    	state const & last_state = this->get_state(last_state_id);

		event_id e0{ 0 };
		event_id e1{ 1 };

		// Preconditions: Always applicable by default.

		// Postconditions: No change by default.
		// e0 : event representing all the attention shifts
		// e1 : event representing no changes after the action
		for (auto p : add)
		{
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, p);
			ac_action.set_post(e0, attention_agent_prop, true, this->proposition_bitset_state);

			bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(p, this->proposition_bitset_state); 
			ac_action.set_post(e0, p, prop_actual_valuation, this->proposition_bitset_state);
		}

		for (auto p : del)
		{
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, p);
			ac_action.set_post(e0, attention_agent_prop, false, this->proposition_bitset_state);
		}

		// Accessibility: Nothing accessible by default.
		formula & f = ac_action.formulas;
		formula::node_id f_TOP = f.new_top();

		//the agent who performs the private attention shift unconditionally accesses e0
		ac_action.set_accessible(i, e0, e0, f_TOP);
		ac_action.set_accessible(i, e1, e1, f_TOP);

		std::vector<std::vector<proposition_id>> set_subsets_add = util::generate_subsets(add);
		std::vector<std::vector<proposition_id>> set_subsets_del = util::generate_subsets(del);

		// First and last subsets of each set are respectively empty and full

		std::vector<std::vector<proposition_id>> set_subsets_propositions; // contains the propostions each event pays attention

		std::vector<proposition_id>  subset_propositions=add;
		subset_propositions.insert(subset_propositions.end(), del.begin(), del.end()); 
    	
		set_subsets_propositions.push_back(subset_propositions);  // First element (e0)
		set_subsets_propositions.push_back({}); // Second element (e1)
		
		// set_post for remaining $num_event - 2$ events 
		for (auto subset_add : set_subsets_add)
		{
			for (auto subset_del : set_subsets_del)
			{
				if(subset_del.size()==del.size() && subset_add.size()==add.size()) continue; //e0
				if(subset_del.empty() && subset_add.empty()) continue; //e1

				subset_propositions.clear();
				subset_propositions.insert(subset_propositions.end(), subset_add.begin(), subset_add.end()); 
				subset_propositions.insert(subset_propositions.end(), subset_del.begin(), subset_del.end());
				
				set_subsets_propositions.push_back(subset_propositions); //size updated 
				 
				event_id current_e{ static_cast<del::size_type>(set_subsets_propositions.size() - 1)}; 

				for (auto p : subset_add)
				{
					proposition_id attention_agent_prop = this->get_attention_proposition_id(i, p);
					//learning +h_a{p}
					ac_action.set_post(current_e, attention_agent_prop, true, this->proposition_bitset_state);

					//learning p
					bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(p, this->proposition_bitset_state); 
					ac_action.set_post(current_e, p, prop_actual_valuation, this->proposition_bitset_state);
				}
				
				for (auto p : subset_del)
				{
					proposition_id attention_agent_prop = this->get_attention_proposition_id(i, p);
					//learning -h_a{p}
					ac_action.set_post(current_e, attention_agent_prop, false, this->proposition_bitset_state);
				}				
			}
		}
		

		for (size_type j = 0; j < this->num_agents; ++j)
		{
			agent_id j_id{ j };	
			
			if (j_id == i) continue;

			std::vector<formula::node_id>  action_all_attention_propositions, action_no_attention_propositions;
			subset_propositions.clear();
			subset_propositions=set_subsets_propositions[0]; //full set
			for(proposition_id p : subset_propositions)
			{
				action_all_attention_propositions.emplace_back(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p))));
				action_no_attention_propositions.emplace_back(f.new_not(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p)))));
			}
			
			formula::node_id j_pays_attention_to_all_p= f.new_and(action_all_attention_propositions);
			formula::node_id j_pays_no_attention= f.new_and(action_no_attention_propositions);

			ac_action.set_accessible(j_id, e0, e0, j_pays_attention_to_all_p);

			//j_id accesses e0->e1 if j_id is not paying attention to any of the changed propositions in this action
			ac_action.set_accessible(j_id, e0, e1, j_pays_no_attention);
			ac_action.set_accessible(j_id, e1, e1, f_TOP);

			// e0 inaccessible from e1 by default.

			// All other events are accessible from e0, with the corresponding attention propositions as conditions.
			// Each event is accessible to itself with a tautology (always true) condition.
			
			for (size_t h = 2; h < set_subsets_propositions.size(); ++h) // Not Permanent: to avoid e0 and e1 propositions subsets
			{
				subset_propositions.clear();
    			subset_propositions = set_subsets_propositions[h];

				std::vector<formula::node_id> subset_attention_conditions;

				for(proposition_id p : subset_propositions)
				{
					subset_attention_conditions.emplace_back(f.new_prop( this->get_proposition_id(this->get_attention_proposition_name(j_id, p))));
				}

				// set_subsets_propositions[0] is add + del propositions
				std::vector<proposition_id> inattention_propositions = this->get_remaining_non_attention_propositions(subset_propositions, set_subsets_propositions[0]);


				for (auto prop : inattention_propositions) {
					subset_attention_conditions.push_back(f.new_not(f.new_prop(this->get_attention_proposition_id(j_id, prop))));
				}

				formula::node_id j_attentive_to_subset = f.new_and(subset_attention_conditions);

				event_id current_e{ static_cast<del::size_type>(h) }; 
				
				
				//e0 -> current_e
				ac_action.set_accessible(j_id, e0, current_e, j_attentive_to_subset);

				//do_action.set_pre(current_e, j_pays_attention_to_p);

				//current_e -> current_e
				ac_action.set_accessible(j_id, current_e, current_e, f_TOP);
				 
			}
		}  

		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}

/*
	std::pair<action_id, state_id> domain::perform_ac_top_down(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del)
	{

		size_type num_add_events = 1 << add.size();  // 2^(|add|) possible subsets of add propositions

		action_id ac_action_id = { static_cast<size_type>(this->actions.size()) };

		size_type num_events ;

		// If there are delete attention propositions, there is 1 more possible event, because agent i is the only aware of delete
		if(del.empty()) num_events=num_add_events;
		else num_events=num_add_events + 1;

    	action & ac_action = this->actions.emplace_back(this->num_agents, num_events, this->proposition_bitset_state);

		state_id last_state_id = { static_cast<size_type>(this->states.size() - 1) };
    	state const & last_state = this->get_state(last_state_id);


		// **Delete Event**: Only the performing agent knows about his attention shift
		event_id agent_performer_event{0};

		for (auto prop : del) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			ac_action.set_post(agent_performer_event, attention_agent_prop, false, this->proposition_bitset_state); // Remove attention
		}

		for (auto prop : add) {
			proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
			bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);

			ac_action.set_post(agent_performer_event, attention_agent_prop, true, this->proposition_bitset_state); // Add attention
			ac_action.set_post(agent_performer_event, prop, prop_actual_valuation, this->proposition_bitset_state); // Add prop current valuation
		}

    	formula & f = ac_action.formulas;
    	formula::node_id f_TOP = f.new_top();

		//TODO: (**) Add attention change with possibility for the agent to no be aware of delete
		// It might be to just have a boolean set this accessible or not. Not sure and 

		// Set accessibility: Only the performing agent has access to this event
		ac_action.set_accessible(i, agent_performer_event, agent_performer_event, f_TOP);


		// **Add Events**: Based on attention configurations
		std::vector<std::vector<proposition_id>> add_subsets = util::generate_subsets(add);
		size_type event_counter = 1;  // Start counting after delete_event
		
		// If there are no delete attentions, an agent who is aware of all the propositions, has access to the same event
		// If there are delete attentions, that does not happen because of the property **every agent beliefs the others are full attentive**
		if(del.empty()) // DOUBT: Does this work after public attention shifts ? (Bottom Up attention shift)
		{
			add_subsets.pop_back(); // Eliminate full set
			for (size_type j = 0; j < this->num_agents; ++j) {
				agent_id j_id{ j };

				if(j_id == i) continue;

				std::vector<formula::node_id> attention_conditions;
				for (auto prop : add) {
					attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
				}

				// Combine the attention conditions for this agent on the current add subset
				formula::node_id j_attentive_to_set = f.new_and(attention_conditions);

				// Set accessibility rules:
				ac_action.set_accessible(j_id, agent_performer_event, agent_performer_event, j_attentive_to_set);
			}
		}

    	for (const auto &add_subset : add_subsets) {
			event_id current_event{ event_counter++ };

			// Set postconditions for add_subset: setting attention and proposition values
			for (auto prop : add_subset) {
				proposition_id attention_agent_prop = this->get_attention_proposition_id(i, prop);
				bool prop_actual_valuation = last_state.get_prop_valuation_actual_world(prop, this->proposition_bitset_state);
				ac_action.set_post(current_event, attention_agent_prop, true, this->proposition_bitset_state);
				ac_action.set_post(current_event, prop, prop_actual_valuation, this->proposition_bitset_state); // TODO: Change for Default Value later
			}

			// Define accessibility conditions for each agent
			for (size_type j = 0; j < this->num_agents; ++j) 
			{
				agent_id j_id{ j };

				std::vector<formula::node_id> attention_conditions;
				for (auto prop : add_subset) {
					attention_conditions.push_back(f.new_prop(this->get_attention_proposition_id(j_id, prop)));
				}
				
				// The agent is not attentive to the remaining propositions
				std::vector<proposition_id> inattention_propositions = this->get_remaining_non_attention_propositions(add_subset, add);
				for (auto prop : inattention_propositions) {
					attention_conditions.push_back(f.new_not(f.new_prop(this->get_attention_proposition_id(j_id, prop))));
				}

				// Combine the attention conditions for this agent on the current add subset
				formula::node_id j_attentive_to_subset = f.new_and(attention_conditions);

				// Set accessibility rules:
				// - The performing agent accesses the event unconditionally
				// - Other attentive agents access the event based on `j_attentive_to_subset`

				//TODO: (**) And to also make this condiiton for everyone, for agent i to not be aware of his attention shift
				if(!(j_id == i)) ac_action.set_accessible(j_id, agent_performer_event, current_event, j_attentive_to_subset);

				ac_action.set_accessible(j_id, current_event, current_event, f_TOP);

			}
    	}

		// Apply the product update
		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(ac_action, this->num_agents, this->proposition_bitset_state));

		return { ac_action_id, new_state_id };
	}
*/
	
	std::pair<action_id, state_id> domain::perform_oc(std::vector<std::pair<agent_id, agent_id>> add, std::vector<std::pair<agent_id, agent_id>> del)
	{
		action_id oc_action_id = { static_cast<size_type>(this->actions.size()) };
		action & oc_action = this->actions.emplace_back(this->num_agents, 2, this->proposition_bitset_state);

		event_id e0{ 0 };
		event_id e1{ 1 };

		// Preconditions: Always applicable by default.

		// e0 is the event that represents the "expected" changes after the observation change
		// e1 is the event that represents no changes after the observation change
		// Postconditions: No change by default.
		std::vector<bool> observers(this->num_agents, false);
		for (auto [a1, a2] : add)
		{
			observers[a1.id] = true; //set true add agent pairs (a1 sees a2)
			oc_action.set_post(e0, this->get_sees_proposition_id(a1, a2), true, this->proposition_bitset_state);
		}
		for (auto [a1, a2] : del)
		{
			observers[a1.id] = true; //set true add agent pairs (a1 does not see a2)
			oc_action.set_post(e0, this->get_sees_proposition_id(a1, a2), false, this->proposition_bitset_state);
		}

		// Accesibility: Nothing accessible by default.
		formula & f = oc_action.formulas;
		formula::node_id f_TOP = f.new_top();
		for (size_type j = 0; j < this->num_agents; ++j)
		{
			agent_id j_id{ j };
			
			if (observers[j])  oc_action.set_accessible(j_id, e0, e0, f_TOP);
			else oc_action.set_accessible(j_id, e0, e1, f_TOP);
			oc_action.set_accessible(j_id, e1, e1, f_TOP);
			
			// e0 inaccessible from e1 by default.
		}

		state_id new_state_id = { static_cast<size_type>(this->states.size()) };
		state & new_state = this->states.emplace_back(this->states.back().product_update(oc_action, this->num_agents, this->proposition_bitset_state));

		return { oc_action_id, new_state_id };
	}

	bool domain::evaluate_formula(state_id s, formula const & f, formula::node_id n) const
	{
		return f.evaluate(this->get_state(s), world_id{ 0 }, n, this->proposition_bitset_state);
	}

	std::string domain::get_sees_proposition_name(agent_id a1, agent_id a2) const
	{
		return this->get_agent_name(a1) + "_sees_" + this->get_agent_name(a2);
	}

	std::string domain::get_attention_proposition_name(agent_id a, proposition_id p) const
	{
		return this->get_agent_name(a) + "_is_paying_attention_to_" + this->get_proposition_name(p);
	}
/*
	std::pair<agent_id, proposition_id> domain::get_agent_and_proposition(proposition_id attention_prop) const {
		auto it = this->attention_map.find(attention_prop.id);
		if (it != attention_map.end()) {
			return it->second;
		} else {
			throw std::invalid_argument("Attention proposition not found.");
		}
	}
	*/
	void domain::print_state_overview(state const & s, std::vector<proposition_id> const propositions) const
	{
		
		size_type num_worlds= s.get_num_worlds();
		for (size_type wid = 0; wid < num_worlds; ++wid)
		{
			if(! s.get_reachable_world_boolean(wid) ) continue;
			
			world_id w{wid};
			std::cout << "------ World " << wid << " ------\n" ;
			for (proposition_id p : propositions)				
				std::cout << get_proposition_name(p) << " = " << (s.get_valuation(w, p, this->proposition_bitset_state) ? "true" : "false") << ".\n";

			std::cout << "Accessibility: \n" ;	

			for (size_type aid = 0; aid< this->num_agents; ++aid)
			{
				agent_id a {aid};
				std::cout << " " << this->get_agent_name(a) <<  ": " ;		

				for (size_type vid = 0; vid < num_worlds; ++vid)
				{
					std::cout << "w" << vid << ": "<<  (s.get_accessible(a , w, world_id{ vid }) ? "true" : "false") << "|" ;
				}
				std::cout << "\n";	
			}	
		
		}
	}
	
	void domain::others_agents_belief_regarding_attention(state_id s, agent_id a ) const 
	{
		std::cout << "------ Other agents belief regarding " << this->get_agent_name(a) << " attention ------\n" ;

		auto non_attention_propositions = this->get_domain_non_attention_propositions_id();

		formula f;
		for(auto name: this->agents)
		{
			bool some_attention=false;
			std::cout << "***** " << name << " beliefs ****\n" ;

			for(auto p : non_attention_propositions)
			{
				auto f_believes_attention_p = f.new_believes(this->get_agent_id(name),f.new_prop(this->get_attention_proposition_id(a,p)));
				
				if(this->evaluate_formula(s, f, f_believes_attention_p))
				{
					std::cout << f.to_string(*this, f_believes_attention_p)<<"\n" ;
					some_attention= true;
				}
			}
		    if(!some_attention) std::cout <<"Agent "<<name <<"believes " << this->get_agent_name(a) << "is not paying attention to any proposition \n" ;
		}

	}

	std::vector<proposition_id> domain::get_remaining_non_attention_propositions(std::vector<proposition_id> propositions_subset, std::vector<proposition_id> propositions_set) const
	{	
		std::vector<proposition_id> remaining_non_attention_propositions;

		for(auto p : propositions_set) {
			// Check if prop_id is NOT in the passed propositions vector
			if (std::find(propositions_subset.begin(), propositions_subset.end(), p) == propositions_subset.end()) {
				// If prop_id is not in the propositions vector, add it to remaining_non_attention_propositions
				remaining_non_attention_propositions.push_back(p);
			}
		}
		/*
		std::cout <<"\nAdd : ";
		for(auto p: propositions_subset)
		{
			std::cout << this->get_proposition_name(p) << " " ;
		}
		std::cout <<"\nRemaining: ";
		for(auto p: remaining_non_attention_propositions)
		{
			std::cout << this->get_proposition_name(p) << " " ;
		}
		*/
		return remaining_non_attention_propositions;
	}

	bool domain::get_proposition_default_value(proposition_id p) const
	{
		return this->propositions_default[p.id];
	} 

}

	namespace del::util {

    std::vector<std::vector<proposition_id>> generate_subsets(const std::vector<proposition_id>& props) {

        std::vector<std::vector<proposition_id>> subsets;
        size_t n = props.size();
        
        // There are 2^n subsets, so we iterate through all numbers from 0 to 2^n - 1
        for (size_t i = 0; i < static_cast<size_t>(1 << n); ++i) {
            std::vector<proposition_id> subset;
            for (size_t j = 0; j < n; ++j) {
                // Check if the j-th element is included in the subset (if j-th bit of i is 1) Each elemnt is treated as a bit from i representation
                if (i & (1 << j)) {
                    subset.push_back(props[j]);
                }
            }
            subsets.push_back(subset);
        }
        
        //first and last elements of subsets are empty and full subset respectively
        return subsets;
    }
	}