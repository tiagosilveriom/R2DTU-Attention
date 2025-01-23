#include <reasoning/EpistemicReasoning.h>
#include <unordered_set>
#include <common/Message.h>
#include <msgs/NaoMsgs.h>
#include <behaviour/Behaviour.h>
#include <State.hpp>
#include <World.hpp>
#include <string>

#include <l10n/FBT.h>
#include <Action_Library.hpp>
#include <Planner.hpp>

void EpistemicReasoning::reset() {
    agent_infos_.clear();
    container_infos_.clear();
    agents_.clear();
    storable_.clear();
    containers_.clear();
    all_objects_.clear();
    agents_.push_back("R2DTU");
    agent_infos_.insert({"R2DTU", Agent {0, Agent::State::Present}});
    setup_domain();
}

void EpistemicReasoning::setup_domain() {
    std::vector<std::string> propositions;
    std::vector<bool> propositions_default_value;
    //It is currently needed to add ALL possible object permutations as propositions
    //as the query system does not yet understand object types and therefore needs to try all combinations
    //std::cout << "Number of objects: " << all_objects_.size() << "\n";

    for (auto& fst : all_objects_) { 
        try {
            int fst_number = std::stoi(fst.substr(fst.find('-') + 1));

            // If fst marker >1000 (Container Object Type) continue
            if (fst_number > 1000) {
                //std::cout << "Fst is a container object. Skipping.\n";
                continue;
            }
        } catch (const std::exception& e) {
           // std::cout << "Invalid fst format: " << fst << ". Skipping.\n";
            continue;
        }

        for (auto& snd : all_objects_) {
            //std::cout << "Fst: " << fst << "\n";
            //std::cout << "Snd: " << snd << "\n";

            if (fst == snd) {
             //   std::cout << "Same prop cannot be inside itself. Skipping.\n";
                continue;
            }

            try {
                int snd_number = std::stoi(snd.substr(snd.find('-') + 1));

                // If snd marker <200 (Storable Object Type) continue
                if (snd_number < 200) {
              //      std::cout << "Snd is a storable object. Skipping.\n";
                    continue;
                }
            } catch (const std::exception& e) {
             //   std::cout << "Invalid snd format: " << snd << ". Skipping.\n";
                continue;
            }

            // Fst is Storable ObjectType and Snd is Container ObjectType
            propositions.push_back("in(" + fst + "," + snd + ")");
            propositions_default_value.push_back(false);
        }
    }

    d_ = std::make_unique<del::domain>(agents_, propositions,propositions_default_value);

	std::vector<del::proposition_id> initial_propositions;
    printf("** Before Initial propositions added **\n");

    /*
    for (auto& info : agent_infos_) {
        info.second.id = d_->get_agent_id(info.first);
        if (info.second.state == Agent::Away) continue;
        for (auto& other : agent_infos_) {
            if (info.first != other.first && other.second.state == Agent::Present) {
                initial_propositions.push_back(d_->get_sees_proposition_id(info.second.id, other.second.id));
            }
        }
    }
    */
    //TODO: Change attention propositions at initial state for dynamic alternative (gaze estimation)
    for (auto& info : agent_infos_) {
        info.second.id = d_->get_agent_id(info.first);
        if (info.second.state == Agent::Away) continue; 
        for(auto prop_name : propositions)
        {
            initial_propositions.push_back(d_->get_attention_proposition_id(info.second.id, d_->get_proposition_id(prop_name)) ); //every agent attentive to every proposition
        }

    }
    printf("** Initial propositions added **\n");
    latest_state_ = d_->add_initial_state(initial_propositions);
    printf("** Initial State added **\n");

    if(initial_propositions.size()>0) produce_views();
    printf("** Domain Setup completed **\n");
}

