#include <stdio.h>

#include <core/Node.h>
#include <msgs/CameraMsgs.h>
#include <msgs/AudioMsgs.h>
#include <msgs/ProprioceptiveMsgs.h>
#include <msgs/ActingMsgs.h>

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: onboard master_ip");
        exit(-1);
    }

    Node node;
    node.id = 0;
    node.initialize("dist/r2dtu", true, argv[1]);


    node.load_module("modules/libonboard_cameras.so", 30);
    node.load_module("modules/libonboard_nao.so", 30);

    node.network_advertise<ImageMsg>("vis_forward");
    node.network_advertise<ImageMsg>("vis_down");
    node.network_advertise<SkeletonMsg>("skeleton");
    node.network_advertise<JointSensorMsg>("joints_sensor");
    node.network_advertise<HealthTelemetryMsg>("health_telemetry");
    node.network_advertise<WavMsg>("tts_network");

    node.spin();

    node.shutdown();

    return 0;
}
