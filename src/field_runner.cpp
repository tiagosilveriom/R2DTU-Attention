#include <stdio.h>
#include <unistd.h>

#include <core/Node.h>
#include <msgs/ControlMsgs.h>
#include <msgs/ActuationMsgs.h>
#include <msgs/ActingMsgs.h>
#include <msgs/NaoMsgs.h>

void print_usage() {
    printf("Usage: field -i onboard_ip [OPTION]\n");
    printf("  --s \t\t\t Run stand alone without connecting to the robot\n");
    printf("  --r NAME\t\t Stores all received sensor data into folder NAME\n");
    printf("  --p NAME\t\t Playback of recorded sensor data from folder NAME\n");
}

int main(int argc, char** argv) {

    Node node;
    node.id = 1;

    std::string onboard_ip = "127.0.0.1";
    bool record = false;
    bool playback = false;
    bool stand_alone = false;
    std::string mock_folder;

    int c;
    while ((c = getopt(argc, argv, "si:r:p:")) != -1) {
        switch (c) {
            case 's':
                stand_alone = true;
                break;
            case 'i':
                onboard_ip = optarg;
                break;
            case 'r':
                record = true;
                mock_folder = optarg;
                break;
            case 'p':
                playback = true;
                mock_folder = optarg;
                break;
            case '?':
                print_usage();
                return -1;
            default:
                abort();
        }
    }

    if (playback || stand_alone) {
        node.initialize("dist/r2dtu", false, nullptr);
    } else {
        node.initialize("dist/r2dtu", false, onboard_ip.c_str());
    }

    if (record || playback) {
        node.access_kv().set(KVStore::Always, "mockbot-dir", mock_folder);
        if (record) node.load_module("dist/modules/libmockbot_recorder.so", 30);
        if (playback) node.load_module("dist/modules/libmockbot_player.so", 30);
        if (record && playback) {
            printf("Cannot record and playback simultaneously!\n");
            return -1;
        }

    }

    node.load_module("dist/modules/libocs.so", 30);
    node.load_module("dist/modules/libdemo.so", 30);
    node.load_module("dist/modules/libperception.so", 30);



    node.network_advertise<GamepadAxisMsg>("gamepad_axis");
    node.network_advertise<GamepadButtonMsg>("gamepad_button");
    node.network_advertise<MoveToMsg>("move_to_cmd");
    node.network_advertise<TrajectoryCmdMsg>("trajectory_cmd");
    node.network_advertise<JointCmdMsg>("joint_cmd");
    node.network_advertise<TTSSayMsg>("tts_say");
    node.network_advertise<TTSSettingsMsg>("tts_settings");
    node.network_advertise<AnimationCmdMsg>("animation_cmd");
    node.network_advertise<ToggleBreathingMsg>("toggle_breathing");
    node.network_advertise<AudioRecordMsg>("audio_record");

    node.start();

    node.spin();

    node.shutdown();

    return 0;
}
