
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

        int getStreamingId() {
            return m_iStreamingId;
        }

        EAvStreamingType getStreamingType() {
            return m_streamingType;
        }

        PAvSample getSampleMeta() {
            return m_sampleMeta;
        }

        STensor& getData() {
            return m_spData;
        }

        long getTimeStamp() {
            return m_nTimeStamp;
        }

        int readPAvFrame(PAvFrame::FVisitor visitor) {
            unsigned char* data[8];
            int linesize[8];
            data[0] = (unsigned char*)m_spData->getDataPtr<unsigned char*>();
            
            PAvFrame frame;
            frame.sampleMeta = m_sampleMeta;
            frame.streamingId = m_iStreamingId;
            frame.streamingType = m_streamingType;
            frame.timeStamp = m_nTimeStamp;
            frame.planeDatas = data;
            return SError::ERRORTYPE_SUCCESS;
        }

    public:
        EAvStreamingType m_streamingType;
        int m_iStreamingId;
        PAvSample m_sampleMeta;
        long m_nTimeStamp;
        STensor m_spData;
    };

public:
    SAvFrame createFrame(const PAvFrame& avFrame) {

        STensor spTensor;
        switch(avFrame.streamingType) {
        case EAvStreamingType::AvStreamingType_Video:
            {
                int nPixBytes = 0;
                PAvSample sampleMeta = avFrame.sampleMeta;
                switch(sampleMeta.sampleType) {
                case EAvSampleType::AvSampleType_Video_RGBA:
                    nPixBytes = 4;
                    break;

                case EAvSampleType::AvSampleType_Video_RGB:
                    nPixBytes = 3;
                    break;

                default:
                    return STensor();
                }

                int dimsize[3] = { sampleMeta.videoHeight, sampleMeta.videoWidth, nPixBytes };
                STensor spDimTensor = STensor::createVector(3, dimsize);

                //
                //  修正视频宽度。及nTargetWidth != m_pVideoLinesizes[0]/4时，以后者为准。如果返
                //  回值中的视频，一行的字节数不等于视频宽度*4(RGBA)，则这个时候视频数据有点难处
                //  理，只能暂时将返回图像的宽度扩大(有垃圾数据)或缩小(数据丢失）到返回的实际宽度。
                //  如果想调整为实际视频大小，则需要对每一行的数据做处理，性能太低。
                //
                spTensor = STensor::createTensor(
                                        spDimTensor, 
                                        sampleMeta.videoWidth*sampleMeta.videoHeight*nPixBytes,
                                        avFrame.planeDatas[0]);
            }
            break;

        case EAvStreamingType::AvStreamingType_Audio:
            {
                int nBytesPerSample = 0;
                PAvSample sampleMeta = avFrame.sampleMeta;
                switch(sampleMeta.sampleType) {
                case EAvSampleType::AvSampleType_Audio_U8:
                    nBytesPerSample = 1;
                    break;

                case EAvSampleType::AvSampleType_Audio_S16:
                    nBytesPerSample = 2;
                    break;

                default:
                    return STensor();
                }

                // 重采样返回的一帧音频数据大小(以字节为单位)
                int nSamples = avFrame.samples;
                int dimsize[3] = { sampleMeta.audioChannels, nSamples, nBytesPerSample };
                STensor spDimTensor = STensor::createVector(3, dimsize);

                int nData = sampleMeta.audioChannels * nSamples * nBytesPerSample;
                spTensor = STensor::createTensor(spDimTensor, nData, avFrame.planeDatas[0]);              
            }
            break;

        default:
            return STensor();
        }

        SObject spObject;
        CAvFrame* pFrame = CObject::createObject<CAvFrame>(spObject);
        pFrame->m_iStreamingId = avFrame.streamingId;
        pFrame->m_nTimeStamp = avFrame.timeStamp;
        pFrame->m_sampleMeta = avFrame.sampleMeta;
        pFrame->m_streamingType = avFrame.streamingType;
        pFrame->m_spData = spTensor;
        return SAvFrame::wrapPtr(pFrame);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvFrameFactory, SAvFrame::SAvFrameFactory::getClassKey())