bool EpistemicReasoning::plan(std::string goal_object, bool knows_location) {

    return !knows_location;


    //Setup planning domain
    del::State initial_state(2);
    initial_state.create_worlds(3);

    std::string goal_proposition = "holding(" + goal_object + ")";
    std::string boxes[] = {"box1", "box2", "box3"};
    std::string propositions[3];
    for (int i = 0; i < 3; ++i) {
        propositions[i] = "in(" + goal_object + "," + boxes[i] + ")";
    }
    for (uint32_t i = 0; i < 3; ++i) {
        initial_state.add_true_propositions(del::World_Id{i}, {propositions[i]});

        auto o1 = (i + 1) % 3;
        auto o2 = (i + 2) % 3;
        initial_state.add_indistinguishability_relation(del::Agent_Id{0}, del::World_Id{i}, del::World_Id{i});
        initial_state.add_indistinguishability_relation(del::Agent_Id{1}, del::World_Id{i},del::World_Id{i});

        if (!knows_location) {
            initial_state.add_indistinguishability_relation(del::Agent_Id{1}, del::World_Id{i}, del::World_Id{o1});
            initial_state.add_indistinguishability_relation(del::Agent_Id{1}, del::World_Id{i}, del::World_Id{o2});
        }
    }
    initial_state.add_designated_world(del::World_Id { 0 });

    del::Action_Library planning_library(2);

    //Announce action
    for (int i = 0; i < 3; ++i) {
        del::Action action(del::Agent_Id{ 0 }, 2);
        del::Formula f;
        f.f_prop(propositions[i]);
        del::Action_Event event = del::Action_Event(del::Event_Id { 0 }, std::move(f), std::unordered_set<std::string>(), std::unordered_set<std::string>());
        action.add_event(event);
        action.add_designated_event(del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 0 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 1 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });

        planning_library.add_action(action);
    }

    //Pickup action
    for (int i = 0; i < 3; ++i) {
        del::Action action(del::Agent_Id{ 1 }, 2);
        del::Formula f;
        f.f_prop(propositions[i]);
        del::Action_Event event = del::Action_Event(del::Event_Id { 0 }, std::move(f), {goal_proposition}, {propositions[i]});
        action.add_event(event);
        action.add_designated_event(del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 0 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 1 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });

        planning_library.add_action(action);
    }

    //Perceive action
    for (int i = 0; i < 3; ++i) {
        del::Action action(del::Agent_Id{ 1 }, 2);
        del::Formula f;
        f.f_prop(propositions[i]);
        del::Action_Event event = del::Action_Event(del::Event_Id { 0 }, std::move(f), std::unordered_set<std::string>(), std::unordered_set<std::string>());
        action.add_event(event);
        action.add_designated_event(del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 0 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });
        action.add_indistinguishability_relation(del::Agent_Id{ 1 }, del::Event_Id{ 0 }, del::Event_Id{ 0 });

        planning_library.add_action(action);
    }

    del::Formula goal;
    goal.f_prop(goal_proposition);

    del::Planner planner;

    del::Policy policy = planner.find_policy(goal, planning_library, initial_state);

    del::Action action(del::Agent_Id {0},0);
    bool success;
    std::tie(action, success) = policy.get_action(initial_state);

    uint32_t owner = action.get_owner().id;

    printf("Solved %d %d %lu\n", policy.is_solved(), success, action.get_owner().id);
    printf("Action: %s", action.to_string().c_str());

    return owner == 0;
}

void EpistemicReasoning::tick() {
}

std::unique_ptr<Query> produce_nested_belief_predicate(std::vector<std::string>& belief_chain, std::string predicate, std::string& obj, size_t i) {
    //std::cout<<"EpistemicReasoning::produce_nested_belief_predicate: Started \n"; 

   if (i == belief_chain.size()) {
       return make_predicate(predicate, make_constant(obj), make_variable("x"));
   } else {
       return make_believes(belief_chain[i], produce_nested_belief_predicate(belief_chain, predicate, obj, i+1));
   }
}

