#include <Python.h>
#include <string>
#include <thread>
#include "asr/ASR.h"

struct Context {
    PyObject* mod;
    PyThreadState* save;
    std::thread init_thread;
    bool first_run;
};

static Context ctx;

void DanishASR::initialize(Node* node) {
    Py_Initialize();
    PyEval_InitThreads();
    ctx.init_thread = std::thread([]() {
        ctx.save = PyEval_SaveThread();
        auto pys = PyGILState_Ensure();
        PyRun_SimpleString(
                "import sys\n"
                "sys.path.append('../src/asr')\n"
                "print('Initializing Python ' + sys.version)\n"
        );
        ctx.mod = PyImport_ImportModule("danspeech");
        if (!ctx.mod) {
            printf("Failed to find danspeech\n");
            return;
        }
        if (PyErr_Occurred()) PyErr_Print();
        auto init_fn = PyObject_GetAttrString(ctx.mod, "initialize");
        PyObject_CallFunction(init_fn, "");
        if (PyErr_Occurred()) PyErr_Print();
        PyGILState_Release(pys);
        printf("ASR ready!\n");
    });
    ctx.first_run = true;
}

void DanishASR::start_recording() {
    if (ctx.first_run) {
        ctx.init_thread.join();
        ctx.first_run = false;
    }
    auto pys = PyGILState_Ensure();
    auto start_fn = PyObject_GetAttrString(ctx.mod, "start_recording");
    PyObject_CallFunction(start_fn, "");
    if (PyErr_Occurred()) PyErr_Print();
    PyGILState_Release(pys);
}

std::string DanishASR::finish_recording() {
    auto pys = PyGILState_Ensure();
    auto stop_fn = PyObject_GetAttrString(ctx.mod, "finish_recording");
    PyObject* sentence = PyObject_CallFunction(stop_fn, "");
    if (PyErr_Occurred()) PyErr_Print();
    if (!sentence) { return ""; }
    PyObject* bytes = PyUnicode_AsUTF8String(sentence);
    std::string s = PyBytes_AsString(bytes);
    if (PyErr_Occurred()) return "";
    printf("%s\n", s.c_str());
    Py_DECREF(bytes);
    PyGILState_Release(pys);
    return s;
}


void DanishASR::shutdown() {
    Py_Finalize();
}
