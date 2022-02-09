
#include "nn.h"
#include "SNnEvalOutTensor.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnEvalOutTensor");
class CNnEvalOutTensor : public CObject, public ITensor, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(ITensor)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData){
        const PNnEvalOutTensor* pInitializer = CData<PNnEvalOutTensor>(pData);
        if(pInitializer == nullptr) {
            return sCtx.error("缺少初始化参数");
        }
        m_spBatchIn = pInitializer->spBatchIn;
        m_spBatchOp = pInitializer->spBatchOp;
        m_spBatchOutDimension = pInitializer->spBatchOutDimension;
        m_nBatchOutOffset = pInitializer->nBatchOutOffset;
        return sCtx.success();
    }

public://ITensor
    int getVer() {
        return m_spBatchOp.ver();
    }

    int updateVer() {
        return m_spBatchOp.updateVer();
    }

    int getDimension(SDimension& spDim) {
        spDim = m_spBatchOutDimension;
        return sCtx.success();
    }

    unsigned int getDataType(){
        return m_spBatchOp.type();
    }

    int getDataSize() {
        return m_spBatchOutDimension.dataSize();
    }

    void* getDataPtr(unsigned int eElementType, int iPos=0) {
        return m_spBatchOp->getDataPtr(iPos+m_nBatchOutOffset);
    }

private://IArchivable
    int getClassVer() { return 220209; }
    const char* getClassName() { return "EvalOutTensor"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arObject("batchin", m_spBatchIn);
        ar.arObject("batchop", m_spBatchOp);
        ar.arObject("batchoutdim", m_spBatchOutDimension);
        ar.arBlock("batchoutoffset", m_nBatchOutOffset);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.EvalOutTensor"; }

private:
    STensor m_spBatchIn;
    STensor m_spBatchOp;
    SDimension m_spBatchOutDimension;
    int m_nBatchOutOffset;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnEvalOutTensor, CNnEvalOutTensor::__getClassKey())