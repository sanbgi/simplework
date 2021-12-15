#ifndef __SimpleWork_AV_CAvSampleMeta_H__
#define __SimpleWork_AV_CAvSampleMeta_H__

#include "av.h"
#include "EAvSampleType.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 音视频样本类型
//
struct CAvSampleMeta {

private:
    const static int getClassVer() { return 201214; }
    const int getVer() { return nVer; }
    int nVer;

public:
    CAvSampleMeta() {
        nVer = getClassVer();
        eSampleType = EAvSampleType::AvSampleType_None;
        nVideoWidth = 0;
        nVideoHeight = 0;
    }

    //
    // 样本类型
    //
    EAvSampleType eSampleType;

    union {
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
        int nAudioRate;

        //
        // 视频高度，只针对视频采样有效
        //
        int nVideoHeight;

    };

    union {
        //
        // 音频通道数
        //
        int nAudioChannels;
        
        //
        // 视频宽度，只针对视频采样有效
        //
        int nVideoWidth;
    };
};

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_CAvSampleMeta_H__