
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
    SNnVariable m_spWeights;
    SNnVariable m_spBais;

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

    if(!m_spWeights) {
        SDimension spDim = spInVars[0].dimension();
        if(spDim.size() < 1) {
            return sCtx.error("线性变化输入的数据，维度不能小于一");
        }

        int pWeightDimSizes[2] = {m_nCells, spDim.data()[spDim.size()-1]};
        m_spWeights = SNnVariable::createWeight({SDimension(2, pWeightDimSizes),0});
        if( !m_spWeights ) {
            return sCtx.error("权重变量创建失败");
        }

        if(m_bBais) {
            m_spBais = SNnVariable::createWeight( {SDimension(1, &m_nCells), 0});
            if( !m_spBais ) {
                return sCtx.error("偏置创建失败");
            }
        }
    }

    SNnVariable x = spInVars[0];
    SNnVariable y = SNnVariable::product(x, m_spWeights);
    if(m_bBais)
        y = y + m_spBais;
    if(m_strActivator.length() > 0) {
        spOutVar = y.solveOp(m_strActivator.c_str());
    }else{
        spOutVar = y;
    }
    return sCtx.success();
}

int CLinearUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.arBlock("cells", m_nCells);
    ar.arBlock("usebais", m_bBais);
    ar.arBlock("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    ar.arObject("weight", m_spWeights);
    ar.arObject("bais", m_spBais);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLinearUnit, CLinearUnit::__getClassKey())