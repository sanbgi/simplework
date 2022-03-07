typedef struct {
    int batch;
    int height;
    int width;
    int layers;
}CBatchSize3D;

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
}CRect2D;

typedef struct{
    int batch;
    int height;
    int width;
}CBatchSize2D;

typedef struct {
    int nStrideWidth;
    int nStrideHeight;

    //网络参数
    int nLayers;
    int nInputLayers;

    //输入、输出、卷积尺寸
    CBatchSize3D sizeIn;
    CBatchSize3D sizeConv;
    CBatchSize3D sizeOut;

    //填充尺寸
    CRect2D padding;
    CRect2D completeOut;

    //输入、输出、卷积步长
    CBatchSize2D stepInMove;
    CBatchSize2D stepInConv;
    CBatchSize2D stepOut;
    CBatchSize2D stepConv;
}PConvParameter;

kernel void conv_eval(
    global PConvParameter* pThis, 
    int nBatchs,
    int nIn, global float * pIn,
    int nMat, global float * pWeights,
    int nOut, global float * pOut)
{
    CBatchSize3D sizeIn = pThis->sizeIn;
    CBatchSize3D sizeOut = pThis->sizeOut;
    CBatchSize3D sizeConv = pThis->sizeConv;
    CBatchSize2D stepInMove = pThis->stepInMove;
    CBatchSize2D stepInConv = pThis->stepInConv;
    CBatchSize2D stepOut = pThis->stepOut;
    CBatchSize2D stepConv = pThis->stepConv;

    int gid = get_global_id(0);
    int iBatch = gid / stepOut.batch;
    //int iBatch = get_global_id(0);
    //int gid = iBatch * get_global_size(1) + get_global_id(1);

    pIn = pIn + stepInConv.batch * iBatch;
    
    //计算坐标值
    int iHeight = (gid / stepOut.height) % sizeOut.height;
    int iWidth = (gid / stepOut.width) % sizeOut.width;
    int iLayer = gid % sizeOut.layers;

    //将输入指针向Padding后的起点偏移，变成一个非法指针
    CRect2D rcConv, rcPading = pThis->padding;
    int nOffsetIn = rcPading.left * stepInConv.width + rcPading.top * stepInConv.height;
    nOffsetIn = iHeight * stepInMove.height + iWidth * stepInMove.width - nOffsetIn;
    pIn += nOffsetIn;

    int nOffsetWeight = iLayer * stepConv.batch * sizeConv.batch;
    pWeights += nOffsetWeight;

    //上面填充了都填充了空值，不能参与运算
    if(iHeight < pThis->completeOut.top) {
        rcConv.top = rcPading.top - iHeight * pThis->nStrideHeight;
        rcConv.bottom = sizeConv.height;
        pIn += rcConv.top * stepInConv.height;
        pWeights += rcConv.top * stepConv.height;
    }else if(iHeight > pThis->completeOut.bottom) {
        rcConv.top = 0;
        rcConv.bottom = sizeConv.height + (sizeOut.height - 1 - iHeight) * pThis->nStrideHeight - rcPading.bottom;
    }else{
        rcConv.top = 0;
        rcConv.bottom = sizeConv.height;
    }

    //左边填充了都填充了空值，不能参与运算
    if(iWidth < pThis->completeOut.left) {
        rcConv.left = rcPading.left - iWidth * pThis->nStrideWidth;
        rcConv.right = sizeConv.width;
        pIn += rcConv.left * stepInConv.width;
        pWeights += rcConv.left * stepConv.width;
    }else if(iWidth > pThis->completeOut.right) {
        rcConv.left = 0;
        rcConv.right = sizeConv.width + (sizeOut.width - 1 - iWidth) * pThis->nStrideWidth - rcPading.right;
    }else{
        rcConv.left = 0;
        rcConv.right = sizeConv.width;
    }

    struct CItOutVariables {
        global float* pIn;
        global float* pWeights;
        int index;
    }itVars4, itVars5, itVars6, it = {
        pIn,
        pWeights
    };

    //初始化卷积结果
    float dConv = 0.0f;
    for(itVars4.index=rcConv.top; itVars4.index<rcConv.bottom; itVars4.index++) {
        itVars4.pIn = it.pIn;
        itVars4.pWeights = it.pWeights;
        for(itVars5.index=rcConv.left; itVars5.index<rcConv.right; itVars5.index++) {
            itVars5.pIn = it.pIn;
            itVars5.pWeights = it.pWeights;
            for(itVars6.index=0; itVars6.index<sizeConv.layers; itVars6.index++) {
                //卷积结果为乘积结果求和
                dConv += (*it.pWeights) * (*it.pIn);
                it.pIn++;
                it.pWeights++;
            }
            it.pIn = itVars5.pIn + stepInConv.width;
            it.pWeights = itVars5.pWeights + stepConv.width;
        }
        it.pIn = itVars4.pIn + stepInConv.height;
        it.pWeights = itVars4.pWeights + stepConv.height;
    }
    //卷积结果减去偏置
    pOut[gid] = dConv;// - (*it.pBais);
}

