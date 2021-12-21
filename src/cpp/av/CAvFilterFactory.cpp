
#include "av.h"
#include "ffmpeg/CAvFrameConverter.h"


class CAvFrameConverterFactory : public CObject, public SAvFrameConverter::IAvFrameConverterFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvFrameConverter::IAvFrameConverterFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createFilter(const PAvSample& targetSample, SAvFrameConverter& spFilter) {
        return ffmpeg::CAvFrameConverter::createFilter(targetSample, spFilter);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFrameConverterFactory, SAvFrameConverter::SAvFrameConverterFactory::getClassKey())