void EpistemicReasoning::produce_view_msg(std::vector<std::string> belief_chain) {
    //printf("EpistemicReasoning::produce_view_msg: Starting \n");

    MessageBuilder<EpistemicStateViewMsg> msg;
    if (!blank_views_flag) {
        std::vector<std::pair<size_t, double>> container_order;
        //std::cout<<"EpistemicReasoning::produce_view_msg: Containers size "<< containers_.size() << "\n"; 

        container_order.reserve(containers_.size());
        for (auto i = 0u; i < containers_.size(); ++i) {
            container_order.push_back({i, container_infos_[containers_[i]].x});
        }

        std::sort(container_order.begin(), container_order.end(), [](std::pair<size_t, double> a, std::pair<size_t, double> b) {
            return a.second < b.second;
        });


        for (auto i = 0u; i < EpistemicStateViewMsg::MAX_CONTAINERS; ++i) {
            if (i < containers_.size()) {
                msg->containers[i] = containers_[container_order[i].first].back();
            } else {
                msg->containers[i] = '\0';
            }
        }
        //std::cout<<"EpistemicReasoning::produce_view_msg: Before nested belief predicate\n"; 
        //std::cout<<"EpistemicReasoning::produce_view_msg: Storable size "<< storable_.size() << "\n"; 

        for (auto i = 0u; i < EpistemicStateViewMsg::MAX_MOVABLES; ++i) {
            if (i < storable_.size()) {
                
                // The problem is here !  
                auto q = produce_nested_belief_predicate(belief_chain, "in", storable_[i], 0);
                //std::cout<<"EpistemicReasoning::produce_view_msg: Belief predicate "<< q << "\n"; 
                auto response = query(q);
                //std::cout<<"EpistemicReasoning::produce_view_msg: Belief predicate Response "<< response << "\n"; 

                msg->movables_location[i] = response.size() == 1 ? response[0].back() : ' ';
                std::string::size_type dashPos = storable_[i].find('-'); // Find position of '-'

                if (dashPos != std::string::npos) { // Check if '-' exists in the string
                    std::string prefix = storable_[i].substr(0, dashPos); // Extract substring until '-'
                    
                    if (prefix == "marker") {
                        msg->movables_id[i] = storable_[i].back(); // Store last ID digit
                    } else {
                        msg->movables_id[i] = storable_[i].front();
                    }
                } else {
                    // Handle the case where '-' is not found, if needed
                        msg->movables_id[i] = storable_[i].back(); // Store last ID digit
                        std::cout<<"FALHA A AVALIAR STORABLES ****\n";
                }

            } else {
                msg->movables_id[i] = '\0';
            }
        }

        //std::cout<<"EpistemicReasoning::produce_view_msg: After nested belief predicate\n"; 

    } else {
        msg->containers[0] = '\0';
        msg->movables_id[0] = '\0';
    }
    std::string name;
    for (auto i = 0u; i < belief_chain.size(); ++i) {
        if (i > 0) name += " -> ";
        name += belief_chain[i];
    }

    msg.write_string(&msg->name, name);

    view_pub_->publish(msg);
}

void EpistemicReasoning::produce_views() {
    //printf("EpistemicReasoning::produceviews: Starting \n");
    produce_view_msg({"R2DTU"});
    //printf("EpistemicReasoning::produceviews: R2DTU successfull \n");
    for (auto i = 1u; i < agents_.size(); ++i) {
        produce_view_msg({agents_[i]});
        //std::cout <<"EpistemicReasoning::produceviews:"<< agents_[i]<<" successfull \n";
        for (auto j = 1u; j < agents_.size(); ++j) { //to produce the perception of i about perceptio of j (or the opposite)
            if (i != j) {
                //produce_view_msg({agents_[i], agents_[j]});
            }
        }
    }
}

std::string translate_into_symbol_names(std::string name) {
    if (name.rfind("box", 0) == 0) {
        return "marker-100" + std::string(1, name.back()); // box + number
    } else if (name == "red") {
        return "marker-1";
    } else if (name == "green") {
        return "marker-2";
    } else if (name == "blue") {
        return "marker-3";
    } else {
        printf("Unknown obj: %s\n", name.c_str());
        return name;
    }
};

std::string translate_into_symbol_names_dk(std::string name) {
    if (name.rfind("kasse", 0) == 0) {
        return "marker-100" + std::string(1, name.back());
    } else if (name == "røde") {
        return "marker-1";
    } else if (name == "grønne") {
        return "marker-2";
    } else if (name == "blå") {
        return "marker-3";
    } else {
        printf("Unknown obj: %s\n", name.c_str());
        return name;
    }
};

static std::unordered_map<std::string, std::string> inverse_translate_map_en = {
        {"marker-1", "red cube"},
        {"marker-2", "green cube"},
        {"marker-3", "blue cube"},
        {"marker-1001", "box 1"},
        {"marker-1002", "box 2"},
        {"marker-1003", "box 3"},
        {"marker-1004", "box 4"},
};

static std::unordered_map<std::string, std::string> inverse_translate_map_dk = {
        {"marker-1", "røde klods"},
        {"marker-2", "grønne klods"},
        {"marker-3", "blå klods"},
        {"marker-1001", "kasse 1"},
        {"marker-1002", "kasse 2"},
        {"marker-1003", "kasse 3"},
        {"marker-1004", "kasse 4"},
};

std::string inverse_translate(std::string name, std::unordered_map<std::string, std::string> map) {

    auto search = map.find(name);
    if (search != map.end()) {
        return search->second;
    } else {
        return name;
    }
};