kernel void conv_devia(
    global PConvParameter* pThis, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia,
    int nMat, global float * pWeights, global float * pWeightsDevia,
    int nOut, global float * pOutDevia)
{
    CBatchSize3D sizeIn = pThis->sizeIn;
    CBatchSize3D sizeOut = pThis->sizeOut;
    CBatchSize3D sizeConv = pThis->sizeConv;
    CBatchSize2D stepInMove = pThis->stepInMove;
    CBatchSize2D stepInConv = pThis->stepInConv;
    CBatchSize2D stepOut = pThis->stepOut;
    CBatchSize2D stepConv = pThis->stepConv;

    int gid = get_global_id(0);
    int iBatch = gid / stepOut.batch;
    //int iBatch = get_global_id(0);
    //int gid = iBatch * get_global_size(1) + get_global_id(1);

    pIn = pIn + stepInConv.batch * iBatch;
    pInDevia = pInDevia + stepInConv.batch * iBatch;

    //计算坐标值
    int iHeight = (gid / stepOut.height) % sizeOut.height;
    int iWidth = (gid / stepOut.width) % sizeOut.width;
    int iLayer = gid % sizeOut.layers;

    //将输入指针向Padding后的起点偏移，变成一个非法指针
    CRect2D rcConv, rcPading = pThis->padding;
    int nOffsetIn = rcPading.left * stepInConv.width + rcPading.top * stepInConv.height;
    nOffsetIn = iHeight * stepInMove.height + iWidth * stepInMove.width - nOffsetIn;
    pIn += nOffsetIn;
    pInDevia += nOffsetIn;
    int nOffsetWeight = iLayer * stepConv.batch * sizeConv.batch;
    pWeights += nOffsetWeight;
    pWeightsDevia += nOffsetWeight;

    //上面填充了都填充了空值，不能参与运算
    if(iHeight < pThis->completeOut.top) {
        rcConv.top = rcPading.top - iHeight * pThis->nStrideHeight;
        rcConv.bottom = sizeConv.height;
        pIn += rcConv.top * stepInConv.height;
        pWeights += rcConv.top * stepConv.height;
    }else if(iHeight > pThis->completeOut.bottom) {
        rcConv.top = 0;
        rcConv.bottom = sizeConv.height + (sizeOut.height - 1 - iHeight) * pThis->nStrideHeight - rcPading.bottom;
    }else{
        rcConv.top = 0;
        rcConv.bottom = sizeConv.height;
    }

    //左边填充了都填充了空值，不能参与运算
    if(iWidth < pThis->completeOut.left) {
        rcConv.left = rcPading.left - iWidth * pThis->nStrideWidth;
        rcConv.right = sizeConv.width;
        pIn += rcConv.left * stepInConv.width;
        pWeights += rcConv.left * stepConv.width;
    }else if(iWidth > pThis->completeOut.right) {
        rcConv.left = 0;
        rcConv.right = sizeConv.width + (sizeOut.width - 1 - iWidth) * pThis->nStrideWidth - rcPading.right;
    }else{
        rcConv.left = 0;
        rcConv.right = sizeConv.width;
    }

    struct CItOutVariables {
        global float* pIn;
        global float* pInDeviation;
        global float* pWeights;
        global float* pWeightDevivation;
        int index;
    }it = {
        pIn,
        pInDevia,
        pWeights,
        pWeightsDevia,
    }, itVars4,itVars5,itVars6;
    union {
        unsigned int intVal;
        float floatVal;
    } newVal, prevVal;
    //
    //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
    //  
    float deviationZ = pOutDevia[gid];
    if(deviationZ > 1.0e-16 || deviationZ < -1.0e-16) {
        for(itVars4.index=rcConv.top; itVars4.index<rcConv.bottom; itVars4.index++) {
            itVars4.pIn = it.pIn;
            itVars4.pInDeviation = it.pInDeviation;
            itVars4.pWeights = it.pWeights;
            itVars4.pWeightDevivation = it.pWeightDevivation;
            for(itVars5.index=rcConv.left; itVars5.index<rcConv.right; itVars5.index++) {
                itVars5.pIn = it.pIn;
                itVars5.pInDeviation = it.pInDeviation;
                itVars5.pWeights = it.pWeights;
                itVars5.pWeightDevivation = it.pWeightDevivation;
                for(itVars6.index=0; itVars6.index<sizeConv.layers; itVars6.index++) {
                    //
                    // 累计计算权重值
                    //
                    //(*it.pInDeviation) += deviationZ * (*it.pWeights);
                    //(*it.pWeightDevivation) += deviationZ * (*it.pIn);
                    do {
                        prevVal.floatVal = (*it.pInDeviation);
                        newVal.floatVal = prevVal.floatVal + deviationZ * (*it.pWeights);
                    } while (atomic_cmpxchg((volatile __global unsigned int *)it.pInDeviation, 
                                            prevVal.intVal, newVal.intVal) 
                                            != prevVal.intVal);
                    do {
                        prevVal.floatVal = (*it.pWeightDevivation);
                        newVal.floatVal = prevVal.floatVal + deviationZ * (*it.pIn);
                    } while (atomic_cmpxchg((volatile __global unsigned int *)it.pWeightDevivation, 
                                            prevVal.intVal, newVal.intVal) 
                                            != prevVal.intVal);

                    it.pIn++;
                    it.pInDeviation++;
                    it.pWeights++;
                    it.pWeightDevivation++;
                }
                it.pIn = itVars5.pIn + stepInConv.width;
                it.pInDeviation = itVars5.pInDeviation + stepInConv.width;
                it.pWeights = itVars5.pWeights + stepConv.width;
                it.pWeightDevivation = itVars5.pWeightDevivation + stepConv.width;
            }

            it.pIn = itVars4.pIn + stepInConv.height;
            it.pInDeviation = itVars4.pInDeviation + stepInConv.height;
            it.pWeights = itVars4.pWeights + stepConv.height;
            it.pWeightDevivation = itVars4.pWeightDevivation + stepConv.height;
        }
    }
}
