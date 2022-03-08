
#include "module.h"
#include <string>

using namespace sw;
using namespace std;

static SCtx sCtx("CGapModule");
class CGapModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);
    
private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "GapModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GapModule"; }
};

int CGapModule::__initialize(const PData* pData) {
    return sCtx.success();
}

int CGapModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("Gvp单元输入参数必须为一个");
    }

    spOutVar = spInVars[0].gap();
    return sCtx.success();
}

int CGapModule::toArchive(const SArchive& ar) {
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGapModule, CGapModule::__getClassKey())