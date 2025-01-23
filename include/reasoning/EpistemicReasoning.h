#pragma once

#include <functional>
#include <del/domain.hpp>
#include <Domain.hpp>
#include <msgs/ReasoningMsgs.h>
#include <msgs/ActuationMsgs.h>
#include <msgs/ControlMsgs.h>

#include <core/Node.h>
#include <msgs/NaoMsgs.h>
#include <glm/vec3.hpp>

struct Query;

struct AtomNode {
    std::string name;
};

struct PredicateNode { //predicate_name(children)
    std::string name;
    std::vector<std::unique_ptr<Query>> children; //the arguments / variables 
};

struct BeliefNode { //B_agent(inner)
    std::string agent; 
    std::unique_ptr<Query> inner; 
};

struct UnaryConnectiveNode {
    std::unique_ptr<Query> inner;
};

struct BinaryConnectiveNode {
    std::unique_ptr<Query> left;
    std::unique_ptr<Query> right;
};

struct Query {
    enum class Type {
        Variable,
        Constant,
        Predicate,
        Believes,
        Not,
        And,
        Or,
    };
    Type type;

    explicit Query(Type type) : node(type) {}

    ~Query() {
        switch (type) {
            case Type::Variable: node.atom.~AtomNode(); break;
            case Type::Constant: node.atom.~AtomNode(); break;
            case Type::Predicate: node.predicate.~PredicateNode(); break;
            case Type::Believes: node.belief.~BeliefNode(); break;
            case Type::Not: node.unary.~UnaryConnectiveNode(); break;
            case Type::And: node.binary.~BinaryConnectiveNode(); break;
            case Type::Or: node.binary.~BinaryConnectiveNode(); break;
        }
    }

    union Node {
        AtomNode atom;
        PredicateNode predicate;
        BeliefNode belief;
        UnaryConnectiveNode unary;
        BinaryConnectiveNode binary;

        Node(Type type) {
            switch (type) {
                case Type::Variable: new(&atom) AtomNode(); break;
                case Type::Constant: new(&atom) AtomNode(); break;
                case Type::Predicate: new(&predicate) PredicateNode(); break;
                case Type::Believes: new(&belief) BeliefNode(); break;
                case Type::Not: new(&unary) UnaryConnectiveNode(); break;
                case Type::And: new(&binary) BinaryConnectiveNode(); break;
                case Type::Or: new(&binary) BinaryConnectiveNode(); break;
            }
        }

        ~Node() {}
    };

    Node node; //node stores the info required for the type of query
};

std::unique_ptr<Query> make_constant(std::string name);

std::unique_ptr<Query> make_variable(std::string name);

std::unique_ptr<Query> make_predicate(std::string name, std::vector<std::unique_ptr<Query>> elements);

//Initializer list as ****** broken. No support for objects with move semantics
//So we just hardcode this as predicates are only going to have 0-3 arity anyways

std::unique_ptr<Query> make_predicate(std::string name);

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst);

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst, std::unique_ptr<Query> snd);

std::unique_ptr<Query> make_predicate(std::string name, std::unique_ptr<Query> fst, std::unique_ptr<Query> snd, std::unique_ptr<Query> thd);

std::unique_ptr<Query> make_believes(std::string agent, std::unique_ptr<Query> inner);

std::unique_ptr<Query> make_not(std::unique_ptr<Query> inner);

std::unique_ptr<Query> make_and(std::unique_ptr<Query> left, std::unique_ptr<Query> right);

std::unique_ptr<Query> make_or(std::unique_ptr<Query> left, std::unique_ptr<Query> right);

enum class EpistemicObjectType {
    Storable,
    Container,
    Agent
};

class EpistemicReasoning {
public:

    explicit EpistemicReasoning(Node& node) : view_pub_(node.advertise<EpistemicStateViewMsg>("epistemic_view")),
    logging_pub_(node.advertise<LoggingMsg>("logging")) {
    }


