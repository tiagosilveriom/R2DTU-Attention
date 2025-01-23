#include <stdio.h>

#include <core/Node.h>
#include <msgs/TestMsgs.h>

int main(int argc, char** argv) {


    if (argc != 3) {
        printf("Usage: core_test master_ip node_id");
        exit(-1);
    }

    Node node;

    auto ip = argv[1];
    auto id = (uint16_t) atoi(argv[2]);
    node.id = id;

    node.initialize("../core_test", id == 0, ip);

    node.load_module("dist/modules/libcore_test_mod.so", 2);

    if (node.id == 0) {
        node.network_advertise<TestReqMsg>("test_cmd");
    } else {
        node.network_advertise<TestRespMsg>("test_resp");
        node.start();
    }

    node.spin();

    node.shutdown();

    return 0;
}
