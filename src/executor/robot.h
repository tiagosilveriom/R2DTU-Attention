#pragma once

#include <core/Module.h>
#include <acting/Trajectory.h>
#include <behaviour/Behaviour.h>
#include <msgs/ControlMsgs.h>
#include <reasoning/WorldModel.h>
#include <glm/gtx/string_cast.hpp>
#include <asr/ASR.h>
#include <l10n/FBT.h>

class Robot {
public:
    explicit Robot(Node* node) : world(*node) {
        trajectory_pub = node->advertise<TrajectoryCmdMsg>("trajectory_cmd");
        joint_cmd_pub = node->advertise<JointCmdMsg>("joint_cmd");
        move_pub = node->advertise<MoveToMsg>("move_to_cmd");
        say_pub = node->advertise<TTSSayMsg>("tts_say");
        animation_pub = node->advertise<AnimationCmdMsg>("animation_cmd");
        terminal_pub = node->advertise<TerminalCmdMsg>("terminal_cmd");
        terminal_override_pub = node->advertise<TerminalOverrideMsg>("terminal_override");
        tts_settings = node->advertise<TTSSettingsMsg>("tts_settings");
        toggle_breathing_pub = node->advertise<ToggleBreathingMsg>("toggle_breathing");
        logging_pub = node->advertise<LoggingMsg>("logging");

        asr = LANG_DANISH ? (ASR*) new DanishASR() : (ASR*) new EnglishASR();
        asr->initialize(node);
    }

    ~Robot() {
        asr->shutdown();
        delete asr;
    }

    void play_animation(const std::string& name) {
        MessageBuilder<AnimationCmdMsg> msg;
        msg.write_string(&msg->name, name);
        animation_pub->publish(msg);
    }

    void say(std::string sentence) {
        MessageBuilder<TTSSayMsg> msg;
        msg.write_string(&msg->text, sentence);
        say_pub->publish(msg);
    }

    void setup_tts(bool danish, float speed) {
        MessageBuilder<TTSSettingsMsg> msg;
        msg->lang = (danish) ? TTSLang::Danish : TTSLang::English;
        msg->speed = speed;
        msg->pitch = 1.0;
        msg->volume = 0.6;
	    msg->over_network = false;

        tts_settings->publish(msg);
    }

    void start_recording() {
        asr->start_recording();
    }

    std::string finish_recording() {
        return "\"" + asr->finish_recording() + "?\"";
    }

    void send_to_terminal(std::string text) {
        MessageBuilder<TerminalOverrideMsg> override_msg;
        override_msg.write_string(&override_msg->text, text);
        terminal_override_pub->publish(override_msg);
    }

    void hear(std::string text) {
        MessageBuilder<TerminalCmdMsg> cmd_msg;
        cmd_msg.write_string(&cmd_msg->cmd_type, "hear:");
        cmd_msg.write_string(&cmd_msg->param, text);
        terminal_pub->publish(cmd_msg);
    }

    void tick() {
        world.tick();
    }

    void toggle_breathing(bool enabled) {
        MessageBuilder<ToggleBreathingMsg> msg;
        msg->enabled = enabled;
        toggle_breathing_pub->publish(msg);
    }

    void start_learn(uint32_t id) {
        MessageBuilder<TerminalCmdMsg> msg;
        std::string cmd = "startlearn";
        std::string id_str = std::to_string(id);
        msg.write_string(&msg->cmd_type, cmd);
        msg.write_string(&msg->param, id_str);
        terminal_pub->publish(msg);
    }

    void log(LoggingMsg::Level level, std::string source, std::string text) {
        MessageBuilder<LoggingMsg> msg;
        msg->level = level;
        msg.write_string(&msg->source, source);
        msg.write_string(&msg->text, text);
        time(&msg->timestamp);
        printf("%s: %s\n", source.c_str(), text.c_str());
        logging_pub->publish(msg);
    }

