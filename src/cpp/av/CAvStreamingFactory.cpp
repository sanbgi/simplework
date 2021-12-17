
#include "av.h"

using namespace sw::core;
using namespace sw::av;

class CAvStreamingFactory : public CObject, public SAvStreaming::IAvStreamingFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvStreaming::IAvStreamingFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    class CAvStreaming : public CObject, public IAvStreaming {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
            SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    public:
        int getStreamingId() { return m_iStreamingId; }
        int getTimeRate() { return m_nTimeRate; }
        long getDuration() { return m_nDuration; }
        EAvSampleType getSampleType() { return m_eStreamingType; }
        const PAvSample& getSampleMeta() { return m_sampleMeta; }

    public:
        EAvSampleType m_eStreamingType;
        int m_iStreamingId;
        int m_nTimeRate;
        long m_nDuration;
        PAvSample m_sampleMeta;
    };

public:
    SAvStreaming createStreaming(  
                            int iStreamingId,
                            int nTimeRate,
                            EAvSampleType eStreamingType, 
                            const PAvSample& sampleMeta ) {
        SObject spObject;
        CAvStreaming* pStreaming = CObject::createObject<CAvStreaming>(spObject);
        pStreaming->m_eStreamingType = eStreamingType;
        pStreaming->m_iStreamingId = iStreamingId;
        pStreaming->m_nTimeRate = nTimeRate;
        pStreaming->m_sampleMeta = sampleMeta;
        return SAvStreaming::wrapPtr(pStreaming);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvStreamingFactory, SAvStreaming::SAvStreamingFactory::getClassKey())