#include "../inc/Tensor.h"

using namespace sw::core;

SIMPLEWORK_MODULE_EXPORT IModule* getSimpleWorkModule() {
    static IModulePtr s_spModule = getCoreApi()->createModule();
    return s_spModule;
}