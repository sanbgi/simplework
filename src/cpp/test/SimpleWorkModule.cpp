#include "../inc/SimpleWork.h"

//SIMPLEWORK_MODULE_REGISTER("sw.test")

SIMPLEWORK_MODULE_EXPORT SIMPLEWORK_CORE_NAMESPACE::IModule* getSimpleWork() { 
    static SIMPLEWORK_CORE_NAMESPACE::IModulePtr s_spModule = SIMPLEWORK_CORE_NAMESPACE::getCoreApi()->createModule("sw.test");
    return s_spModule; \
}
