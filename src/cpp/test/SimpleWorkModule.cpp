#include "../inc/SimpleWork.h"

//SIMPLEWORK_MODULE_REGISTER("sw.test")

SIMPLEWORK_MODULE_EXPORT SIMPLEWORK_CORE_NAMESPACE::Module& getSimpleWork() { 
    static SIMPLEWORK_CORE_NAMESPACE::Module s_spModule = SIMPLEWORK_CORE_NAMESPACE::getCoreApi()->createModule("sw.test");
    return s_spModule;
}