void EpistemicReasoning::log(LoggingMsg::Level level, std::string source, std::string text) {
	MessageBuilder<LoggingMsg> msg;
        msg->level = level;
        msg.write_string(&msg->source, source);
        msg.write_string(&msg->text, text);
        time(&msg->timestamp);
        printf("%s: %s\n", source.c_str(), text.c_str());
        logging_pub_->publish(msg);
}

#if LANG_DANISH
const std::string colours[] = {u8"røde", u8"grønne", u8"blå"};
#else
const std::string colours[] = {"red", "green", "blue"};
#endif

std::string EpistemicReasoning::answer_question(std::string question) {

    try {

	printf("Q: %s\n", question.c_str());

        std::string object_colour = "";
        for (auto &col : colours) {
	    auto res = strstr(question.c_str(), col.c_str());
            if (res != nullptr) {
                object_colour = col;
                break;
            }
        }
        if (object_colour.empty()) {
            printf("Failed to understand colour!\n");
            return "";
        }
	    std::string object_name = LANG_DANISH ? translate_into_symbol_names_dk(object_colour) : translate_into_symbol_names(object_colour);
        size_t a1_pos = 0;
        size_t a2_pos = 0;

        if (agents_.size() > 1) { //R2DTU also belongs to agents_
            printf("Agent 1 name: %s\n", agents_[1].c_str());
            a1_pos = (size_t) strstr(question.c_str(), agents_[1].c_str());
        }
        if (agents_.size() > 2) {
            printf("Agent 2 name: %s\n", agents_[2].c_str());
            a2_pos = (size_t) strstr(question.c_str(), agents_[2].c_str());
        }

	    printf("%lu %lu\n", a1_pos, a2_pos);

        bool a1 = a1_pos != 0;
        bool a2 = a2_pos != 0;

        auto& inverse_translate_map = LANG_DANISH ? inverse_translate_map_dk : inverse_translate_map_en;

        if (a1 && a2) { //2nd order beliefs
            std::string name1 = (a1_pos < a2_pos) ? agents_[1] : agents_[2];
            std::string name2 = (a1_pos < a2_pos) ? agents_[2] : agents_[1];
            auto q = make_believes(name1, make_believes(name2, make_predicate("in", make_constant(
                    translate_into_symbol_names_dk(object_name)), make_variable("x"))));
            auto response = query(q);
            char buffer[255];
            if (response.size() == 1) {
                sprintf(buffer, L10N_SECOND_ORDER_BOX, name1.c_str(), name2.c_str(), inverse_translate(object_name, inverse_translate_map).c_str(), inverse_translate(response[0], inverse_translate_map).c_str());
                return buffer;
            } else {
                sprintf(buffer, L10N_SECOND_ORDER_TABLE, name1.c_str(), name2.c_str(), inverse_translate(object_name, inverse_translate_map).c_str());
                return buffer;
            }
        } else if (a1 || a2) { // 1st order beliefs
            auto name = (a1) ? agents_[1] : agents_[2];
            auto q = make_believes(name,
                                   make_predicate("in", make_constant(translate_into_symbol_names_dk(object_name)),
                                                  make_variable("x")));
            auto response = query(q);
            char buffer[255];
            if (response.size() == 1) {
                sprintf(buffer, L10N_FIRST_ORDER_BOX, name.c_str(), inverse_translate(object_name, inverse_translate_map).c_str(), inverse_translate(response[0], inverse_translate_map).c_str());
                return buffer;
            } else {
                sprintf(buffer, L10N_FIRST_ORDER_TABLE, name.c_str(), inverse_translate(object_name, inverse_translate_map).c_str());
                return buffer;
            }
        } else { // 0th order beliefs
            auto q = make_predicate("in", make_constant(object_name), make_variable("x"));
            auto response = query(q);
            char buffer[255];
            if (response.size() == 1) {
                sprintf(buffer, L10N_ZEROTH_ORDER_BOX, inverse_translate(object_name, inverse_translate_map).c_str(), inverse_translate(response[0], inverse_translate_map).c_str());
                return buffer;
            } else {
                sprintf(buffer, L10N_ZEROTH_ORDER_TABLE, inverse_translate(object_name, inverse_translate_map).c_str());
                return buffer;
            }
        }
    } catch (std::out_of_range& err) {
        printf("Misheard sentence\n");
    }
    return "";
}

