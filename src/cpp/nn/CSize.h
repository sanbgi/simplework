#ifndef __SimpleWork_NN_CStepSize_H__
#define __SimpleWork_NN_CStepSize_H__

struct CBatchSize2D
{
    int batch;
    int height;
    int width;
};

struct CBatchSize3D
{
    int batch;
    int height;
    int width;
    int layers;
};

struct CRect {
    int left;
    int right;
    int top;
    int bottom;
};

#endif//__SimpleWork_NN_CStepSize_H__
