
#include "av.h"
#include "ffmpeg/CAvFilter.h"


class CAvFilterFactory : public CObject, public SAvFilter::IAvFilterFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvFilter::IAvFilterFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createFilter(const PAvSample& targetSample, SAvFilter& spFilter) {
        return ffmpeg::CAvFilter::createFilter(targetSample, spFilter);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFilterFactory, SAvFilter::SAvFilterFactory::getClassKey())