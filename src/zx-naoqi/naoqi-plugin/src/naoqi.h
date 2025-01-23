#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//System
bool naoqi_initialize();
void naoqi_shutdown();

void naoqi_shutdown_robot();
void naoqi_reboot_robot();

//Constants

const uint8_t NAOQI_ACTUATOR_HEAD_PITCH = 0;
const uint8_t NAOQI_ACTUATOR_HEAD_YAW = 1;
const uint8_t NAOQI_ACTUATOR_R_SHOULDER_ROLL = 2;
const uint8_t NAOQI_ACTUATOR_R_SHOULDER_PITCH = 3;
const uint8_t NAOQI_ACTUATOR_R_ELBOW_YAW = 4;
const uint8_t NAOQI_ACTUATOR_R_ELBOW_ROLL = 5;
const uint8_t NAOQI_ACTUATOR_R_WRIST_YAW = 6;
const uint8_t NAOQI_ACTUATOR_R_HAND = 7;
const uint8_t NAOQI_ACTUATOR_L_SHOULDER_ROLL = 8;
const uint8_t NAOQI_ACTUATOR_L_SHOULDER_PITCH = 9;
const uint8_t NAOQI_ACTUATOR_L_ELBOW_YAW = 10;
const uint8_t NAOQI_ACTUATOR_L_ELBOW_ROLL = 11;
const uint8_t NAOQI_ACTUATOR_L_WRIST_YAW = 12;
const uint8_t NAOQI_ACTUATOR_L_HAND = 13;
const uint8_t NAOQI_ACTUATOR_HIP_PITCH = 14;
const uint8_t NAOQI_ACTUATOR_HIP_ROLL = 15;
const uint8_t NAOQI_ACTUATOR_KNEE_PITCH = 16;
const uint8_t NAOQI_ACTUATOR_WHEEL_FL = 17;
const uint8_t NAOQI_ACTUATOR_WHEEL_FR = 18;
const uint8_t NAOQI_ACTUATOR_WHEEL_B = 19;
const uint8_t NAOQI_ACTUATOR_COUNT = 20;

//Tablet

void naoqi_tablet_connect_wifi();
void naoqi_tablet_disconnect_wifi();
void naoqi_tablet_set_brightness();
void naoqi_tablet_get_brightness();
void naoqi_tablet_reset();
void naoqi_tablet_set_screen_on(bool on);
void naoqi_tablet_show_image(const char* url);
void naoqi_tablet_load_url();
void naoqi_tablet_sleep();
void naoqi_tablet_wakeup();

//Sensors

struct naoqi_sensors_actuator_telemetry {
    float current[NAOQI_ACTUATOR_COUNT];
    float temperature[NAOQI_ACTUATOR_COUNT];
    float stiffness[NAOQI_ACTUATOR_COUNT];
};

void naoqi_sensors_read_actuator_telemetry();
void naoqi_sensors_read_actuator_position(uint8_t actuator);
float naoqi_sensors_read_cpu_temp();
float naoqi_sensors_read_battery_charge();
bool naoqi_sensors_is_power_hatch_open();

//Audio

void naoqi_tts_say(const char* sentence);
void naoqi_tts_say_to_file(const char* sentence, const char* filename);
void naoqi_tts_set_language(const char* language);
float naoqi_tts_get_speed();
void naoqi_tts_set_speed(float ratio);
float naoqi_tts_get_pitch();
void naoqi_tts_set_pitch(float ratio);
float naoqi_tts_get_volume();
void naoqi_tts_set_volume(float gain);
void naoqi_tts_interrupt_speech();

typedef int32_t naoqi_audio_handle_t;
naoqi_audio_handle_t naoqi_audio_load_file(const char* file_name);
void naoqi_audio_play_handle(naoqi_audio_handle_t, float volume, float pan);
void naoqi_audio_stop_all();

//DCM

int32_t naoqi_dcm_get_timestamp();

const uint8_t NAOQI_DCM_MERGE_COMMAND = 0x0;
const uint8_t NAOQI_DCM_CLEAR_ALL_COMMAND = 0x1;
const uint8_t NAOQI_DCM_CLEAR_AFTER_COMMAND = 0x2;
const uint8_t NAOQI_DCM_CLEAR_BEFORE_COMMAND = 0x3;

void naoqi_dcm_send_command(uint8_t actuator, uint8_t update_type, bool is_stiffness, uint8_t num_points, float* point_values, int32_t* point_timestamps);

#ifdef __cplusplus
} // extern "C"
#endif
