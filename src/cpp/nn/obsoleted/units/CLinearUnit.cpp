
#include "unit.h"
#include <string>

using namespace sw;
using namespace std;


//
//  线性变换神经网络，注意数据格式：
//
//
static SCtx sCtx("CLinearUnit");
class CLinearUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "LinearUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.LinearUnit"; }
    static int createUnit(const PData& rData, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    int m_bBais;
    double m_dDropoutRate;
    string m_strActivator;

public:
    CLinearUnit() {
        m_nCells = 0;
    }
};

int CLinearUnit::__initialize(const PData* pData){
    const PNnLinear* pLinear = CData<PNnLinear>(pData);
    if(pLinear == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_nCells = pLinear->nCells;
    m_dDropoutRate = 0;
    if( pLinear->szActivator!=nullptr )
        m_strActivator = pLinear->szActivator;
    return sCtx.success();
}

int CLinearUnit::createUnit(const PData& rData, SNnUnit& spUnit) {
    const PNnLinear* pLinear = CData<PNnLinear>(rData);
    if(pLinear == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    CPointer<CLinearUnit> spLinear;
    CObject::createObject(spLinear);
    spLinear->m_nCells = pLinear->nCells;
    spLinear->m_dDropoutRate = 0;
    if( pLinear->szActivator!=nullptr )
        spLinear->m_strActivator = pLinear->szActivator;
    spUnit.setPtr(spLinear.getPtr());
    return sCtx.success();
}

int CLinearUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }

    spOutVar = spInVars[0].linear({m_nCells, m_bBais, m_strActivator.c_str()});
    return sCtx.success();
}

int CLinearUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.arBlock("cells", m_nCells);
    ar.arBlock("usebais", m_bBais);
    ar.arBlock("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLinearUnit, CLinearUnit::__getClassKey())