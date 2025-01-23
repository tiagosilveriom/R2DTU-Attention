#include <core/ModuleLoader.h>

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

#include <core/Module.h>

namespace ModuleLoader {

    bool load_function(void* handle, const char* name, void** place) {
        dlerror();
        *place = dlsym(handle, name);
        auto err = dlerror();
        if (err) {
            printf("Failed to load symbol %s\n%s\n", name, err);
            return false;
        }
        return true;
    }

    Module* load(const char* name) {
        auto handle = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
        if (!handle) {
            printf("Failed to load module %s\n%s\n", name, dlerror());
            return nullptr;
        }

        auto mod = (Module*) malloc(sizeof(Module));
        mod->dl_handle = handle;
        mod->name = name;
        if (!(load_function(handle, "initialize", (void**) &mod->initialize) &&
              load_function(handle, "tick", (void**) &mod->tick) &&
              load_function(handle, "shutdown", (void**) &mod->shutdown) &&
              load_function(handle, "unload", (void**) &mod->unload) &&
              load_function(handle, "reload", (void**) &mod->reload))) {
            printf("Aborting load of module %s\n", name);
            free(mod);
            dlclose(handle);
            return nullptr;
        }

        return mod;
    }

    Module* reload(Module* old) {
    //old->unload();

    auto name = old->name;

    ModuleLoader::unload(old);

    auto mod = ModuleLoader::load(name);
    //mod->reload();

    return mod;
}

    void unload(Module* module) {
        dlclose(module->dl_handle);
        free(module);
    }

}

