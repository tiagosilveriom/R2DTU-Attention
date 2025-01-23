#pragma once

#include <common/RobotDefs.h>

#include "MsgsStart.h"


struct ActuatorTelemetryMsg {
    float current[NUMBER_OF_JOINTS];
    float temperature[NUMBER_OF_JOINTS];
    float stiffness[NUMBER_OF_JOINTS];
};

struct OBCTelemetryMsg {
    float temperature;
    float battery;
};

struct ActuatorPositionsMsg {
    float positions[NUMBER_OF_JOINTS];
};

const uint8_t DCM_MERGE_COMMAND = 0x0;
const uint8_t DCM_CLEAR_ALL_COMMAND = 0x1;
const uint8_t DCM_CLEAR_AFTER_COMMAND = 0x2;
const uint8_t DCM_CLEAR_BEFORE_COMMAND = 0x3;
const uint8_t DCM_COMMAND_MASK = 0x3;
const uint8_t DCM_STIFFNESS_FLAG = 0x4;

struct DcmCmdMsg {
    MsgBlob values;
    MsgBlob timestamps;
    uint8_t actuator;
    uint8_t update_type;
};

struct MoveToMsg {
    float forward;
    float left;
    float theta;
};

struct TrajectoryCmdMsg {
    int32_t num_timepoints;
    MsgBlob timestamps;
    MsgBlob num_values;
    MsgBlob actuators;
    MsgBlob values;
};

struct JointCmdMsg {
    float speed;
    uint32_t num_values;
    MsgBlob actuators;
    MsgBlob values;
};

struct AnimationCmdMsg {
    MsgText name;
};

struct ToggleBreathingMsg {
    bool enabled;
};

enum class TabletCmdType : uint8_t {
    Sleep,
    WakeUp,
    Reset,
    LoadUrl,
    ShowImg,
};

struct TabletCmdMsg {
    MsgText arg;
    TabletCmdType type;
};

enum class TTSLang : uint8_t {
    English,
    Danish,
};

struct TTSSettingsMsg {
    float pitch;
    float volume;
    float speed;
    TTSLang lang;
    bool over_network;
};

struct TTSSayMsg {
    MsgText text;
};

struct TTSSayToFileMsg {
    MsgText text;
    MsgText file;
};

struct AudioPlayMsg {
    float volume;
    float pan;
    MsgText filename;
    bool do_not_cache;
};

struct AudioRecordMsg {
    MsgText filename;
};

#include "MsgsEnd.h"
