
#include "module.h"
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
static SCtx sCtx("CDenseModule");
class CDenseModule : public CObject, public INnModule, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnModule)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "DenseModule"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

private://INnModule
    int eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.DenseModule"; }
    static int createModule(const PData& rData, SNnModule& spModule);

private:
    //基础参数
    int m_nCells;
    double m_dDropoutRate;
    string m_strActivator;

public:
    CDenseModule() {
        m_nCells = 0;
    }
};

int CDenseModule::__initialize(const PData* pData){
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

int CDenseModule::createModule(const PData& rData, SNnModule& spModule) {
    const PNnDense* pDense = CData<PNnDense>(rData);
    if(pDense == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    CPointer<CDenseModule> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = pDense->nCells;
    spDense->m_dDropoutRate = 0;
    if( pDense->szActivator!=nullptr )
        spDense->m_strActivator = pDense->szActivator;
    spModule.setPtr(spDense.getPtr());
    return sCtx.success();
}

int CDenseModule::eval(int nInVars, const SNnVariable spInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("全连接网络输入参数必须为一个");
    }
    spOutVar = spInVars[0].dense({m_nCells, m_strActivator.c_str()});
    return sCtx.success();
}

int CDenseModule::toArchive(const SArchive& ar) {
    //基础参数
    ar.arBlock("cells", m_nCells);
    ar.arBlock("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseModule, CDenseModule::__getClassKey())