#include "del/formula.hpp"

#include <memory>
#include <sstream>

#include "del/domain.hpp"
#include "del/state.hpp"

#include <iostream>


namespace del
{
	formula::node_id formula::new_null()
	{
		node_id new_node_id{static_cast<size_type>(this->nodes.size())};
		this->nodes.emplace_back(formula_type::EMPTY);
		return new_node_id;
	}
	formula::node_id formula::new_top()
	{
		node_id new_node_id { static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::TOP);
		return new_node_id;
	}

	formula::node_id formula::new_bot()
	{
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::BOT);
		return new_node_id;
	}

	formula::node_id formula::new_prop(proposition_id p)
	{
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::PROP);
		this->nodes.emplace_back(p);
		return new_node_id;
	}

	formula::node_id formula::new_not(node_id f)
	{
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::NOT);
		this->nodes.emplace_back(f);
		return new_node_id;
	}

	formula::node_id formula::new_and(std::vector<node_id> conjuncts)
	{ /*AND , number of disjuncts, conjucts*/
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::AND);
		//number of conjuncts is the info contained in node after AND node
		this->nodes.emplace_back(static_cast<size_type>(conjuncts.size())); 
		for (node_id conjunct : conjuncts)
		{
			this->nodes.emplace_back(conjunct);
		}
		return new_node_id;
	}

	formula::node_id formula::new_or(std::vector<node_id> disjuncts)
	{ /*OR , number of disjuncts, disjuncts*/
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::OR);
		//number of disjuncts is the info contained in node after AND node
		this->nodes.emplace_back(static_cast<size_type>(disjuncts.size()));
		for (node_id disjunct : disjuncts)
		{
			this->nodes.emplace_back(disjunct);
		}
		return new_node_id;
	}

	formula::node_id formula::new_believes(agent_id a, node_id f)
	{ /*Believes , agent id, formula */
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::BELIEVES);
		//agent id is the next node
		this->nodes.emplace_back(a); 
		//formula comes next
		this->nodes.emplace_back(f);
		return new_node_id;
	}

	formula::node_id formula::new_everyone_believes(std::vector<agent_id> const & as, size_type order, node_id f)
	{ /* Everyone Believes , number of agents, agents ids, formula */
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::EVERYONE_BELIEVES);
		this->nodes.emplace_back(static_cast<size_type>(as.size())); //number of agents come next
		for (agent_id a : as)
		{
			this->nodes.emplace_back(a); //agent ids come next
		}
		this->nodes.emplace_back(order); //order comes next
		this->nodes.emplace_back(f); //formula comes next
		return new_node_id;
	}

	formula::node_id formula::new_common_belief(std::vector<agent_id> const & as, node_id f)
	{ /*Common Belief , number of agents, agents ids, formula */
		node_id new_node_id{ static_cast<size_type>(this->nodes.size()) };
		this->nodes.emplace_back(formula_type::COMMON_BELIEF);
		this->nodes.emplace_back(static_cast<size_type>(as.size())); //number of agents come next
		for (agent_id a : as)
		{
			this->nodes.emplace_back(a); //agent ids come next
		}
		this->nodes.emplace_back(f); //formula comes next
		return new_node_id;
	}
		bool formula::isNull(node_id n) const
		{
			return this->nodes[n.id].type == formula_type::EMPTY;
		}

	bool formula::evaluate(state const & s, world_id w, node_id n, util::bitset<>::common_state proposition_bitset_state) const
	{
		switch (this->nodes[n.id].type)
		{
			case formula::formula_type::TOP:
			{
				return true;
			}
			case formula::formula_type::BOT:
			{
				return false;
			}
			case formula::formula_type::PROP:
			{
				proposition_id p = this->nodes[n.id + 1].prop;
				return s.get_valuation(w, p, proposition_bitset_state);
			}
			case formula::formula_type::NOT:
			{
				node_id f = this->nodes[n.id + 1].nid; //next component of logical formula is evaluated
				return !this->evaluate(s, w, f, proposition_bitset_state);
			}
			case formula::formula_type::AND:
			{
				size_type count = this->nodes[n.id + 1].count;
				for (size_type i = 0; i < count; ++i)
				{
					node_id conjunct = this->nodes[n.id + 2 + i].nid;
					if (!this->evaluate(s, w, conjunct, proposition_bitset_state))
					{
						return false;
					}
				}
				return true;
			}
			case formula::formula_type::OR:
			{
				size_type count = this->nodes[n.id + 1].count;
				for (size_type i = 0; i < count; ++i)
				{
					node_id disjunct = this->nodes[n.id + 2 + i].nid;
					if (this->evaluate(s, w, disjunct, proposition_bitset_state))
					{
						return true;
					}
				}
				return false;
			}
			case formula::formula_type::BELIEVES:
			{
				agent_id a = this->nodes[n.id + 1].agent;
				node_id f = this->nodes[n.id + 2].nid;

				for (size_type vid = 0; vid < s.num_worlds; ++vid)
				{
					world_id v{ vid };

					//if f is false in any of the accessible worlds from the current one, then return false.
					if (s.get_accessible(a, w, v) && !this->evaluate(s, v, f, proposition_bitset_state))
					{
						return false;
					}
				}
				return true;
			}
			case formula::formula_type::EVERYONE_BELIEVES:
			{
				size_type num_agents = this->nodes[n.id + 1].count;
				size_type order = this->nodes[n.id + 2 + num_agents].count;
				node_id f = this->nodes[n.id + 2 + num_agents + 1].nid;

				// TODO: This could be more efficient for many agents if we calculated union of all in one pass.
				// TODO: Could save allocations by not having local workspace if we evaluate this often.
				util::bitset<> joint_R(s.Rcs);
				for (size_type a = 0; a < num_agents; ++a)
				{
					joint_R.inplace_union(s.Rcs, s.R[this->nodes[n.id + 2 + a].agent.id]);
				}

				// Check 'f' in all worlds accessible from 'w' by 'agents' with distance at most 'order'.
				// TODO: Could save allocations by not having local workspace if we evaluate this often.
				// BFS for reachable worlds, quadratic complexity because we can't efficiently iterate over accessible neighbors in bitsets.
				// Could perhaps do iterated union of accessible worlds instead of BFS; though this requires splitting joint_R into array of bitsets for each world (currently tightly packed).
				util::bitset<>::common_state vcs(s.num_worlds);
				util::bitset<> visited(vcs);

				// TODO: Vectors galore...
				std::vector<world_id> queue;
				queue.reserve(s.num_worlds);
				std::vector<world_id> next_queue;
				next_queue.reserve(s.num_worlds);

				size_type current_distance_class = 0;
				visited.set(vcs, w.id, true);
				queue.push_back(w);

				while (current_distance_class < order)
				{
					for (world_id v : queue)
					{
						if (!this->evaluate(s, v, f, proposition_bitset_state))
						{
							return false;
						}

						for (size_type v2 = 0; v2 < s.num_worlds; ++v2)
						{
							if (joint_R.get(s.Rcs, v.id * s.num_worlds + v2) && !visited.get(vcs, v2))
							{
								visited.set(vcs, v2, true);
								next_queue.emplace_back(world_id{ v2 });
							}
						}
					}

					std::swap(queue, next_queue);
					next_queue.clear();
					++current_distance_class;
				}
				for (world_id v : queue)
				{
					if (!this->evaluate(s, v, f, proposition_bitset_state))
					{
						return false;
					}
				}

				return true;
			}
			case formula::formula_type::COMMON_BELIEF: /*Common Belief not implemented */
			{
				// TODO: Implement.
				throw std::runtime_error("not implemented");
			}
			case formula::formula_type::EMPTY:
				return false; // Treat EMPTY as invalid
		}

#if defined(_MSC_VER)
		__assume(false);
#elif defined(__GNUG__) || defined(__clang__)
		__builtin_unreachable();
#else
		throw std::runtime_error("unreachable code");
#endif
	}

	std::string formula::to_string(domain const & d, node_id n) const
	{
		switch (this->nodes[n.id].type)
		{
		case formula::formula_type::TOP:
		{
			return "TOP";
		}
		case formula::formula_type::BOT:
		{
			return "BOT";
		}
		case formula::formula_type::PROP:
		{
			proposition_id p = this->nodes[n.id + 1].prop;
			return d.get_proposition_name(p);
		}
		case formula::formula_type::NOT:
		{
			node_id f = this->nodes[n.id + 1].nid;
			return "NOT(" + this->to_string(d, f) + ")";
		}
		case formula::formula_type::AND:
		{
			size_type count = this->nodes[n.id + 1].count;
			std::string s = "AND(";
			for (size_type i = 0; i < count; ++i)
			{
				node_id conjunct = this->nodes[n.id + 2 + i].nid;
				s += (i != 0 ? ", " : "") + this->to_string(d, conjunct);
			}
			return s + ")";
		}
		case formula::formula_type::OR:
		{
			size_type count = this->nodes[n.id + 1].count;
			std::string s = "OR(";
			for (size_type i = 0; i < count; ++i)
			{
				node_id disjunct = this->nodes[n.id + 2 + i].nid;
				s += (i != 0 ? ", " : "") + this->to_string(d, disjunct);
			}
			return s + ")";
		}
		case formula::formula_type::BELIEVES:
		{
			agent_id a = this->nodes[n.id + 1].agent;
			node_id f = this->nodes[n.id + 2].nid;
			return "BELIEVES[" + d.get_agent_name(a) + "](" + this->to_string(d, f) + ")";
		}
		case formula::formula_type::EVERYONE_BELIEVES:
		{
			std::stringstream buffer;
			buffer << "EVERYONE_BELIEVES[";
			size_type num_agents = this->nodes[n.id + 1].count;
			for (size_type a = 0; a < num_agents; ++a)
			{
				if (a != 0) buffer << ",";
				buffer << d.get_agent_name(this->nodes[n.id + 2 + a].agent);
			}
			size_type order = this->nodes[n.id + 2 + num_agents].count;
			node_id f = this->nodes[n.id + 2 + num_agents + 1].nid;

			buffer << "]^" << order << "(" << this->to_string(d, f) << ")";

			return buffer.str();
		}
		case formula::formula_type::COMMON_BELIEF:
		{
			// TODO: Implement.
			throw std::runtime_error("not implemented");
		}
		case formula::formula_type::EMPTY:
				return "EMPTY"; // Treat EMPTY as invalid
		}

#if defined(_MSC_VER)
		__assume(false);
#elif defined(__GNUG__) || defined(__clang__)
		__builtin_unreachable();
#else
		throw std::runtime_error("unreachable code");
#endif
	}
}
