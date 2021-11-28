#include "../inc/Tensor.h"

sw::IModule* getSimpleWorkModule() {
    static sw::IModulePtr s_spModule = sw::getCoreApi()->createModule();
    return s_spModule;
}