std::unordered_set<std::string> free_variables(const std::unique_ptr<Query>& query) {
    if (query->type == Query::Type::Constant) return {};
    if (query->type == Query::Type::Variable) return {query->node.atom.name};
    if (query->type == Query::Type::Believes) return free_variables(query->node.belief.inner);
    if (query->type == Query::Type::Not) return free_variables(query->node.unary.inner);

    if (query->type == Query::Type::And || query->type == Query::Type::Or) {
        auto left = free_variables(query->node.binary.left);
        auto right = free_variables(query->node.binary.right);
        left.merge(right);
        return left;
    };

    if (query->type == Query::Type::Predicate) {

        auto result = std::unordered_set<std::string>();
        for (auto &inner : query->node.predicate.children) {
            auto inner_res = free_variables(inner);
            if (!inner_res.empty()) {
                result.merge(inner_res);
            }
        }

        return result;
    };
    ASSERT(false, "Invalid query tree structure");
}

std::string reify_predicate(const std::string& name, const std::vector<std::unique_ptr<Query>>& leafs, const std::unordered_map<std::string, std::string>& var_map) {
    std::string result = name + "(";
    //std::cout<<"EpistemicReasoning::reify_predicate: started\n"; 
    //std::cout<<"EpistemicReasoning::reify_predicate: predicate name "<<name<< "\n"; 
    //std::cout<<"EpistemicReasoning::reify_predicate: "<< leafs.size() <<"\n"; 

    for (auto i = 0u; i < leafs.size(); ++i) {
    //std::cout<<"EpistemicReasoning::reify_predicate: leafs iteration"<< i <<"\n"; 

        if (leafs[i]->type == Query::Type::Constant) {
            result += leafs[i]->node.atom.name;
        }
        if (leafs[i]->type == Query::Type::Variable) {
        //    std::cout<<"EpistemicReasoning::reify_predicate: Before var_map\n"; 

            result += var_map.at(leafs[i]->node.atom.name);

        //    std::cout<<"EpistemicReasoning::reify_predicate: After var_map\n"; 

        }
        if (i + 1 != leafs.size()) result += ",";
    }
    //std::cout<<"EpistemicReasoning::reify_predicate: leaving\n"; 
    return result + ")";
}

del::formula::node_id EpistemicReasoning::build_formula(const std::unique_ptr<Query>& query, const std::unordered_map<std::string, std::string>& var_map, del::formula& f) const {
    if (query->type == Query::Type::Believes) {
    //std::cout<<"EpistemicReasoning::build_formula: Believes\n"; 
        auto inner = build_formula(query->node.belief.inner, var_map, f); //2nd order: inner is Query Type Believes ; 1st order: inner is Query Type Predicate
    //    std::cout<<"EpistemicReasoning::build_formula: Believes "<< f.to_string(*d_,inner) <<"\n"; 
        return f.new_believes(d_->get_agent_id(query->node.belief.agent), inner);
    }
    if (query->type == Query::Type::Not) {
        auto inner = build_formula(query->node.unary.inner, var_map, f);
        return f.new_not(inner);
    }
    if (query->type == Query::Type::And) {
        auto left = build_formula(query->node.binary.left, var_map, f);
        auto right = build_formula(query->node.binary.right, var_map, f);
        return f.new_and({left, right});
    }
    if (query->type == Query::Type::Or) {
        auto left = build_formula(query->node.binary.left, var_map, f);
        auto right = build_formula(query->node.binary.right, var_map, f);
        return f.new_or({left, right});
    }
    if (query->type == Query::Type::Predicate) {
    //std::cout<<"EpistemicReasoning::build_formula: Predicate\n"; 
        auto pred = reify_predicate(query->node.predicate.name, query->node.predicate.children, var_map); //just to have the predicate as a string in the standard way : predicate (a,b,c,...)
    //    std::cout<<"EpistemicReasoning::build_formula: "<< pred <<"\n"; 
        try
        {
            return f.new_prop(d_->get_proposition_id(pred)); // The segmentation fault happens here, as it searches for a non-existing predicate
        }
        catch( const std::out_of_range)
        {
            return f.new_null();
        }
    }
    ASSERT(false, "Invalid query tree structure");
}

bool EpistemicReasoning::evaluate(const std::unique_ptr<Query>& query) const {
    std::unordered_map<std::string, std::string> var_map;

    del::formula f;
    auto root = build_formula(query, var_map, f);

    return d_->evaluate_formula(latest_state_, f, root);
}

