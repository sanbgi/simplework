
#include "unit.h"
#include <string>

using namespace sw;
using namespace std;


//
//  全连接神经网络，注意数据格式：
//
//      输入数据：nBatchs(第一个维度) * nCells(余下所有维度)
//      输出数据：nBatchs(第一个维度) * nCells(第二个维度)
//
//  比如：输入维度[10, 3, 1]，表示一共有十个输入数据，每一个数据数据包含3 X 1个输入神经元
//
static SCtx sCtx("CDenseUnit");
class CDenseUnit : public CObject, public INnUnit, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnUnit)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "DenseUnit"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnUnit
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseUnit"; }
    static int createUnit(const PData& rData, SNnUnit& spUnit);

private:
    //基础参数
    int m_nCells;
    double m_dDropoutRate;
    string m_strActivator;
    SNnVariable m_spWeights;
    SNnVariable m_spBais;

public:
    CDenseUnit() {
        m_nCells = 0;
    }
};

int CDenseUnit::__initialize(const PData* pData){
    const PNnDense* pDense = CData<PNnDense>(pData);
    if(pDense == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_nCells = pDense->nCells;
    m_dDropoutRate = 0;
    if( pDense->szActivator!=nullptr )
        m_strActivator = pDense->szActivator;
    return sCtx.success();
}

int CDenseUnit::createUnit(const PData& rData, SNnUnit& spUnit) {
    const PNnDense* pDense = CData<PNnDense>(rData);
    if(pDense == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    CPointer<CDenseUnit> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = pDense->nCells;
    spDense->m_dDropoutRate = 0;
    if( pDense->szActivator!=nullptr )
        spDense->m_strActivator = pDense->szActivator;
    spUnit.setPtr(spDense.getPtr());
    return sCtx.success();
}

int CDenseUnit::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }

    if(!m_spWeights) {
        SDimension spDim = spInVars[0].dimension();

        int nInputCells = spDim->getElementSize();
        int pWeightDimSizes[2] = {m_nCells, nInputCells};
        m_spWeights = SNnVariable::createWeight({SDimension(2, pWeightDimSizes), 1.0f/nInputCells});
        m_spBais = SNnVariable::createWeight({SDimension(1, &m_nCells),0});
        if( !m_spWeights || !m_spBais ) {
            return sCtx.error("权重变量创建失败");
        }
    }

    SNnVariable x = spInVars[0];
    SNnVariable y = SNnVariable::product(x, m_spWeights) + m_spBais;
    if(m_strActivator.length() > 0) {
        spOutVar = y.solveOp(m_strActivator.c_str());
    }else{
        spOutVar = y.solveOp("relu");
    }
    return sCtx.success();
}

int CDenseUnit::toArchive(const SArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visit("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    ar.visitObject("weight", m_spWeights);
    ar.visitObject("bais", m_spBais);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseUnit, CDenseUnit::__getClassKey())