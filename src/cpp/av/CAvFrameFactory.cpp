
#include "av.h"

using namespace sw::core;
using namespace sw::av;
using namespace sw::math;

class CAvFrameFactory : public CObject, public SAvFrame::IAvFrameFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvFrame::IAvFrameFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    class CAvFrame : public CObject, public IAvFrame {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
            SIMPLEWORK_INTERFACE_ENTRY(IAvFrame)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    public:
        SAvStreaming& getStreaming() {
            return m_spStreaming;
        }

        STensor& getData() {
            return m_spData;
        }

        long getTimeStamp() {
            return m_nTimeStamp;
        }

    public:
        SAvStreaming m_spStreaming;
        long m_nTimeStamp;
        STensor m_spData;
    };

public:
    SAvFrame createFrame(SAvStreaming& rStreaming, long nTimeStamp, STensor& spData) {
        SObject spObject;
        CAvFrame* pFrame = CObject::createObject<CAvFrame>(spObject);
        pFrame->m_spStreaming = rStreaming;
        pFrame->m_spData = spData;
        pFrame->m_nTimeStamp = nTimeStamp;
        return SAvFrame::wrapPtr(pFrame);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFrameFactory, SAvFrame::SAvFrameFactory::getClassKey())