std::vector<std::string> EpistemicReasoning::query(const std::unique_ptr<Query>& query) const {
    //std::cout<<"EpistemicReasoning::query: Query started\n"; 
    auto free_vars = free_variables(query);
    //std::cout<<"EpistemicReasoning::query: Query FREE VAR "<<free_vars.size()<<  "\n"; 

    ASSERTF(free_vars.size() == 1, "Currently we only support queries with exactly one variable and not %lu\n", free_vars.size());
    std::unordered_map<std::string, std::string> var_map;

    std::vector<std::string> results;

    for (const auto& obj : all_objects_) {

        var_map[*free_vars.begin()] = obj;

        del::formula f;
      //      std::cout<<"EpistemicReasoning::query: Before build_formula "<< obj <<"\n"; 

        auto root = build_formula(query, var_map, f);
        
        //std::cout<<"EpistemicReasoning::query: After build_formula "<< f.to_string(*d_,root) <<"\n"; 
        if(f.isNull(root)) continue;

        if (d_->evaluate_formula(latest_state_, f, root)) {
            results.push_back(obj);
        }
    }
        // std::cout<<"EpistemicReasoning::query: Query leaving\n"; 

    return results;
}

std::unique_ptr<Query> make_constant(std::string name) {
    auto q = std::make_unique<Query>(Query::Type::Constant);
    q->type = Query::Type::Constant;
    q->node.atom = AtomNode { std::move(name) };
    return q;
}

std::unique_ptr<Query> make_variable(std::string name) {
    auto q = std::make_unique<Query>(Query::Type::Variable);
    q->type = Query::Type::Variable;
    q->node.atom = AtomNode { name };
    return q;
}

std::unique_ptr<Query> make_predicate(std::string name, std::vector<std::unique_ptr<Query>> elements) {
    auto q = std::make_unique<Query>(Query::Type::Predicate);
    q->type = Query::Type::Predicate;
    q->node.predicate = PredicateNode { std::move(name), std::move(elements) };
    return q;
}

//Initializer list are ****** broken. No support for objects with move semantics
//So instead we just hardcode 4 different versions of the function, as predicates are only going to have 0-3 arity anyways

std::unique_ptr<Query> make_predicate(std::string name) {
    std::vector<std::unique_ptr<Query>> elements;
    return make_predicate(std::move(name), std::move(elements));
}

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst) {
    std::vector<std::unique_ptr<Query>> elements;
    elements.push_back(std::move(fst));
    return make_predicate(std::move(name), std::move(elements));
}

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst, std::unique_ptr<Query> snd) {
    std::vector<std::unique_ptr<Query>> elements;
    elements.push_back(std::move(fst));
    elements.push_back(std::move(snd));
    return make_predicate(std::move(name), std::move(elements));
}

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst, std::unique_ptr<Query> snd, std::unique_ptr<Query> thd) {
    std::vector<std::unique_ptr<Query>> elements;
    elements.push_back(std::move(fst));
    elements.push_back(std::move(snd));
    elements.push_back(std::move(thd));
    return make_predicate(std::move(name), std::move(elements));
}

std::unique_ptr<Query> make_believes(std::string agent, std::unique_ptr<Query> inner) {
    auto q = std::make_unique<Query>(Query::Type::Believes);
    q->type = Query::Type::Believes;
    q->node.belief = BeliefNode { std::move(agent), std::move(inner) };
    return q;
}

std::unique_ptr<Query> make_not(std::unique_ptr<Query> inner) {
    auto q = std::make_unique<Query>(Query::Type::Not);
    q->type = Query::Type::Not;
    q->node.unary = UnaryConnectiveNode { std::move(inner) };
    return q;
}

std::unique_ptr<Query> make_and(std::unique_ptr<Query> left, std::unique_ptr<Query> right) {
    auto q = std::make_unique<Query>(Query::Type::And);
    q->type = Query::Type::And;
    q->node.binary = BinaryConnectiveNode { std::move(left), std::move(right) };
    return q;
}

std::unique_ptr<Query> make_or(std::unique_ptr<Query> left, std::unique_ptr<Query> right) {
    auto q = std::make_unique<Query>(Query::Type::Or);
    q->type = Query::Type::Or;
    q->node.binary = BinaryConnectiveNode { std::move(left), std::move(right) };
    return q;
}
