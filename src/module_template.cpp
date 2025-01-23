#include <core/Node.h>
#include <core/Module.h>

struct Context {

};

extern "C" {

ModuleState* initialize(Node* node) {

}

void tick(Node* node, Context* ctx) {

}

void shutdown(Node* node, Context* ctx) {

}

void unload(Node* node, Context* ctx) {

}

void reload(Node* node, Context* ctx) {

}

} //extern "C"