    void add_object(EpistemicObjectType type, std::string& name) {
        switch (type) {
            case EpistemicObjectType::Storable:
                storable_.push_back(name);
                break;
            case EpistemicObjectType::Container:
                containers_.push_back(name);
                container_infos_.insert({name, Container {0}});
                break;
            case EpistemicObjectType::Agent:
                agents_.push_back(name);
                agent_infos_.insert({name, Agent {0, Agent::State::Present}});

 //               agent_infos_.insert({name, Agent {0, Agent::State::Away}});
                break;
            default:
                return;
        }
        all_objects_.push_back(name);
        setup_domain();
    }

    void reset();
    void setup_domain();

    void tick();

    void produce_view_msg(std::vector<std::string> belief_chain);
    void produce_views();

    std::vector<std::string> query(const std::unique_ptr<Query>& query) const;
    bool evaluate(const std::unique_ptr<Query>& query) const;

    std::vector<std::pair<del::agent_id, del::agent_id>> compute_observability_changes(
            const std::string& name, std::function<void(std::vector<std::pair<del::agent_id, del::agent_id>>&, del::agent_id, del::agent_id)> fn) {

        std::vector<std::pair<del::agent_id, del::agent_id>> changes;

        auto observer_id = agent_infos_.at(name).id;

        for (auto& [other_name, info] : agent_infos_) {
            if (other_name == name) continue;
            if (info.state == Agent::Present) {
                auto observed_id = info.id;
                fn(changes, observer_id, observed_id);
            }
        }
        return changes;
    }

    void agent_appear(const std::string& name) {
        printf("Agent appear %s\n", name.c_str());
	    log(LoggingMsg::Level::INFO, "Epistemic", "Agent appear " + name);
            
        /*
            auto changes = compute_observability_changes(name, [](auto& changes, auto observer, auto observed){
                changes.emplace_back(std::pair{observer, observed});
                changes.emplace_back(std::pair{observed, observer});
            });
        */
        //   auto result = d_->perform_oc(changes, {});

        // TODO: implement gaze estimation to reduce the attendable propositions
        //auto i = agent_infos_.at(name).id;
        //agent_infos_.at(name).state = Agent::Present;
        
        //auto changes = d_->get_domain_non_attention_propositions_id(); // This can only return non attentional propositions
        // Changes are the set of added proposition_id when agent i appears
        //auto result = d_->perform_ac_top_down_v2(i, changes, {});
        //auto result = d_->perform_ac_bottom_up_v1({i}, changes, {});
        //latest_state_ = result.second;

        //d_->print_state_overview(d_->get_state(latest_state_),changes);

        //d_->others_agents_belief_regarding_attention(latest_state_,i);
        printf("Agent appeared without attention changes !");

    }

    

    void blank_views() {
        blank_views_flag = !blank_views_flag;
        produce_views();
    }

