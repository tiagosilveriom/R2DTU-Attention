#pragma once

#include <string>
#include <vector>

#include "del/types.hpp"

#include "del/util/bitset.hpp"


namespace del
{
	class domain; // TODO: Only used for to_string.
	class state;

	/*
		L_epis:
			phi ::= p | !p | phi ^ phi | B_i phi | C_B phi

		ATTENTION IMPLEMENTATION: Attention as logical formula will have to be added here 
		TODO: More efficient representation? We can encode TOP, BOT, and PROP as implicit indices (assuming formula associated with a static domain for PROP).
	*/

	/*EXPLANATION:
	 Each formula in the system is composed of multiple nodes. Each node represents a specific component or part of the formula.
	 These nodes work together to form a complete logical expression. The types of nodes include:

		-Tautologies or contradictions
		-Propositions (basic logical statements).
		-Logical operators like negation (NOT), conjunction (AND), or disjunction (OR).
		-Belief operators (e.g., B_i, where agent i believes a formula).
		-Group belief operators like common belief (C_B).
	 The process of building a formula involves creating and linking these nodes, each containing one piece of information,
	  such as the type of formula (e.g., a belief operator), the specific agent or proposition involved, or the logical connection between nodes.
	*/
	class formula { 
	public:
		struct node_id
		{
			size_type id;
		};

		formula() = default;

		// Need any of these?
		formula(formula const &) = delete;
		formula & operator=(formula const &) = delete;
		formula(formula &&) = default;
		formula & operator=(formula &&) = default;

		node_id new_null();
		node_id new_top();
		node_id new_bot();
		node_id new_prop(proposition_id p);
		node_id new_not(node_id f);
		node_id new_and(std::vector<node_id> conjuncts);
		node_id new_or(std::vector<node_id> disjuncts);
		node_id new_believes(agent_id a, node_id f);
		node_id new_everyone_believes(std::vector<agent_id> const & as, size_type order, node_id f);
		node_id new_common_belief(std::vector<agent_id> const & as, node_id f);

		bool evaluate(state const & s, world_id w, node_id n, util::bitset<>::common_state proposition_bitset_state) const;

		bool isNull(node_id n) const;
		// TODO: Only for debugging.
		std::string to_string(domain const & d, node_id n) const;

	private:
		enum class formula_type : size_type
		{
			TOP, //tautology— a formula that is always true
			BOT, //contradiction— a formula that is always false
			PROP,
			NOT,
			AND,
			OR,
			BELIEVES,
			EVERYONE_BELIEVES,
			COMMON_BELIEF,
			EMPTY
		};

		// With a union, memory is allocated only for the largest member of the union at any given time, so only one of the union's variables can be stored in that memory space at a time.
		union node 
		{
			formula_type type;
			proposition_id prop;
			node_id nid;
			agent_id agent;
			size_type count;

			node(formula_type type) : type(type) {};
			node(proposition_id prop) : prop(prop) {};
			node(node_id nid) : nid(nid) {};
			node(agent_id agent) : agent(agent) {};
			node(size_type count) : count(count) {};
		};

		std::vector<node> nodes;
	};
}
