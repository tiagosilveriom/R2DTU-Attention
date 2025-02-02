#include "pch.h"
#include "CppUnitTest.h"

#include <iostream>

#include "../Pepper_Planner/Types.hpp"
#include "../Pepper_Planner/DEL_Operations.hpp"
#include "../Pepper_Planner/Bisimulation_Context.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PepperPlannerTests
{
	using namespace del;
	TEST_CLASS(Bisimulation_Context_Tests)
	{
	public:

		TEST_METHOD(Test_Scenario_From_Gumenuks_Thesis_Positive) {
			State state1(1);
			state1.create_worlds(8);

			state1.add_true_propositions(World_Id{ 0 }, {   });
			state1.add_true_propositions(World_Id{ 1 }, {"q"});
			state1.add_true_propositions(World_Id{ 2 }, {"p"});
			state1.add_true_propositions(World_Id{ 3 }, {"p","q"});
			state1.add_true_propositions(World_Id{ 4 }, {   });
			state1.add_true_propositions(World_Id{ 5 }, {"q"});
			state1.add_true_propositions(World_Id{ 6 }, {"p"});
			state1.add_true_propositions(World_Id{ 7 }, {"p","q"});

			for (size_t i = 1; i < 8; i++) {
				state1.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ 0 }, World_Id{ i });
				state1.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ i }, World_Id{ i });
			}

			state1.add_designated_world(World_Id{ 0 });
			state1.add_designated_world(World_Id{ 3 });
			state1.add_designated_world(World_Id{ 4 });
			state1.add_designated_world(World_Id{ 7 });

			State state2(1);
			state2.create_worlds(5);
			state2.add_true_propositions(World_Id{ 0 }, {});
			state2.add_true_propositions(World_Id{ 1 }, {});
			state2.add_true_propositions(World_Id{ 2 }, {"q"});
			state2.add_true_propositions(World_Id{ 3 }, {"p"});
			state2.add_true_propositions(World_Id{ 4 }, {"p","q"});

			for (size_t i = 1; i < 5; i++) {
				state2.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ 0 }, World_Id{ i });
				state2.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ i }, World_Id{ i });
			}

			state2.add_designated_world(World_Id{ 0 });
			state2.add_designated_world(World_Id{ 1 });
			state2.add_designated_world(World_Id{ 4 });


			Bisimulation_Context context(state1, state2);
			Assert::IsTrue(context.is_bisimilar());

		}

		TEST_METHOD(Test_Scenario_From_Gumenuks_Thesis_Negative) {
			State state1(1);
			state1.create_worlds(8);

			state1.add_true_propositions(World_Id{ 0 }, {   });
			state1.add_true_propositions(World_Id{ 1 }, { "q" });
			state1.add_true_propositions(World_Id{ 2 }, { "p" });
			state1.add_true_propositions(World_Id{ 3 }, { "p","q" });
			state1.add_true_propositions(World_Id{ 4 }, {   });
			state1.add_true_propositions(World_Id{ 5 }, { "q" });
			state1.add_true_propositions(World_Id{ 6 }, { "p" });
			state1.add_true_propositions(World_Id{ 7 }, { "p","q" });

			for (size_t i = 1; i < 8; i++) {
				state1.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ 0 }, World_Id{ i });
				state1.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ i }, World_Id{ i });
			}

			state1.add_designated_world(World_Id{ 0 });
			state1.add_designated_world(World_Id{ 3 });
			state1.add_designated_world(World_Id{ 4 });
			state1.add_designated_world(World_Id{ 5 });
			state1.add_designated_world(World_Id{ 7 });

			State state2(1);
			state2.create_worlds(5);
			state2.add_true_propositions(World_Id{ 0 }, {});
			state2.add_true_propositions(World_Id{ 1 }, {});
			state2.add_true_propositions(World_Id{ 2 }, { "q" });
			state2.add_true_propositions(World_Id{ 3 }, { "p" });
			state2.add_true_propositions(World_Id{ 4 }, { "p","q" });

			for (size_t i = 1; i < 5; i++) {
				state2.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ 0 }, World_Id{ i });
				state2.add_indistinguishability_relation(Agent_Id{ 0 }, World_Id{ i }, World_Id{ i });
			}

			state2.add_designated_world(World_Id{ 0 });
			state2.add_designated_world(World_Id{ 1 });
			state2.add_designated_world(World_Id{ 4 });


			Bisimulation_Context context(state1, state2);
			Assert::IsFalse(context.is_bisimilar());

		}
	};
}