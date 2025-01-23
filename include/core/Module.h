#pragma once

#include <stdint.h>

struct Node;

typedef void ModuleState;

struct Module {

    //Initialize the module state and subsystems
    ModuleState* (*initialize)(Node*);

    //Will be called at a fixed frequency
    void (*tick)(Node*, ModuleState*);

    //The process is shutting down and the module will not be reloaded
    void (*shutdown)(Node*, ModuleState*);

    //Stop all activities and prepare state for handover to reloaded module
    void (*unload)(Node*, ModuleState*);

    //Initialize the module by reusing the old state
    void (*reload)(Node*, ModuleState*);

    void* dl_handle;
    const char* name;
};
