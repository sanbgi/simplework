#include "network.h"
#include "vector"
static SCtx sCtx("CCompositeNetwork");

class CCompositeNetwork : public CObject, public INnNetwork, public IArchivable{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnNetwork)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://INnNetwork
    int eval(const STensor& spBatchIn, STensor& spBatchOut);
    int devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation);
    int update(const STensor& spBatchInDeviation);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "CompositeNetwork"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.CompositeNetwork"; }

public:
    std::vector<SNnNetwork> m_arrNetworks;
};

int CCompositeNetwork::__initialize(const PData* pData){
    const SNnNetwork::PNnCompositeNetwork* pNet = CData<SNnNetwork::PNnCompositeNetwork>(pData);
    if(pNet == nullptr) {
        return sCtx.error("缺少构造参数");
    }

    for(int i=0; i<pNet->nNetworks; i++) {
        m_arrNetworks.push_back(pNet->pNetworks[i]);
    }
    return sCtx.success();
}

int CCompositeNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    STensor spIn = spBatchIn;
    std::vector<SNnNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor spOut;
        if( int errCode = (*it)->eval(spIn, spOut) != sCtx.success() ){
            return errCode;
        }
        spIn = spOut;
        it++;
    }
    spBatchOut = spIn;
    return sCtx.success();
}

int CCompositeNetwork::devia(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {
    STensor spOut = spBatchOut;
    STensor spOutDev = spOutDeviation;
    std::vector<SObject> arrDevias;
    std::vector<SNnNetwork>::reverse_iterator it = m_arrNetworks.rbegin();
    while(it != m_arrNetworks.rend() ) {
        STensor spIn;
        STensor spInDev;
        if( int errCode = (*it)->devia(spOut, spOutDev, spIn, spInDev) != sCtx.success() ) {
            return errCode;
        }
        arrDevias.push_back(spInDev);
        spOut = spIn;
        spOutDev = spInDev;
        it++;
    }
    spBatchIn = spOut;

    STensor spNetDevias;
    if( STensor::createVector(spNetDevias, (int)arrDevias.size(), arrDevias.data()) != sCtx.success() ) {
        return sCtx.error("创建保存偏差的对象张量失败");
    }
    return CNnResizeTensor::createResizeTensor({spOutDev, spNetDevias}, spInDeviation);
}

int CCompositeNetwork::update(const STensor& spBatchInDeviation) {
    SNnResizeTensor spResizeDevia = spBatchInDeviation;
    if( !spResizeDevia ) {
        return sCtx.error("非有效的输出，无法用于学习");
    }

    PNnResizeTensor sResizeTensor;
    spResizeDevia->getResizeData(sResizeTensor);
    STensor spDevias = sResizeTensor.spExtra1;
    
    int nDevias = spDevias.size();
    SObject * pDevias = spDevias.data<SObject>();
    if(nDevias != m_arrNetworks.size() || pDevias == nullptr) {
        return sCtx.error("无效的偏差张量");
    }

    std::vector<SNnNetwork>::reverse_iterator it = m_arrNetworks.rbegin();
    while(it != m_arrNetworks.rend() ) {
        if( int errCode = (*it)->update(*pDevias) != sCtx.success() ) {
            return errCode;
        }
        it++, pDevias++;
    }
    return sCtx.success();
}

int CCompositeNetwork::toArchive(const SArchive& ar) {
    ar.arObjectArray("nodes", m_arrNetworks);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CCompositeNetwork, CCompositeNetwork::__getClassKey())