#pragma once
#include "MsgsStart.h"

struct GamepadButtonMsg {
    uint32_t timestamp;
    uint8_t button_id;
    uint8_t state;
};


#define GAMEPAD_B 0
#define GAMEPAD_A 1
#define GAMEPAD_Y 2
#define GAMEPAD_X 3
#define GAMEPAD_L1 4
#define GAMEPAD_R1 5
#define GAMEPAD_L2 6
#define GAMEPAD_R2 7
#define GAMEPAD_MINUS 8
#define GAMEPAD_PLUS 9
#define GAMEPAD_LEFT_STICK 10
#define GAMEPAD_RIGHT_STICK 11
#define GAMEPAD_HOME 12
#define GAMEPAD_CIRCLE 13
#define GAMEPAD_DUP 14
#define GAMEPAD_DRIGHT 15
#define GAMEPAD_DDOWN 16
#define GAMEPAD_DLEFT 17


struct GamepadAxisMsg {
    uint32_t timestamp;
    uint8_t axis_id;
    int16_t value;
};


struct TerminalCmdMsg {
    MsgText cmd_type;
    MsgText param;
};

struct TerminalOverrideMsg {
    MsgText text;
};

struct StateCmdMsg {
    enum Type : uint8_t {
        Confirm,
        Next,
        Back,
    };
    Type type;
};

struct LoggingMsg {
    enum Level : uint64_t {
        ERROR = 0,
        WARNING = 1,
        INFO = 2,
    };
    Level level;
    MsgText source;
    MsgText text;
    time_t timestamp;
};

struct TextDrawingMsg {
    int32_t x,y;
    MsgText text;
};

#include "MsgsEnd.h"

