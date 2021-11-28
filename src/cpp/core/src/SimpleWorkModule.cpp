#include "Core.h"
#include "CoreApi.hpp"

sw::IModule* __getSimpleWorkCoreModule() {
    static sw::IModulePtr s_spModule = sw::CCoreApi::createSimpleWorkCoreModule();
    return s_spModule;
}
