#ifndef __SimpleWork_AV_PAvSample_H__
#define __SimpleWork_AV_PAvSample_H__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 样本信息，对应于视频就是帧数据格式信息，音频的采样点格式信息。详细的描述可以参考PAvFrame说明
//
struct PAvSample {

    enum EAvSampleType {
        AvSampleType_None,
        AvSampleType_Video,
        AvSampleType_Audio,
    } sampleType;

    //
    // 样本类型
    //
    enum EAvSampleFormat {
        AvSampleFormat_None         = 0x00F00001,
        AvSampleFormat_Video_RGB    = 0x00F10001,
        AvSampleFormat_Video_RGBA   = 0x00F10002,
        AvSampleFormat_Audio_U8     = 0x00F20001,    
        AvSampleFormat_Audio_S16    = 0x00F20002,
    } sampleFormat;

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
};
typedef PAvSample::EAvSampleFormat EAvSampleFormat;
typedef PAvSample::EAvSampleType EAvSampleType;


SIMPLEWORK_AV_NAMESPACE_LEAVE


#endif//__SimpleWork_AV_PAvSample_H__