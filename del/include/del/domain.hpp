#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "del/action.hpp"
#include "del/formula.hpp"
#include "del/state.hpp"
#include "del/types.hpp"

#include "del/util/bitset.hpp"




namespace del
{
	class domain {
	public:
		domain(std::vector<std::string> const & agents, std::vector<std::string> const & propositions, std::vector<bool> const & default_values);

		// Should not need these, but maybe for convenience of library consumers?
		domain(domain const &) = delete;
		domain & operator=(domain const &) = delete;
		domain(domain &&) = delete;
		domain & operator=(domain &&) = delete;

		size_type get_num_agents() const;
		agent_id get_agent_id(std::string const & name) const;
		std::string const & get_agent_name(agent_id id) const;
		
		size_type get_num_propositions() const;
		proposition_id get_proposition_id(std::string const & name) const;
		std::string const & get_proposition_name(proposition_id id) const;

		proposition_id get_sees_proposition_id(agent_id a1, agent_id a2) const;

		proposition_id get_attention_proposition_id(agent_id a, proposition_id p) const;

		std::vector<proposition_id> get_domain_non_attention_propositions_id() const;


		std::vector<proposition_id> get_domain_propositions_id() const;

		state const & get_state(state_id id) const; 
		action const & get_action(action_id id) const;

		state_id add_initial_state(std::vector<proposition_id> add);
		
		std::pair<action_id, state_id> perform_do(agent_id agent, std::vector<proposition_id> add, std::vector<proposition_id> del);

		//Represents the public attention shift on some proposition - version 1
		std::pair<action_id, state_id> perform_ac_bottom_up_v1(std::vector<agent_id>  agents_attention_shifter, std::vector<proposition_id> add, std::vector<proposition_id>  del);

		//Represents the public attention shift on some proposition - version 2
		std::pair<action_id, state_id> perform_ac_bottom_up_v2(std::vector<agent_id>  agents_attention_shifter, std::vector<proposition_id> add, std::vector<proposition_id>  del);

		//Represents the private attention shift of agent i on some proposition -version 1
		std::pair<action_id, state_id> perform_ac_top_down_v1(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del);

		//Represents the private attention shift of agent i on some proposition -version 2
		std::pair<action_id, state_id> perform_ac_top_down_v2(agent_id i, std::vector<proposition_id> add, std::vector<proposition_id>  del);

		// represents an observation change between two agents. It models changes in who observes whom, which is reflected in the postconditions and the accessibility relations.
		std::pair<action_id, state_id> perform_oc(std::vector<std::pair<agent_id, agent_id>> add, std::vector<std::pair<agent_id, agent_id>> del);

		bool evaluate_formula(state_id s, formula const & f, formula::node_id n) const;

		void print_state_overview(state const & s, std::vector<proposition_id> propositions) const ; 

		void others_agents_belief_regarding_attention(state_id s, agent_id a) const ; 

		// Retrieve agent and proposition from attention proposition
		std::pair<agent_id, proposition_id> get_agent_and_proposition(proposition_id attention_prop) const ;

		bool get_proposition_default_value(proposition_id p) const ;
	private:
		size_type num_agents;
		size_type num_propositions;
		size_type num_non_attention_propositions;
		util::bitset<>::common_state proposition_bitset_state; //efficient way to store the blocks and bits size of each propositions bitset

		std::vector<state> states;
		std::vector<action> actions;

		std::vector<std::string> agents;
		std::unordered_map<std::string, agent_id> agent_name_to_id;
		std::vector<std::string> propositions;
		std::vector<bool> propositions_default;
		std::unordered_map<std::string, proposition_id> prop_name_to_id;

		std::string get_sees_proposition_name(agent_id a1, agent_id a2) const;
		std::string get_attention_proposition_name(agent_id a, proposition_id p) const;
	

		std::vector<proposition_id> get_remaining_non_attention_propositions(std::vector<proposition_id> const propositions, std::vector<proposition_id> const propositions_set) const;
	};
}

namespace del::util
{

    std::vector<std::vector<proposition_id>> generate_subsets(const std::vector<proposition_id>& props);

}
