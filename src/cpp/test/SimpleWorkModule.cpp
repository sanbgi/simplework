#include "../inc/SimpleWork.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
SIMPLEWORK_MODULE_EXPORT IModule* getSimpleWorkModule() {
    static IModulePtr s_spModule = getCoreApi()->createModule("sw.test");
    return s_spModule;
}