    void agent_disappear(const std::string& name) {
	    log(LoggingMsg::Level::INFO, "Epistemic", "Agent disappear " + name);
        /*
        auto changes = compute_observability_changes(name, [](auto& changes, auto observer, auto observed){
            changes.emplace_back(std::pair{observer, observed});
            changes.emplace_back(std::pair{observed, observer});
        });
        */
        //auto result = d_->perform_oc({}, {changes});

        auto i = agent_infos_.at(name).id;
        agent_infos_.at(name).state = Agent::Away;

        auto changes = d_->get_domain_non_attention_propositions_id();
        // Changes are the set of deleted proposition_id when agent i disappears
        //auto result = d_->perform_ac_top_down_v2(i , {} , changes);
        auto result = d_->perform_ac_bottom_up_v1({i} , {} , changes);

        /*Confirm other agents know the agent who disappeared is not paying attention to anything */

        latest_state_ = result.second;

        d_->print_state_overview(d_->get_state(latest_state_), d_->get_domain_propositions_id());

        d_->others_agents_belief_regarding_attention(latest_state_,i);


    }
    // Comment for now because there is nto the equivalent with attention propositions
/*  
    void spy(const std::string& name) {
        auto changes = compute_observability_changes(name, [](auto& changes, auto observer, auto observed){
            changes.emplace_back(std::pair{observer, observed});
        });

        agent_infos_.at(name).state = Agent::Spying;
        auto result = d_->perform_oc({changes}, {});
        latest_state_ = result.second;
    }
*/
    void remove_conscious_attention(const std::string& agent, const std::string& object, const std::string& container) {
        auto a_id = agent_infos_.at(agent).id;
        auto p_id = d_->get_proposition_id("in(" + object + "," + container + ")");
        printf("Perform conscious attention shift of  %s out of %s located at %s\n",  agent.c_str(), object.c_str(), container.c_str());
        auto result = d_->perform_ac_top_down_v2(a_id,{}, {p_id});
        latest_state_ = result.second;
        produce_views();

        d_->print_state_overview(d_->get_state(latest_state_), d_->get_domain_propositions_id());

        d_->others_agents_belief_regarding_attention(latest_state_,a_id);
    }
    void direct_conscious_attention(const std::string& agent, const std::string& object, const std::string& container) {
        auto a_id = agent_infos_.at(agent).id;
        auto p_id = d_->get_proposition_id("in(" + object + "," + container + ")");
        printf("Perform conscious attention shift of  %s regarding %s located at %s\n",  agent.c_str(), object.c_str(), container.c_str());
        auto result = d_->perform_ac_top_down_v2(a_id,{p_id}, {});
        latest_state_ = result.second;
        produce_views();

        d_->print_state_overview(d_->get_state(latest_state_), d_->get_domain_propositions_id());

        d_->others_agents_belief_regarding_attention(latest_state_,a_id);
    }
    void direct_unconscious_attention(const std::string& agent, const std::string& object, const std::string& container) {
        //auto a_id = agent_infos_.at(agent).id;
        auto p_id = d_->get_proposition_id("in(" + object + "," + container + ")");
        printf("Perform universal unconscious attention shift regarding %s located at %s\n", object.c_str(), container.c_str());

        std::vector<del:: agent_id > agents_id ;
        for (auto agent:agents_)
        {
            agents_id.push_back(agent_infos_.at(agent).id);
        }

        auto result = d_->perform_ac_bottom_up_v1(agents_id,{p_id}, {});
        latest_state_ = result.second;
        produce_views();

        d_->print_state_overview(d_->get_state(latest_state_), d_->get_domain_propositions_id());

        //d_->others_agents_belief_regarding_attention(latest_state_,a_id);
    }
    void put_in_container(const std::string& agent, const std::string& object, const std::string& container) {
        auto a_id = agent_infos_.at(agent).id;
        auto p_id = d_->get_proposition_id("in(" + object + "," + container + ")");
        printf("Perform do put %s from %s to %s\n", object.c_str(), agent.c_str(), container.c_str());
        auto result = d_->perform_do(a_id, {p_id}, {});
        latest_state_ = result.second;
        produce_views();
    }

    void take_out_of_container(const std::string& agent, const std::string& object, const std::string& container) {
        auto a_id = agent_infos_.at(agent).id;
        auto p_id = d_->get_proposition_id("in(" + object + "," + container + ")");
        printf("Perform do take %s from %s to %s\n", object.c_str(), container.c_str(), agent.c_str());

        auto result = d_->perform_do(a_id, {}, {p_id});
        latest_state_ = result.second;
        produce_views();
    }

    void update_container(std::string& name, glm::vec3 pos) {
        if (abs(container_infos_[name].x - pos.x) > 0.2) {
            container_infos_[name].x = pos.x;
            if(d_->get_num_propositions()>0)produce_views();
        }
    }

    void log(LoggingMsg::Level level, std::string source, std::string text);

    std::string answer_question(std::string question);

    bool plan(std::string goal_object, bool knows_location);

private:

    del::formula::node_id build_formula(const std::unique_ptr<Query>& query, const std::unordered_map<std::string, std::string>& var_map, del::formula& f) const;

    std::unique_ptr<del::domain> d_;

    del::state_id latest_state_;

    std::vector<std::string> all_objects_;

    std::vector<std::string> agents_;
    std::vector<std::string> storable_;
    std::vector<std::string> containers_;

    struct Agent {
        enum State {
            Present,
            Spying,
            Away
        };
        del::agent_id id;
        State state;
    };

    struct Container {
        double x;
    };


    std::unordered_map<std::string, Agent> agent_infos_;
    std::unordered_map<std::string, Container> container_infos_;

    bool blank_views_flag = false;

    Publisher* view_pub_;
    Publisher* logging_pub_;
};
