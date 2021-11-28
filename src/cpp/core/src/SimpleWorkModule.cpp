#include "Core.h"
#include "CoreApi.hpp"

using namespace SIMPLEWORK_CORE_NAMESPACE;
IModule* __getSimpleWorkCoreModule() {
    static IModulePtr s_spModule = getCoreApi()->createModule(SIMPLEWORK_CORE_MODULEKEY);
    return s_spModule;
}
