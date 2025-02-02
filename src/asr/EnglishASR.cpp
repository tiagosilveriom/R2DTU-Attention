#include <Python.h>
#include <string>
#include <thread>
#include <msgs/NaoMsgs.h>
#include "asr/ASR.h"
#include <algorithm>

struct Context {
    PyObject* mod;
    PyThreadState* save;
    std::thread init_thread;
    bool first_run;
    Publisher* recording_pub;
};

static Context ctx;

void EnglishASR::initialize(Node* node) {
}

void EnglishASR::start_recording() {
    
}

std::string EnglishASR::finish_recording() {
    return "";
}


void EnglishASR::shutdown() {
}



std::string exec(const char* cmd) {
    char buffer[1024] = {};
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

std::string cloud_listen() {
    auto s = exec("python3 /home/pepper/r2dtu_original/src/asr/cloudspeech.py");
    std::replace( s.begin(), s.end(), '\n', ' ');
    std::replace( s.begin(), s.end(), '\r', ' ');
    printf("%s\n", s.c_str());
    if (s[0] == ':')
        return s.substr(1);
    return "?";
}

