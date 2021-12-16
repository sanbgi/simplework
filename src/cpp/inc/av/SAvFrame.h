#ifndef __SimpleWork_IO_AvFrame_h__
#define __SimpleWork_IO_AvFrame_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 采样数据
//      
//
struct SAvFrameData {

    //
    // 所属的流对应的ID和类型
    //
    int streamingId;
    enum EAvStreamingType {
        AvStreamingType_None,
        AvStreamingType_Video,
        AvStreamingType_Audio,
    } streamingType;

    //
    // 时钟频率和对应的时间戳，比如：时钟频率是1000，时间戳是10，则表示10/1000秒
    //
    int timeRate;
    long timeStamp;


    //
    // 样本数据类型
    //
    enum EAvSampleType {
        AvSampleType_None,
        AvSampleType_Video_RGB,
        AvSampleType_Video_RGBA,
        AvSampleType_Audio_U8,
        AvSampleType_Audio_S16,
    };

    //
    // 样本描述类型
    //
    struct SAvSampleMeta {

        //
        // 样本类型
        //
        EAvSampleType sampleType;

        //
        // 音频采样率，每秒样本（采样点）数，常见值：
        //           8,000 -- 电话
        //          11,025 -- AM调幅广播
        //          22,050 -- FM调频广播
        //          24,000 -- FM
        //          32,000 -- 数码视频
        //          44,100 -- CD音质，常用于 MPEG-1 音频（VCD, SVCD, MP3）所用采样率
        //          47,250 -- 商用 PCM 录音机所用采样率
        //          48,000 -- miniDV、数字电视、DVD、DAT、电影和专业音频所用的数字声音所用采样率
        //          50,000 -- 商用数字录音机所用采样率
        //          等等
        //
        int audioRate;
        int audioChannels;

        //
        // 视频尺寸，宽度、高度
        //
        int videoHeight;
        int videoWidth;
    }& sampleMeta;

    //
    // 样本数据包含多少数据面
    //
    int samplePlanes;

    //
    // 指针数组，其长度为和samplePlanes一致，其每一项内容为指向数据面的指针
    //
    unsigned char** sampleData;

    //
    // 数据长度数组，其长度为和samplePlanes一致，单位为字节数
    //
    int* sampleSize;
};
typedef SAvFrameData::EAvSampleType EAvSampleType;
typedef SAvFrameData::SAvSampleMeta SAvSampleMeta;
typedef SAvFrameData::EAvStreamingType EAvStreamingType;
class SAvStreaming;

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)
public:
    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 211206)
        //
        // 获取流
        //
        virtual SAvStreaming& getStreaming() = 0;

        //
        // 获取时间戳，单位是SAvStreaming::getTimeRate对应的时间单位
        //
        virtual long getTimeStamp() = 0;

        //
        // 获取当前帧的数据, 张量数据格式为
        //     视频：
        //          张量维度：height(行) * width(列) * bytes (单像素字节数)
        //          张量类型：unsigned char
        //     音频：
        //          张量维度：采样次数 * 通道数 * bytes (单采样点字节数)
        //          张量类型：unsigned char
        //
        virtual sw::math::STensor& getData() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(AvFrameFactory, "sw.av.AvFrameFactory")
        SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrameFactory", 211214)
            //
            // 创建流对象
            //
            virtual SAvFrame createFrame(
                                SAvStreaming& rStreaming,
                                long nTimeStamp,
                                sw::math::STensor& spData ) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrameFactory)

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_IO_AvFrame_h__