#include "module.h"
#include <string>

using namespace sw;
using namespace std;
static SCtx sCtx("CGruModule");
class CGruModule : public CObject, public INnModule, public IArchivable{

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
    const char* getClassName() { return "GruModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.GruModule"; }

private:
    //基础参数
    int m_nCells;

public:
    CGruModule() {
    }
};

int CGruModule::__initialize(const PData* pData) {
    const PNnRnn* pRnn = CData<PNnRnn>(pData);
    if(pRnn == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_nCells = pRnn->nCells;
    return sCtx.success();
}

int CGruModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("Rnn单元输入参数必须为一个");
    }

    spOutVar = spInVars[0].gru({m_nCells});
    return sCtx.success();
}

int CGruModule::toArchive(const SArchive& ar) {
    //基础参数
    ar.arBlock("cells", m_nCells);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGruModule, CGruModule::__getClassKey())