#include <iostream>

#include "del/domain.hpp"


/*
	TODO (GENERAL):
		- Bisimulation contraction instead of current hack to reduce model size.
		- Get rid of vectors when structures are of known fixed size at runtime; e.g. states, actions.
			+ This would also allow us to rid ourselves of the unintended move ctors/ops; ease transition by changing vector to list first.
		- Support for designated worlds/events (currently w0 and e0 are implicitly designated).
		- Verify/correct that includes, forward declarations, and friend declarations are minimal.
*/


int main(int argc, char* argv[])
{
	std::cout << "Hello world.\n\n";
#pragma warning(suppress: 4996)
	std::cout << "PATH = " << std::getenv("PATH") << "\n\n";

	using namespace del;
	
	//constexpr is a keyword that specifies that the value of a variable or the result of a function can be evaluated at compile time, rather than at runtime. 
	constexpr auto cs_size = sizeof(util::bitset<>::common_state);
	constexpr auto bs_size = sizeof(util::bitset<>);

	constexpr auto domain_size = sizeof(domain);
	constexpr auto state_size = sizeof(state);
	constexpr auto formula_size = sizeof(formula);
	constexpr auto action_size = sizeof(action);

	// Prepare domain.
	std::vector<std::string> agents = { "sally", "anne" }; //Ag
	std::vector<std::string> propositions = { "marble_in_basket", "marble_in_box", "marble_in_table"}; //At
	std::vector<bool> propositions_default_value = { false, false, false}; 

	domain d(agents, propositions, propositions_default_value); // creates domain, filling with agents and propositions information (including sees and attention propositions)

	std::cout << "\nDomain Succesfully Created\n";


	agent_id sally = d.get_agent_id("sally");
	agent_id anne = d.get_agent_id("anne");
	proposition_id marble_in_basket = d.get_proposition_id("marble_in_basket");
	proposition_id marble_in_box = d.get_proposition_id("marble_in_box");
	proposition_id marble_in_table = d.get_proposition_id("marble_in_table");


	//Save attention propositions ids
	proposition_id sally_attention_marble_in_basket = d.get_attention_proposition_id(sally,marble_in_basket);
	proposition_id sally_attention_marble_in_box = d.get_attention_proposition_id(sally,marble_in_box);
	proposition_id sally_attention_marble_in_table = d.get_attention_proposition_id(sally,marble_in_table);

	proposition_id anne_attention_marble_in_basket = d.get_attention_proposition_id(anne,marble_in_basket);
	proposition_id anne_attention_marble_in_box = d.get_attention_proposition_id(anne,marble_in_box);
	proposition_id anne_attention_marble_in_table = d.get_attention_proposition_id(anne,marble_in_table);

	std::vector<proposition_id> propositions_id = d.get_domain_propositions_id();

	std::cout << "Propositions: \n\n";
	for(auto p: propositions_id)
	{
		std::cout << "ID: "<< p.id << "| Name: " << d.get_proposition_name(p) << "|Default: "<< d.get_proposition_default_value(p)<<"\n";
	}
	// Verifying Attention Propositions
	
	//	std::cout << d.get_proposition_name(sally_attention_marble_in_basket) << "\n";
	//	std::cout << d.get_proposition_name(sally_attention_marble_in_box) << "\n";
	//	std::cout << d.get_proposition_name(anne_attention_marble_in_basket) << "\n";
	//	std::cout << d.get_proposition_name(anne_attention_marble_in_box) << "\n";

	// Create initial state with different attention atoms to different agents 
	// Perform actions (marble_box and marble_basket) making sure 1 agent is attentive and other agent is not
	// Check agents beliefs and confirm the attention atoms work
	
	// Sally is paying attention to whether marble in basket and whether marble in table
	// Anne is paying attention to whether marble in box and whether marble in table

	/* UNCOMMENT THIS FOR ALL THE TESTS
	auto s0 = d.add_initial_state({sally_attention_marble_in_basket,anne_attention_marble_in_box});
	
	// Formulas.
	formula f;

		// ** Attentiveness Test **
	// Attentiveness states that if an agent pays attention to a specific atom,
	// then she learns the literal in the announcement
 	// corresponding to it and that she was paying attention to it

	std::cout << "\n----------------------------------------------------------------\n" ;
	std::cout << "\n---------------------- ATTENTIVENESS TEST ----------------------\n" ;
	std::cout << "\n----------------------------------------------------------------\n" ;

	// Marble in table?
	auto f_marble_in_table = f.new_prop(marble_in_table);
	std::cout << f.to_string(d, f_marble_in_table) << " = " << (d.evaluate_formula(s0, f, f_marble_in_table) ? "true" : "false") << ".\n";
	
	// Sally is paying attention to marble in table?
	auto f_sally_attention_marble_in_table = f.new_prop(sally_attention_marble_in_table);
	std::cout << f.to_string(d, f_sally_attention_marble_in_table) << " = " << (d.evaluate_formula(s0, f, f_sally_attention_marble_in_table) ? "true" : "false") << ".\n";
	
	// Anne is paying attention to marble in table?
	auto f_anne_attention_marble_in_table = f.new_prop(anne_attention_marble_in_table);
	std::cout << f.to_string(d, f_anne_attention_marble_in_table) << " = " << (d.evaluate_formula(s0, f, f_anne_attention_marble_in_table) ? "true" : "false") << ".\n";
	
	// Sally believes marble in table?
	auto f_sally_believes_table = f.new_believes(sally, f_marble_in_table);
	std::cout << f.to_string(d, f_sally_believes_table) << " = " << (d.evaluate_formula(s0, f, f_sally_believes_table) ? "true" : "false") << ".\n";

	// Anne believes marble in table?
	auto f_anne_believes_table = f.new_believes(anne, f_marble_in_table);
	std::cout << f.to_string(d, f_anne_believes_table) << " = " << (d.evaluate_formula(s0, f, f_anne_believes_table) ? "true" : "false") << ".\n";

	// Anne put marble in table.
	auto[a1, s1] = d.perform_do(anne, {marble_in_table}, {});
	std::cout << "*********************************************\n" ;
	std::cout << "**** ACTION 1:  Anne put marble in table ****\n" ;
	std::cout << "*********************************************\n" ;

	// Marble in table?
	std::cout << f.to_string(d, f_marble_in_table) << " = " << (d.evaluate_formula(s1, f, f_marble_in_table) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_attention_marble_in_table) << " = " << (d.evaluate_formula(s1, f, f_sally_attention_marble_in_table) ? "true" : "false") << ".\n";
	
	// Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_attention_marble_in_table) << " = " << (d.evaluate_formula(s1, f, f_anne_attention_marble_in_table) ? "true" : "false") << ".\n";
	
	// Sally believes marble in table?
	std::cout << f.to_string(d, f_sally_believes_table) << " = " << (d.evaluate_formula(s1, f, f_sally_believes_table) ? "true" : "false") << ".\n";

	// Anne believes marble in table?
	std::cout << f.to_string(d, f_anne_believes_table) << " = " << (d.evaluate_formula(s1, f, f_anne_believes_table) ? "true" : "false") << ".\n";

	std::cout << "\n-----------------------------------------------------------------------\n" ;
	std::cout << "\n---------------------- INERTIA AND ATTENTIVENESS ----------------------\n" ;
	std::cout << "\n-----------------------------------------------------------------------\n" ;
	
	
	// Marble in basket?
	auto f_marble_in_basket = f.new_prop(marble_in_basket);
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s1, f, f_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally is paying attention to marble in basket?
	auto f_sally_attention_marble_in_basket = f.new_prop(sally_attention_marble_in_basket);
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s1, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Anne is paying attention to marble in basket?
	auto f_anne_attention_marble_in_basket = f.new_prop(anne_attention_marble_in_basket);
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s1, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally believes marble in basket?
	auto f_sally_believes_basket = f.new_believes(sally, f_marble_in_basket);
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s1, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in basket?
	auto f_anne_believes_basket = f.new_believes(anne, f_marble_in_basket);
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s1, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	// Sally put marble in basket.
	auto[a2, s2] = d.perform_do(sally, {marble_in_basket}, {});
	std::cout << "***********************************************\n" ;
	std::cout << "**** ACTION 2:  Sally put marble in basket ****\n" ;
	std::cout << "***********************************************\n" ;
	// Marble in table?
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_marble_in_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally believes marble in table?
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in table?
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	std::cout << "\n-------------------------------------------------------------------------------------------\n" ;
	std::cout << "\n----------------------  False Beliefs About Their Own Attention ----------------------\n" ;
	std::cout << "\n-------------------------------------------------------------------------------------------\n" ;

	// Sally is paying attention to marble in box?
	auto f_sally_attention_marble_in_box = f.new_prop(sally_attention_marble_in_box);

	// Anne is paying attention to marble in box?
	auto f_anne_attention_marble_in_box = f.new_prop(anne_attention_marble_in_box);

	std::cout << "***********************\n" ;
	std::cout << "**** Sally ****\n" ;
	std::cout << "***********************\n" ;

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally bellieves is paying attention to marble in basket?
	auto f_sally_believes_paying_attention_basket = f.new_believes(sally, f_sally_attention_marble_in_basket);
	std::cout << f.to_string(d, f_sally_believes_paying_attention_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_paying_attention_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in box?
	std::cout << f.to_string(d, f_sally_attention_marble_in_box) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Sally believes is paying attention to marble in box?
	auto f_sally_believes_paying_attention_box = f.new_believes(sally, f_sally_attention_marble_in_box);
	std::cout << f.to_string(d, f_sally_believes_paying_attention_box) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_paying_attention_box) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_attention_marble_in_table) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Sally believes is paying attention to marble in table?
	auto f_sally_believes_paying_attention_table = f.new_believes(sally, f_sally_attention_marble_in_table);
	std::cout << f.to_string(d, f_sally_believes_paying_attention_table) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_paying_attention_table) ? "true" : "false") << ".\n";

	std::cout << "***********************\n" ;
	std::cout << "**** Anne ****\n" ;
	std::cout << "***********************\n" ;

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne bellieves is paying attention to marble in basket?
	auto f_anne_believes_paying_attention_basket = f.new_believes(anne, f_anne_attention_marble_in_basket);
	std::cout << f.to_string(d, f_anne_believes_paying_attention_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_paying_attention_basket) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in box?
	std::cout << f.to_string(d, f_anne_attention_marble_in_box) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Anne believes is paying attention to marble in box?
	auto f_anne_believes_paying_attention_box = f.new_believes(anne, f_anne_attention_marble_in_box);
	std::cout << f.to_string(d, f_anne_believes_paying_attention_box) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_paying_attention_box) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_attention_marble_in_table) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Anne believes is paying attention to marble in table?
	auto f_anne_believes_paying_attention_table = f.new_believes(anne, f_anne_attention_marble_in_table);
	std::cout << f.to_string(d, f_anne_believes_paying_attention_table) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_paying_attention_table) ? "true" : "false") << ".\n";
	
	std::cout << "\n----------------------------------------------------------------------------------------------\n" ;
	std::cout << "\n----------------------  False Beliefs About Other Agents Attention ----------------------\n" ;
	std::cout << "\n----------------------------------------------------------------------------------------------\n" ;

	std::cout << "***********************\n" ;
	std::cout << "**** Sally ****\n" ;
	std::cout << "***********************\n" ;

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally bellieves Anne is paying attention to marble in basket?
	auto f_sally_believes_anne_paying_attention_basket = f.new_believes(sally, f_anne_attention_marble_in_basket);
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_anne_paying_attention_basket) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in box?
	std::cout << f.to_string(d, f_anne_attention_marble_in_box) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Sally believes Anne is paying attention to marble in box?
	auto f_sally_believes_anne_paying_attention_box = f.new_believes(sally, f_anne_attention_marble_in_box);
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_box) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_anne_paying_attention_box) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_attention_marble_in_table) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Sally believes Anne is paying attention to marble in table?
	auto f_sally_believes_anne_paying_attention_table = f.new_believes(sally, f_anne_attention_marble_in_table);
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_table) << " = " << (d.evaluate_formula(s2, f, f_sally_believes_anne_paying_attention_table) ? "true" : "false") << ".\n";

	std::cout << "***********************\n" ;
	std::cout << "**** Anne ****\n" ;
	std::cout << "***********************\n" ;

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne bellieves Sally is paying attention to marble in basket?
	auto f_anne_believes_sally_paying_attention_basket = f.new_believes(anne, f_sally_attention_marble_in_basket);
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_sally_paying_attention_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in box?
	std::cout << f.to_string(d, f_sally_attention_marble_in_box) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Anne believes Sally is paying attention to marble in box?
	auto f_anne_believes_sally_paying_attention_box = f.new_believes(anne, f_sally_attention_marble_in_box);
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_box) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_sally_paying_attention_box) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_attention_marble_in_table) << " = " << (d.evaluate_formula(s2, f, f_sally_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Anne believes Sally is paying attention to marble in table?
	auto f_anne_believes_sally_paying_attention_table = f.new_believes(anne, f_sally_attention_marble_in_table);
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_table) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_sally_paying_attention_table) ? "true" : "false") << ".\n";

	std::cout << "\n----------------------------------------------------------------------------\n" ;
	std::cout << "\n---------------------- TOP DOWN ATTENTION CHANGE ----------------------.----\n" ;
	std::cout << "\n----------------------------------------------------------------------------\n" ;

	// Marble in basket?
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in basket?
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s2, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	// Anne shifts top-down attention to basket
	auto[a3, s3] = d.perform_conscious_top_down(anne, {marble_in_basket}, {});
	std::cout << "*************************************************************\n" ;
	std::cout << "**** ACTION 3:  Anne shifts Top-Down Attention to basket ****\n" ;
	std::cout << "*************************************************************\n" ;

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s3, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in basket?
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	std::cout << "-------------------------------- // --------------------------------\n" ;

	// Marble in basket?
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s3, f, f_marble_in_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s3, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Sally believes marble in basket?
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Sally shifts top-down attention from basket
	auto[a4, s4] = d.perform_conscious_top_down(sally, {}, {marble_in_basket});
	std::cout << "****************************************************************\n" ;
	std::cout << "**** ACTION 4:  Sally shifts Top-Down Attention from basket ****\n" ;
	std::cout << "****************************************************************\n" ;

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s4, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Sally believes marble in basket?
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s4, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Sally bellieves is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_believes_paying_attention_basket) << " = " << (d.evaluate_formula(s4, f, f_sally_believes_paying_attention_basket) ? "true" : "false") << ".\n";

	// Anne bellieves Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_basket) << " = " << (d.evaluate_formula(s4, f, f_anne_believes_sally_paying_attention_basket) ? "true" : "false") << ".\n";

	std::cout << "-------------------------------- // --------------------------------\n" ;

	// Marble in basket?
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s3, f, f_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s4, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in basket?
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s4, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s4, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Sally believes marble in basket?
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s4, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Sally put marble in basket.
	auto[a5, s5] = d.perform_do(anne, {}, {marble_in_basket});
	std::cout << "**************************************************\n" ;
	std::cout << "**** ACTION 5:  Anne removes marble in basket ****\n" ;
	std::cout << "**************************************************\n" ;

	// Anne believes marble in basket?
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s5, f, f_anne_believes_basket) ? "true" : "false") << ".\n";

	// Sally believes marble in basket?
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s5, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

		std::cout << "\n----------------------------------------------------------------------------------------------\n" ;
	std::cout << "\n----------------------  False Beliefs About Other Agents Attention ----------------------\n" ;
	std::cout << "\n----------------------------------------------------------------------------------------------\n" ;

	std::cout << "***********************\n" ;
	std::cout << "**** Sally ****\n" ;
	std::cout << "***********************\n" ;

	// Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s5, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";
	
	// Sally bellieves Anne is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_basket) << " = " << (d.evaluate_formula(s5, f, f_sally_believes_anne_paying_attention_basket) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in box?
	std::cout << f.to_string(d, f_anne_attention_marble_in_box) << " = " << (d.evaluate_formula(s5, f, f_anne_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Sally believes Anne is paying attention to marble in box?
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_box) << " = " << (d.evaluate_formula(s5, f, f_sally_believes_anne_paying_attention_box) ? "true" : "false") << ".\n";

	// Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_attention_marble_in_table) << " = " << (d.evaluate_formula(s5, f, f_anne_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Sally believes Anne is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_believes_anne_paying_attention_table) << " = " << (d.evaluate_formula(s5, f, f_sally_believes_anne_paying_attention_table) ? "true" : "false") << ".\n";

	std::cout << "***********************\n" ;
	std::cout << "**** Anne ****\n" ;
	std::cout << "***********************\n" ;

	// Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s5, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	// Anne bellieves Sally is paying attention to marble in basket?
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_basket) << " = " << (d.evaluate_formula(s5, f, f_anne_believes_sally_paying_attention_basket) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in box?
	std::cout << f.to_string(d, f_sally_attention_marble_in_box) << " = " << (d.evaluate_formula(s5, f, f_sally_attention_marble_in_box) ? "true" : "false") << ".\n";

	// Anne believes Sally is paying attention to marble in box?
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_box) << " = " << (d.evaluate_formula(s5, f, f_anne_believes_sally_paying_attention_box) ? "true" : "false") << ".\n";

	// Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_sally_attention_marble_in_table) << " = " << (d.evaluate_formula(s5, f, f_sally_attention_marble_in_table) ? "true" : "false") << ".\n";

	// Anne believes Sally is paying attention to marble in table?
	std::cout << f.to_string(d, f_anne_believes_sally_paying_attention_table) << " = " << (d.evaluate_formula(, f, f_anne_believes_sally_paying_attention_table) ? "true" : "false") << ".\n";
	std::cout << "\n----------------------------------------------------------------------------\n" ;
	std::cout << "\n---------------------- BOTTOM UP ATTENTION CHANGE --------------------------\n" ;
	std::cout << "\n----------------------------------------------------------------------------\n" ;

	// Implementation Missing
	
	std::cout << "\n-----------------------------------------------------------------\n" ;
	std::cout << "\n---------------------- DEFAULT VALUES ---------------------------\n" ;
	std::cout << "\n-----------------------------------------------------------------\n" ;

	// Implementation Missing

	*/
	std::cout << "********************Initial State starting*************************\n" ;

	//auto s0 = d.add_initial_state({marble_in_table,marble_in_box, marble_in_basket, sally_attention_marble_in_basket,anne_attention_marble_in_box,anne_attention_marble_in_basket});
	auto s0 = d.add_initial_state({marble_in_box, marble_in_basket,sally_attention_marble_in_box,sally_attention_marble_in_table,anne_attention_marble_in_basket,anne_attention_marble_in_box});

	d.print_state_overview(d.get_state(s0), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket, sally_attention_marble_in_table,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box,anne_attention_marble_in_table  } );

	// Sally shifts top-down attention to basket
	auto[a1, s1] = d.perform_conscious_top_down(sally, {marble_in_basket}, {marble_in_box});
	//auto[a1, s1] = d.perform_minimal_bottom_up({sally}, {marble_in_basket}, {});

	//auto[a1, s1] = d.perform_minimal_bottom_up({sally}, {}, {marble_in_basket});

	std::cout << "****************************************************************\n" ;
	std::cout << "**** ACTION 1:  Sally shifts Top-Down Attention to basket ****\n" ;
	std::cout << "****************************************************************\n" ;

	std::cout<<"\n---------------------- State 1 ----------------------\n";
	d.print_state_overview(d.get_state(s1), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket, sally_attention_marble_in_table,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box,anne_attention_marble_in_table  } );

	// Anne put marble in table.
	auto[a2, s2] = d.perform_do(anne, {marble_in_table}, {marble_in_basket});
	std::cout << "*********************************************\n" ;
	std::cout << "**** ACTION 2:  Anne takes marble from basket ****\n" ;
	std::cout << "*********************************************\n" ;
	std::cout<<"\n---------------------- State 2 ----------------------\n";
	d.print_state_overview(d.get_state(s2), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket, sally_attention_marble_in_table,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box,anne_attention_marble_in_table  } );
	
	// Anne shifts top-down attention from box
	//auto[a3, s3] = d.perform_conscious_top_down(anne, {}, {});
	auto[a3, s3] = d.perform_minimal_bottom_up({anne}, {marble_in_table}, {});

	std::cout << "****************************************************************\n" ;
	std::cout << "**** ACTION 3:  Anne shifts Bottom-up Attention from table ****\n" ;
	std::cout << "****************************************************************\n" ;
	
	std::cout<<"\n---------------------- State 3 ----------------------\n";
	d.print_state_overview(d.get_state(s3), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket, sally_attention_marble_in_table,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box,anne_attention_marble_in_table  } );
	

	// Anne put marble in table.
	/*auto[a1, s1] = d.perform_do(anne, {marble_in_box}, {});
	std::cout << "*********************************************\n" ;
	std::cout << "**** ACTION 1:  Anne put marble in box ****\n" ;
	std::cout << "*********************************************\n" ;

	std::cout<<"\n---------------------- State 1 ----------------------\n";
	d.print_state_overview(d.get_state(s1), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box  } );

		// Sally put marble in basket.
	auto[a2, s2] = d.perform_do(sally, {marble_in_basket}, {});
	std::cout << "*********************************************\n" ;
	std::cout << "**** ACTION 2:  Sally put marble in basket ****\n" ;
	std::cout << "*********************************************\n" ;

	
	std::cout<<"\n---------------------- State 2 ----------------------\n";
	d.print_state_overview(d.get_state(s2), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box  } );


	// Sally shifts top-down attention from basket
	//auto[a3, s3] = d.perform_conscious_top_down(sally, {}, {marble_in_basket});
	auto[a3, s3] = d.perform_minimal_bottom_up({sally}, {}, {marble_in_basket});
	std::cout << "****************************************************************\n" ;
	std::cout << "**** ACTION 3:  Sally shifts Bottom-Up Attention from basket ****\n" ;
	std::cout << "****************************************************************\n" ;

	std::cout<<"\n---------------------- State 3 ----------------------\n";
	d.print_state_overview(d.get_state(s3), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box  } );
	
	// Sally put marble in basket.
	auto[a4, s4] = d.perform_do(anne, {}, {marble_in_basket});
	std::cout << "*********************************************\n" ;
	std::cout << "**** ACTION 4:  Anne removes marble from basket ****\n" ;
	std::cout << "*********************************************\n" ;

	
	std::cout<<"\n---------------------- State 4 ----------------------\n";
	d.print_state_overview(d.get_state(s4), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box  } );
	

	// Sally put marble in basket.
	auto[a5, s5] = d.perform_minimal_bottom_up({sally}, {marble_in_table,marble_in_basket}, {});
	std::cout << "**************************************************\n" ;
	std::cout << "**** ACTION 5:  Sally shift Bottom Up Attention to marble in table ****\n" ;
	std::cout << "**************************************************\n" ;

	std::cout<<"\n---------------------- State 5 ----------------------\n";
	d.print_state_overview(d.get_state(s5), { marble_in_basket,marble_in_box, marble_in_table, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box ,sally_attention_marble_in_table,anne_attention_marble_in_table } );
*/
	//Attentiveness tests:
	//confirm that agent learns p truth value after sand event on p, and also learn that he is attentive to p.

	//Inertia tests:
	//confirm that agent who pays no attention to p, does not learn p truth value after an event on p.

	//Agents may falsely believe to be paying attention tests:
	//confirm that all the agents believe they are attentive to every proposition

	//Top-Down Attention properties tests:
	//confirm that the agent is the only one who is aware of the attention change. (best to test with stop paying attention to something)

	//std::cout<<"\n---------------------- State 2 ----------------------\n";
	//d.print_state_overview(d.get_state(s2), {marble_in_basket, marble_in_box, sally_attention_marble_in_basket,sally_attention_marble_in_box, anne_attention_marble_in_basket,anne_attention_marble_in_box } );

	/*
	//s1 overview
	std::cout<<"\n---------------------- State 1 ----------------------\n";
	d.print_state_overview(d.get_state(s1), { marble_in_basket,marble_in_box, marble_in_table  } );

	//s2 overview
	std::cout<<"\n---------------------- State 2 ----------------------\n";
	d.print_state_overview(d.get_state(s2), { marble_in_basket,marble_in_box, marble_in_table  } );
	*/

	/*
	// --------------------------- REALITY ---------------------------
	std::cout << " --------------------------- REALITY --------------------------- \n" ;
	// Marble in box?
	auto f_marble_in_box = f.new_prop(marble_in_box);
	std::cout << f.to_string(d, f_marble_in_box) << " = " << (d.evaluate_formula(s0, f, f_marble_in_box) ? "true" : "false") << ".\n";

	// Marble in basket?
	auto f_marble_in_basket = f.new_prop(marble_in_basket);
	std::cout << f.to_string(d, f_marble_in_basket) << " = " << (d.evaluate_formula(s0, f, f_marble_in_basket) ? "true" : "false") << ".\n";

	// Marble in table?
	auto f_marble_in_table = f.new_prop(marble_in_table);
	std::cout << f.to_string(d, f_marble_in_table) << " = " << (d.evaluate_formula(s0, f, f_marble_in_table) ? "true" : "false") << ".\n";

	auto f_sally_attention_marble_in_box = f.new_prop(sally_attention_marble_in_box);
	std::cout << f.to_string(d, f_sally_attention_marble_in_box) << " = " << (d.evaluate_formula(s0, f, f_sally_attention_marble_in_box) ? "true" : "false") << ".\n";

	auto f_sally_attention_marble_in_basket = f.new_prop(sally_attention_marble_in_basket);
	std::cout << f.to_string(d, f_sally_attention_marble_in_basket) << " = " << (d.evaluate_formula(s0, f, f_sally_attention_marble_in_basket) ? "true" : "false") << ".\n";

	auto f_anne_attention_marble_in_box = f.new_prop(anne_attention_marble_in_box);
	std::cout << f.to_string(d, f_anne_attention_marble_in_box) << " = " << (d.evaluate_formula(s0, f, f_anne_attention_marble_in_box) ? "true" : "false") << ".\n";

	auto f_anne_attention_marble_in_basket = f.new_prop(anne_attention_marble_in_basket);
	std::cout << f.to_string(d, f_anne_attention_marble_in_basket) << " = " << (d.evaluate_formula(s0, f, f_anne_attention_marble_in_basket) ? "true" : "false") << ".\n";

	
	// --------------------------- 1st ORDER BELIEFS ---------------------------
	std::cout << "\n--------------------------- 1st ORDER BELIEFS --------------------------- \n" ;
	// Sally believes marble in box?
	auto f_sally_believes_box = f.new_believes(sally, f_marble_in_box);
	std::cout << f.to_string(d, f_sally_believes_box) << " = " << (d.evaluate_formula(s0, f, f_sally_believes_box) ? "true" : "false") << ".\n";

	// Sally believes marble in basket?
	auto f_sally_believes_basket = f.new_believes(sally, f_marble_in_basket);
	std::cout << f.to_string(d, f_sally_believes_basket) << " = " << (d.evaluate_formula(s0, f, f_sally_believes_basket) ? "true" : "false") << ".\n";

	// Anne believes marble in box?
	auto f_anne_believes_box = f.new_believes(anne, f_marble_in_box);
	std::cout << f.to_string(d, f_anne_believes_box) << " = " << (d.evaluate_formula(s0, f, f_anne_believes_box) ? "true" : "false") << ".\n";

	// Anne believes marble in basket?
	auto f_anne_believes_basket = f.new_believes(anne, f_marble_in_basket);
	std::cout << f.to_string(d, f_anne_believes_basket) << " = " << (d.evaluate_formula(s0, f, f_anne_believes_basket) ? "true" : "false") << ".\n";
	*/
	/*
	// --------------------------- 2nd ORDER BELIEFS ---------------------------
	std::cout << " --------------------------- 2nd ORDER BELIEFS --------------------------- \n" ;

	// Anne believes Sally believes marble in box?
	auto f_anne_believes_sally_believes_box = f.new_believes(anne, f_sally_believes_box);
	std::cout << f.to_string(d, f_anne_believes_sally_believes_box) << " = " << (d.evaluate_formula(s3, f, f_anne_believes_sally_believes_box) ? "true" : "false") << ".\n";

	// Anne believes Sally believes marble in basket?
	auto f_anne_believes_sally_believes_basket = f.new_believes(anne, f_sally_believes_basket);
	std::cout << f.to_string(d, f_anne_believes_sally_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_anne_believes_sally_believes_basket) ? "true" : "false") << ".\n";

	// Sally believes Anne believes marble in box?
	auto f_sally_believes_anne_believes_box = f.new_believes(sally, f_anne_believes_box);
	std::cout << f.to_string(d, f_sally_believes_anne_believes_box) << " = " << (d.evaluate_formula(s3, f, f_sally_believes_anne_believes_box) ? "true" : "false") << ".\n";

	// Sally believes Anne believes marble in basket?
	auto f_sally_believes_anne_believes_basket = f.new_believes(sally, f_anne_believes_basket);
	std::cout << f.to_string(d, f_sally_believes_anne_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_sally_believes_anne_believes_basket) ? "true" : "false") << ".\n";

	// Sally believes Sally believes marble in box?
	auto f_sally_believes_sally_believes_basket = f.new_believes(sally, f_sally_believes_basket);
	std::cout << f.to_string(d, f_sally_believes_sally_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_sally_believes_sally_believes_basket) ? "true" : "false") << ".\n";

	// Sally believes Anne believes marble in basket?
	auto f_anne_believes_anne_believes_box = f.new_believes(anne, f_anne_believes_box);
	std::cout << f.to_string(d, f_anne_believes_anne_believes_box) << " = " << (d.evaluate_formula(s3, f, f_anne_believes_anne_believes_box) ? "true" : "false") << ".\n";
	
		// --------------------------- EVERYONE BELIEFS ---------------------------
	std::cout << " --------------------------- EVERYONE BELIEFS --------------------------- \n" ;

	// Everyone^1 believes marble in box?
	auto f_everyone_believes_box = f.new_everyone_believes({ anne, sally }, 1, f_marble_in_box);
	std::cout << f.to_string(d, f_everyone_believes_box) << " = " << (d.evaluate_formula(s3, f, f_everyone_believes_box) ? "true" : "false") << ".\n";

	// Everyone^1 believes marble in basket?
	auto f_everyone_believes_basket = f.new_everyone_believes({ anne, sally }, 1, f_marble_in_basket);
	std::cout << f.to_string(d, f_everyone_believes_basket) << " = " << (d.evaluate_formula(s3, f, f_everyone_believes_basket) ? "true" : "false") << ".\n";

	// Everyone^1 believes marble in table?
	auto f_everyone_believes_table = f.new_everyone_believes({ anne, sally }, 1, f_marble_in_table);
	std::cout << f.to_string(d, f_everyone_believes_table) << " = " << (d.evaluate_formula(s3, f, f_everyone_believes_table) ? "true" : "false") << ".\n";
	
	/*
	// Everyone^2 believes marble in box?
	auto f_everyone_believes2_box = f.new_everyone_believes({ anne, sally }, 2, f_marble_in_box);
	std::cout << f.to_string(d, f_everyone_believes2_box) << " = " << (d.evaluate_formula(s4, f, f_everyone_believes2_box) ? "true" : "false") << ".\n";

	// Everyone^2 believes marble in basket?
	auto f_everyone_believes2_basket = f.new_everyone_believes({ anne, sally }, 2, f_marble_in_basket);
	std::cout << f.to_string(d, f_everyone_believes2_basket) << " = " << (d.evaluate_formula(s4, f, f_everyone_believes2_basket) ? "true" : "false") << ".\n";

	// Everyone^1 believes Anne believes marble in box? 
	auto f_everyone_believes_anne_believes_box = f.new_everyone_believes({ anne, sally }, 1, f_anne_believes_box);
	std::cout << f.to_string(d, f_everyone_believes_anne_believes_box) << " = " << (d.evaluate_formula(s4, f, f_everyone_believes_anne_believes_box) ? "true" : "false") << ".\n";

	// Everyone^2 believes Anne believes marble in box?
	auto f_everyone_believes2_anne_believes_box = f.new_everyone_believes({ anne, sally }, 2, f_anne_believes_box);
	std::cout << f.to_string(d, f_everyone_believes2_anne_believes_box) << " = " << (d.evaluate_formula(s4, f, f_everyone_believes2_anne_believes_box) ? "true" : "false") << ".\n";
	*/

	// The table was moved to the backyard
	
	/*auto[a4, s4] = d.perform_ac({}, { {sally,marble_in_table}, {anne,marble_in_table} });

	std::cout<<"\n---------------------- State 4 ----------------------\n";
	d.print_state_overview(d.get_state(s4), { sally_attention_marble_in_table, anne_attention_marble_in_table, sally_attention_marble_in_basket, anne_attention_marble_in_box, marble_in_basket, marble_in_box, marble_in_table  } );
	*/
	// Done.

	std::cout << "\n";
	std::cout << "Goodbye world.\n";

#if _DEBUG && _MSC_VER
	std::cout << "\n";
	std::system("PAUSE");
#endif

	return 0;
}
