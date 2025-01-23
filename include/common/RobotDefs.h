#pragma once

#include <stdint.h>
#include <unordered_map>
#include <string>

const uint32_t JOINT_HEAD_PITCH = 0;
const uint32_t JOINT_HEAD_YAW = 1;
const uint32_t JOINT_HIP_PITCH = 2;
const uint32_t JOINT_HIP_ROLL = 3;
const uint32_t JOINT_KNEE_PITCH = 4;
const uint32_t JOINT_L_ELBOW_ROLL = 5;
const uint32_t JOINT_L_ELBOW_YAW = 6;
const uint32_t JOINT_L_HAND = 7;
const uint32_t JOINT_L_SHOULDER_PITCH = 8;
const uint32_t JOINT_L_SHOULDER_ROLL = 9;
const uint32_t JOINT_L_WRIST_YAW = 10;
const uint32_t JOINT_R_ELBOW_ROLL = 11;
const uint32_t JOINT_R_ELBOW_YAW = 12;
const uint32_t JOINT_R_HAND = 13;
const uint32_t JOINT_R_SHOULDER_PITCH = 14;
const uint32_t JOINT_R_SHOULDER_ROLL = 15;
const uint32_t JOINT_R_WRIST_YAW = 16;
const uint32_t JOINT_WHEEL_FL = 17;
const uint32_t JOINT_WHEEL_FR = 18;
const uint32_t JOINT_WHEEL_B = 19;
const uint32_t NUMBER_OF_JOINTS = 20;

const static std::unordered_map<uint8_t, std::string> joint_names {
        {0, "HeadPitch"},
        {1, "HeadYaw"},
        {2, "HipPitch"},
        {3, "HipRoll"},
        {4, "KneePitch"},
        {5, "LElbowRoll"},
        {6, "LElbowYaw"},
        {7, "LHand"},
        {8, "LShoulderPitch"},
        {9, "LShoulderRoll"},
        {10, "LWristYaw"},
        {11, "RElbowRoll"},
        {12, "RElbowYaw"},
        {13, "RHand"},
        {14, "RShoulderPitch"},
        {15, "RShoulderRoll"},
        {16, "RWristYaw"},
        {17, "WheelFL"},
        {18, "WheelFR"},
        {19, "WheelB"},
};

inline std::string joint_id_to_name(uint8_t id) {
    return joint_names.at(id);
}
