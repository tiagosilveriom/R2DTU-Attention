#pragma once

struct Module;

namespace ModuleLoader {

    Module* load(const char* name);

    void unload(Module* module);
}
