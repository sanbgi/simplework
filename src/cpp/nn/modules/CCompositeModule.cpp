#include "module.h"
#include <vector>
using namespace std;

static SCtx sCtx("CCompositeModule");

class CCompositeModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int __initialize(const PArg* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "CompositeModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.CompositeModule"; }

private:
    vector<SNnModule> m_arrModules;
};

int CCompositeModule::__initialize(const PArg* pData) {
    const SNnModule::PNnCompositeModule* pModule = CArg<SNnModule::PNnCompositeModule>(pData);
    if(pModule == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    for(int i=0; i<pModule->nModules; i++) {
        m_arrModules.push_back(pModule->pModules[i]);
    }
    return sCtx.success();
}

int CCompositeModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("序列执行单元输入参数必须为一个");
    }

    SNnVariable x = spInVars[0];
    vector<SNnModule>::iterator it = m_arrModules.begin();
    while(it != m_arrModules.end()) {
        SNnVariable y;
        if( (*it)->eval(1, &x, y) != sCtx.success() ){
            return sCtx.error("计算错误");
        }
        x = y;
        it++;
    }
    spOutVar = x;
    return sCtx.success();
}

int CCompositeModule::toArchive(const SArchive& ar) {
    ar.arObjectArray("modules", m_arrModules);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CCompositeModule, CCompositeModule::__getClassKey())