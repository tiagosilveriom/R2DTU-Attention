#include <catch2/catch.hpp>

#include <reasoning/EpistemicReasoning.h>

void setup_standard_box_cube_scenario(EpistemicReasoning& reason) {

    reason.add_agent("sally");
    reason.add_agent("anne");

    reason.add_container("box_1");
    reason.add_container("box_2");
    reason.add_container("box_3");
    reason.add_container("box_4");

    reason.add_movable("red_cube");
    reason.add_movable("green_cube");
    reason.add_movable("blue_cube");

    reason.init();
}

TEST_CASE( "1st order FBT", "[reasoning]" ) {
    EpistemicReasoning reason;
    setup_standard_box_cube_scenario(reason);

    //Sally and Anne enters the room
    auto sally_appear = make_event("appear");
    sally_appear->set("agent", make_value("sally"));
    reason.add_event(sally_appear);

    auto anne_appear = make_event("appear");
    anne_appear->set("agent", make_value("anne"));
    reason.add_event(anne_appear);
    reason.tick();

    //Sally puts the red cube into box 1
    auto sally_put = make_event("put");
    sally_put->set("agent", make_value("sally"));
    sally_put->set("object", make_value("red_cube"));
    sally_put->set("container", make_value("box_1"));
    reason.add_event(sally_put);
    reason.tick();

    //Sally leaves the room
    auto sally_disappear = make_event("disappear");
    sally_disappear->set("agent", make_value("sally"));
    reason.add_event(sally_disappear);
    reason.tick();

    //Anne moves the red cube from box 1 to box 2
    auto anne_take = make_event("take");
    anne_take->set("agent", make_value("anne"));
    anne_take->set("object", make_value("red_cube"));
    anne_take->set("container", make_value("box_1"));
    reason.add_event(anne_take);
    reason.tick();

    auto anne_put = make_event("put");
    anne_put->set("agent", make_value("anne"));
    anne_put->set("object", make_value("red_cube"));
    anne_put->set("container", make_value("box_2"));
    reason.add_event(anne_put);
    reason.tick();

    //Sally should now have a false belief about the location of the red cube

    //Where is the red_cube?
    auto query = make_predicate("in", make_constant("red_cube"), make_variable("x"));
    auto response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //What is in box 2?
    query = make_predicate("in", make_variable("x"), make_constant("box_2"));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "red_cube");

    //Where does Anne believe the cube is?
    query = make_believes("anne", make_predicate("in", make_constant("red_cube"), make_variable("x")));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //Where does Sally believe the cube is?
    query = make_believes("sally", make_predicate("in", make_constant("red_cube"), make_variable("x")));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");

    //Where does Anne believe that Sally believe the cube is?
    query = make_believes("anne", make_believes("sally", make_predicate("in", make_constant("red_cube"), make_variable("x"))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");

    //Where does Sally believe that Anne believe the cube is?
    query = make_believes("sally", make_believes("anne", make_predicate("in", make_constant("red_cube"), make_variable("x"))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");
}

TEST_CASE( "2nd order FBT", "[reasoning]" ) {
EpistemicReasoning reason;
    setup_standard_box_cube_scenario(reason);

    //Sally and Anne enters the room
    auto sally_appear = make_event("appear");
    sally_appear->set("agent", make_value("sally"));
    reason.add_event(sally_appear);

    auto anne_appear = make_event("appear");
    anne_appear->set("agent", make_value("anne"));
    reason.add_event(anne_appear);
    reason.tick();

    //Sally puts the red cube into box 1
    auto sally_put = make_event("put");
    sally_put->set("agent", make_value("sally"));
    sally_put->set("object", make_value("red_cube"));
    sally_put->set("container", make_value("box_1"));
    reason.add_event(sally_put);
    reason.tick();

    //Sally leaves the room
    auto sally_disappear = make_event("disappear");
    sally_disappear->set("agent", make_value("sally"));
    reason.add_event(sally_disappear);
    reason.tick();

    //Sally starts spying on Anne
    auto sally_spy = make_event("spy");
    sally_spy->set("agent", make_value("sally"));
    reason.add_event(sally_spy);
    reason.tick();

    //Anne moves the red cube from box 1 to box 2
    auto anne_take = make_event("take");
    anne_take->set("agent", make_value("anne"));
    anne_take->set("object", make_value("red_cube"));
    anne_take->set("container", make_value("box_1"));
    reason.add_event(anne_take);
    reason.tick();

    auto anne_put = make_event("put");
    anne_put->set("agent", make_value("anne"));
    anne_put->set("object", make_value("red_cube"));
    anne_put->set("container", make_value("box_2"));
    reason.add_event(anne_put);
    reason.tick();

    //Anne should now have a false belief about Sally's belief about the location of the red cube

    //Where is the red_cube?
    auto query = make_predicate("in", make_constant("red_cube"), make_variable("x"));
    auto response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //What is in box 2?
    query = make_predicate("in", make_variable("x"), make_constant("box_2"));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "red_cube");

    //Where does Anne believe the cube is?
    query = make_believes("anne", make_predicate("in", make_constant("red_cube"), make_variable("x")));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //Where does Sally believe the cube is?
    query = make_believes("sally", make_predicate("in", make_constant("red_cube"), make_variable("x")));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //Where does Anne believe that Sally believe the cube is?
    query = make_believes("anne", make_believes("sally", make_predicate("in", make_constant("red_cube"), make_variable("x"))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");

    //Where does Sally believe that Anne believe the cube is?
    query = make_believes("sally", make_believes("anne", make_predicate("in", make_constant("red_cube"), make_variable("x"))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //Where does Anne believe that Sally believe that Anne believe that the cube is?
    query = make_believes("anne", make_believes("sally", make_believes("anne", make_predicate("in", make_constant("red_cube"), make_variable("x")))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");

    //Where does Sally believe that Anne believe that Sally believe that the cube is?
    query = make_believes("sally", make_believes("anne", make_believes("sally", make_predicate("in", make_constant("red_cube"), make_variable("x")))));
    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_1");
}

TEST_CASE( "Helpful action query", "[reasoning]" ) {
    EpistemicReasoning reason;
    setup_standard_box_cube_scenario(reason);

    //Sally and Anne enters the room
    auto sally_appear = make_event("appear");
    sally_appear->set("agent", make_value("sally"));
    reason.add_event(sally_appear);

    auto anne_appear = make_event("appear");
    anne_appear->set("agent", make_value("anne"));
    reason.add_event(anne_appear);
    reason.tick();

    //Sally puts the red cube into box 1
    auto sally_put = make_event("put");
    sally_put->set("agent", make_value("sally"));
    sally_put->set("object", make_value("red_cube"));
    sally_put->set("container", make_value("box_1"));
    reason.add_event(sally_put);
    reason.tick();

    //Sally leaves the room
    auto sally_disappear = make_event("disappear");
    sally_disappear->set("agent", make_value("sally"));
    reason.add_event(sally_disappear);
    reason.tick();

    //Anne moves the red cube from box 1 to box 2
    auto anne_take = make_event("take");
    anne_take->set("agent", make_value("anne"));
    anne_take->set("object", make_value("red_cube"));
    anne_take->set("container", make_value("box_1"));
    reason.add_event(anne_take);
    reason.tick();

    auto anne_put = make_event("put");
    anne_put->set("agent", make_value("anne"));
    anne_put->set("object", make_value("red_cube"));
    anne_put->set("container", make_value("box_2"));
    reason.add_event(anne_put);
    reason.tick();

    //Sally should now have a false belief about the location of the red cube

    //Anne leaves the room
    auto anne_disappear = make_event("disappear");
    anne_disappear->set("agent", make_value("anne"));
    reason.add_event(anne_disappear);
    reason.tick();

    //Sally reenters the room
    reason.add_event(sally_appear);

    //Sally says "I have come for the red cube"
    //Does she have a false belief about the red cube?
    auto query = make_and(
            make_predicate("in", make_constant("red_cube"), make_variable("x")),
            make_believes("sally", make_not(make_predicate("in", make_constant("red_cube"), make_variable("x")))));

    auto response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "box_2");

    //Sally says "I have come for box 1"
    //Does she have a false belief about box 1?
    query = make_and(
            make_not(make_predicate("in", make_variable("x"), make_constant("box_1"))),
            make_believes("sally", make_predicate("in", make_variable("x"), make_constant("box_1"))));

    response = reason.query(query);
    REQUIRE(response.size() == 1);
    REQUIRE(response[0] == "red_cube");

    //Sally says "I have come for box 2"
    //Does she have a false belief about box 2?
    query = make_and(
            make_not(make_predicate("in", make_variable("x"), make_constant("box_2"))),
            make_believes("sally", make_predicate("in", make_variable("x"), make_constant("box_2"))));

    response = reason.query(query);
    REQUIRE(response.size() == 0);
}