    void log_info(std::string source, std::string text) {
        log(LoggingMsg::INFO, source, text);
    }

    void log_warn(std::string source, std::string text) {
        log(LoggingMsg::WARNING, source, text);
    }

    void log_error(std::string source, std::string text) {
        log(LoggingMsg::ERROR, source, text);
    }

    World world;

    void set_joint_angels(float pitch, float roll) {
        Trajectory trajectory;

        Timepoint point(300);
        point.add(JOINT_L_SHOULDER_ROLL, roll);
        point.add(JOINT_L_SHOULDER_PITCH, pitch);
        trajectory.add(point);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void set_head_angles(float pitch, float yaw, int32_t timestamp) {
        MessageBuilder<JointCmdMsg> msg;

        msg->num_values = 2;
        msg->speed = 0.2;

        uint8_t actuators[] = {JOINT_HEAD_PITCH, JOINT_HEAD_YAW};
        float values[] = {pitch, yaw};

        msg.write_blob(&msg->actuators, &actuators[0], sizeof(uint8_t) * 2);
        msg.write_blob(&msg->values, (uint8_t*) &values[0], sizeof(float) * 2);

        joint_cmd_pub->publish(msg);

        current_pitch = pitch;
        current_yaw = yaw;
    }

    void push() {
        Trajectory trajectory;

        Timepoint start_look(500);
        start_look.add(JOINT_HEAD_PITCH, 0.3);
        start_look.add(JOINT_HEAD_YAW, -0.3);
        trajectory.add(start_look);

        Timepoint start(1000);
        start.add(JOINT_R_ELBOW_ROLL, 1.57);
        start.add(JOINT_R_ELBOW_YAW, 1.58);
        start.add(JOINT_R_HAND, 1.0);
        start.add(JOINT_R_SHOULDER_PITCH, 1.73);
        start.add(JOINT_R_SHOULDER_ROLL, 0.0);
        start.add(JOINT_R_WRIST_YAW, -1.8);
        trajectory.add(start);

        Timepoint touch(2500);
        touch.add(JOINT_R_ELBOW_ROLL, 0.0);
        touch.add(JOINT_R_SHOULDER_PITCH, 0.56);
        trajectory.add(touch);

        Timepoint touch_end(3000);
        touch_end.add(JOINT_R_ELBOW_YAW, 1.58);
        touch_end.add(JOINT_R_HAND, 1.0);
        touch_end.add(JOINT_R_SHOULDER_ROLL, 0.0);
        touch_end.add(JOINT_R_WRIST_YAW, -1.8);
        touch_end.add(JOINT_HEAD_PITCH, 0.3);
        touch_end.add(JOINT_HEAD_YAW, -0.3);
        trajectory.add(touch_end);

        Timepoint end(4500);
        end.add(JOINT_R_ELBOW_ROLL, 0.26);
        end.add(JOINT_R_ELBOW_YAW, 0.87);
        end.add(JOINT_R_HAND, 0.6);
        end.add(JOINT_R_SHOULDER_PITCH, 1.56);
        end.add(JOINT_R_SHOULDER_ROLL, -0.17);
        end.add(JOINT_R_WRIST_YAW, 0.0);
        end.add(JOINT_HEAD_PITCH, 0.0);
        end.add(JOINT_HEAD_YAW, 0.0);
        trajectory.add(end);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void fbt2_push() {
        toggle_breathing(false);
        Trajectory trajectory;

        Timepoint start_look(500);
        start_look.add(JOINT_HEAD_PITCH, 0.3);
        start_look.add(JOINT_HEAD_YAW, -0.3);
        start_look.add(JOINT_R_ELBOW_ROLL, 0.02);
        start_look.add(JOINT_R_ELBOW_YAW, 1.6);
        start_look.add(JOINT_R_HAND, 0.4);
        start_look.add(JOINT_R_SHOULDER_PITCH, 1.4);
        start_look.add(JOINT_R_SHOULDER_ROLL, -0.14);
        trajectory.add(start_look);

        Timepoint start(1000);
        start.add(JOINT_R_ELBOW_ROLL, 0.0);
        start.add(JOINT_R_ELBOW_YAW, 0.35);
        start.add(JOINT_R_HAND, 1.0);
        start.add(JOINT_R_SHOULDER_PITCH, 0.20);
        start.add(JOINT_R_SHOULDER_ROLL, -0.70);
        start.add(JOINT_R_WRIST_YAW, 1.38);
        trajectory.add(start);

        Timepoint push_forward(3000);
        push_forward.add(JOINT_R_SHOULDER_ROLL, -0.00);
        push_forward.add(JOINT_R_ELBOW_YAW, 0.00);
        push_forward.add(JOINT_R_ELBOW_ROLL, 0.5);
        trajectory.add(push_forward);

        Timepoint push_backward(4500);
        push_backward.add(JOINT_R_SHOULDER_ROLL, -0.70);
        push_backward.add(JOINT_R_ELBOW_YAW, 0.35);
        push_backward.add(JOINT_R_SHOULDER_PITCH, 0.2);
        trajectory.add(push_backward);

        Timepoint end(5500);
        end.add(JOINT_R_SHOULDER_PITCH, 1.4);
        trajectory.add(end);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void handshake_up() {
        Trajectory trajectory;
        Timepoint start(1000);
        start.add(JOINT_HEAD_PITCH, -0.3);
        start.add(JOINT_R_ELBOW_ROLL, 0.14);
        start.add(JOINT_R_WRIST_YAW, 1.7);
        start.add(JOINT_R_ELBOW_YAW, 0);
        start.add(JOINT_R_HAND, 1.0);
        start.add(JOINT_R_SHOULDER_PITCH, -0.3);
        start.add(JOINT_R_SHOULDER_ROLL, -0.1);
        trajectory.add(start);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void handshake_reciprocate() {
        Trajectory trajectory;
        Timepoint start(500);
        start.add(JOINT_R_HAND, 0.5);
        trajectory.add(start);

        Timepoint release(2500);
        release.add(JOINT_R_HAND, 0.9);
        trajectory.add(release);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void handshake_down() {
        Trajectory trajectory;
        Timepoint end(1500);
        end.add(JOINT_R_ELBOW_ROLL, 0.26);
        end.add(JOINT_R_ELBOW_YAW, 0.87);
        end.add(JOINT_R_HAND, 0.6);
        end.add(JOINT_R_SHOULDER_PITCH, 1.56);
        end.add(JOINT_R_SHOULDER_ROLL, -0.17);
        end.add(JOINT_R_WRIST_YAW, 0.0);
        trajectory.add(end);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }

    void calibration_done() {
        current_pos = world.get_source(2).camera_to_world_space({0,0,0});
    }

    void move_to(glm::vec3 position) {
        MessageBuilder<MoveToMsg> msg;
        glm::vec3 delta = position - current_pos;

        msg->forward = delta.z;
        msg->left = -delta.x;
        msg->theta = 0.0;

        delta.y = 0;

        current_pos += delta;

        move_pub->publish(msg);
    }

    void turn(float angle) {
        MessageBuilder<MoveToMsg> msg;

        msg->forward = 0;
        msg->left = 0;
        msg->theta = angle - current_theta;

        current_theta = angle;

        move_pub->publish(msg);
    }

    void look_at(glm::vec3 position) {
        auto dir = position - current_pos;

	auto now = std::chrono::steady_clock::now();

	if (now - last_look_at_update < std::chrono::milliseconds(500))
	  return;

	last_look_at_update = now;

        float pitch = atan2(dir.y, dir.z);
        float yaw = -atan2(dir.x, dir.z);
        int32_t timestamp = 150;

        if (abs(pitch - current_pitch) > 0.01 || abs(yaw - current_yaw) > 0.01 ) {
            set_head_angles(pitch, yaw - current_theta, timestamp);
        }
    }

void look_at_box(glm::vec3 position) {
    auto dir = position - current_pos;  // Calculate direction to the target

    float pitch_offset = -0.5f;  // Slightly raise gaze by 0.1 radians
    float pitch = atan2(dir.y, dir.z) + pitch_offset;  // Apply offset to pitch
    float yaw = -atan2(dir.x, dir.z);  // Calculate yaw as usual

    int32_t timestamp = 150;

    //std::cout << "\nPitch Adjustment: " << pitch_offset << " | Final Pitch: " << pitch << "\n";
    //std::cout << "Yaw: " << yaw << "\n";

    set_head_angles(pitch, yaw - current_theta, timestamp);
}


    glm::vec3 get_current_pos() {
        return current_pos;
    }

    void turn_torso_to(glm::vec3 target_position) {
        glm::vec3 direction = target_position - current_pos;

        // Calculate angle for turning
        float yaw = -atan2(direction.x, direction.z);   // Horizontal angle (for head and torso)

        MessageBuilder<MoveToMsg> torso_turn_msg;
        torso_turn_msg->forward = 0;
        torso_turn_msg->left = 0;
        torso_turn_msg->theta = yaw - current_theta;  // Rotate torso to match yaw


        // Publish torso turn to face the object
        move_pub->publish(torso_turn_msg);

        current_theta = yaw;  // Update current torso yaw

    }

    void rest_arm_position() {

        // Use arm to point
        Trajectory trajectory;

        // Maintain pointing posture for the specified time_pointing duration
        Timepoint end(1000);
        end.add(JOINT_R_ELBOW_ROLL, 0.26);
        end.add(JOINT_R_ELBOW_YAW, 0.87);
        end.add(JOINT_R_HAND, 0.6);
        end.add(JOINT_R_SHOULDER_PITCH, 1.56);
        end.add(JOINT_R_SHOULDER_ROLL, -0.17);
        end.add(JOINT_R_WRIST_YAW, 0.0);
        end.add(JOINT_HEAD_PITCH, 0.0);
        end.add(JOINT_HEAD_YAW, 0.0);
        trajectory.add(end);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }
    void point_at(glm::vec3 target_position) {
        // Calculate direction vector from the robot's position to the target
        glm::vec3 direction = target_position - current_pos;

        // Calculate angles for pointing and turning
        float pitch = atan2(direction.y, direction.z);  // Vertical angle (for head and arm)
        float yaw = -atan2(direction.x, direction.z);   // Horizontal angle (for head and torso)
        
        //int32_t timestamp = 15000;

        //set_head_angles(pitch, yaw - current_theta, timestamp);

        // Use arm to point
        Trajectory trajectory;

        Timepoint start(1000);  // Start pointing after 1 second
        start.add(JOINT_R_SHOULDER_PITCH, pitch);
        start.add(JOINT_R_SHOULDER_ROLL, yaw);  // Align shoulder with target yaw
        trajectory.add(start);

        
        // Use head to gaze at the object
        //set_head_angles(pitch, yaw - current_theta, timestamp);

        auto msg = trajectory.pack();
        trajectory_pub->publish(msg);
    }


    float current_theta;
private:
    Publisher* trajectory_pub;
    Publisher* joint_cmd_pub;
    Publisher* move_pub;
    Publisher* say_pub;
    Publisher* animation_pub;
    Publisher* terminal_pub;
    Publisher* terminal_override_pub;
    Publisher* tts_settings;
    Publisher* toggle_breathing_pub;
    Publisher* logging_pub;

    ASR* asr;
    std::chrono::time_point<std::chrono::steady_clock> last_look_at_update;

    float current_pitch;
    float current_yaw;
    glm::vec3 current_pos;
};

