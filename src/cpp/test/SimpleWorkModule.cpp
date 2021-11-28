#include "../inc/SimpleWork.h"

sw::IModule* getSimpleWorkModule() {
    static sw::IModulePtr s_spModule = sw::getCoreApi();
    return s_spModule;
}