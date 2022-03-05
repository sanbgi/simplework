typedef struct {
    int left;
    int right;
    int top;
    int bottom;
}CRect2D;

typedef struct{
    int m_nMaxPoolWidth;
    int m_nMaxPoolHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;

    //填充尺寸
    CRect2D m_padding;

    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayer;
    int m_nInputTensorSize;

    int m_nOutWidth;
    int m_nOutHeight;
    int m_nOutTensorSize;
}PMaxpoolParameter;

kernel void maxpool_eval(
    global PMaxpoolParameter* pThis, 
    int nBatchs,
    int nIn, global float * pIn,
    int nOut, global float * pOut)
{
    int nMaxPoolWidth = pThis->m_nMaxPoolWidth;
    int nMaxPoolHeight = pThis->m_nMaxPoolHeight;
    int nOutWidth = pThis->m_nOutWidth;
    int nOutHeight = pThis->m_nOutHeight;
    int nLayer = pThis->m_nInputLayer;

    int nInputHstep = pThis->m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
    int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;

    int nOutHstep = pThis->m_nOutWidth * nLayer;
    int nOutWstep = nLayer;

    int gid = get_global_id(0);
    int iBatch = gid / pThis->m_nOutTensorSize;
    pIn = pIn + pThis->m_nInputTensorSize * iBatch;

    //计算坐标值
    int iOutY = (gid / nOutHstep) % nOutHeight;
    int iOutX = (gid / nOutWstep) % nOutWidth;
    int iLayer = gid % nLayer;

    struct CItOutVariables {
        global float* pIn;
    }it = {
        pIn,
    }, varConvYBackup;

    //
    //  注意，关于填充问题，算法相对比较抽象
    //      
    //      首先将数据矩阵首先按照填充规则，进行四周填充，然后，将输入指针指向填充后的最左上角，
    //  此时，指针实际上是一个非法指针。在遍历输出矩阵宽度高度时，如果发现其实位置小于填充尺寸
    //  则，卷积矩阵调整到有效的开始位置，而输入矩阵指针也同时指向有效的地址，与卷积矩阵开始位
    //  置一致。
    //
    CRect2D rcMaxPool, rcPading = pThis->m_padding;
    int nOffset = rcPading.left * nInputWstep + rcPading.top * nInputHstep;
    nOffset = iOutY * nInStrideHstep + iOutX * nInStrideWstep + iLayer - nOffset;
    it.pIn += nOffset;

    int nStrideHeight = pThis->m_nStrideHeight;
    int iMinCompleteHeight = (rcPading.top + nStrideHeight - 1) / nStrideHeight;
    int iMaxCompleteHeight = nOutHeight - 1 - (rcPading.bottom + nStrideHeight - 1) / nStrideHeight;
    //
    //  输出矩阵能够完整卷积的最大下标，再往右，则需要剪裁了
    //
    int nStrideWidth = pThis->m_nStrideWidth;
    int iMinCompleteWidth = (rcPading.left + nStrideWidth - 1) / nStrideWidth;
    int iMaxCompleteWidth = nOutWidth - 1 - (rcPading.right + nStrideWidth - 1) / nStrideWidth;

    //上面填充了都填充了空值，不能参与运算
    if(iOutY < iMinCompleteHeight) {
        //
        // 卷积核顶部裁剪，相当于将起始坐标下移，同时
        //      1,  输入矩阵的起始坐标也需要下移
        //      2,  权重的其实位置也许要同步下移到实际的其实位置
        //
        rcMaxPool.top = rcPading.top - iOutY * nStrideHeight;
        rcMaxPool.bottom = nMaxPoolHeight;
        it.pIn += rcMaxPool.top * nInputHstep;
    }else if(iOutY > iMaxCompleteHeight) {
        rcMaxPool.top = 0;
        rcMaxPool.bottom = nMaxPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
    }else{
        rcMaxPool.top = 0;
        rcMaxPool.bottom = nMaxPoolHeight;
    }

    //左边填充了都填充了空值，不能参与运算
    if(iOutX < iMinCompleteWidth) {
        //
        // 卷积核左部裁剪，相当于将起始坐标右移，同时
        //      1，输入矩阵的起始坐标也需要右移
        //      2，权重矩阵其实坐标也需要右移到对应的开始位置
        //
        rcMaxPool.left = rcPading.left - iOutX * nStrideWidth;
        rcMaxPool.right = nMaxPoolWidth;
        it.pIn += rcMaxPool.left * nInputWstep;
    }else if(iOutX > iMaxCompleteWidth) {
        rcMaxPool.left = 0;
        rcMaxPool.right = nMaxPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
    }else{
        rcMaxPool.left = 0;
        rcMaxPool.right = nMaxPoolWidth;
    }
 
    float dMax = (*it.pIn);
    for( int iMaxPoolY=rcMaxPool.top; iMaxPoolY<rcMaxPool.bottom; iMaxPoolY++) {
        varConvYBackup.pIn = it.pIn;
        for( int iMaxPoolX=rcMaxPool.left; iMaxPoolX<rcMaxPool.right; iMaxPoolX++) {
            if( (*it.pIn) > dMax) {
                dMax = (*it.pIn);
            }
            it.pIn += nInputWstep;
        }
        it.pIn = varConvYBackup.pIn + nInputHstep;
    }
    pOut[gid] = dMax;
}

kernel void maxpool_devia(
    global PMaxpoolParameter* pThis, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia,
    int nOut, global float * pOutDevia)
{
    int nMaxPoolWidth = pThis->m_nMaxPoolWidth;
    int nMaxPoolHeight = pThis->m_nMaxPoolHeight;
    int nOutWidth = pThis->m_nOutWidth;
    int nOutHeight = pThis->m_nOutHeight;
    int nLayer = pThis->m_nInputLayer;

    int nInputHstep = pThis->m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
    int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;

    int nOutHstep = pThis->m_nOutWidth * nLayer;
    int nOutWstep = nLayer;

    int gid = get_global_id(0);
    int iBatch = gid / pThis->m_nOutTensorSize;
    pIn = pIn + pThis->m_nInputTensorSize * iBatch;
    pInDevia = pInDevia + pThis->m_nInputTensorSize * iBatch;

    //计算坐标值
    int iOutY = (gid / nOutHstep) % nOutHeight;
    int iOutX = (gid / nOutWstep) % nOutWidth;
    int iLayer = gid % nLayer;

    struct CItOutVariables {
        global float* pIn;
        global float* pInDeviation;
    }it = {
        pIn,
        pInDevia
    }, varConvYBackup;

    //
    //  注意，关于填充问题，算法相对比较抽象
    //      
    //      首先将数据矩阵首先按照填充规则，进行四周填充，然后，将输入指针指向填充后的最左上角，
    //  此时，指针实际上是一个非法指针。在遍历输出矩阵宽度高度时，如果发现其实位置小于填充尺寸
    //  则，卷积矩阵调整到有效的开始位置，而输入矩阵指针也同时指向有效的地址，与卷积矩阵开始位
    //  置一致。
    //
    CRect2D rcMaxPool, rcPading = pThis->m_padding;
    int nOffset = rcPading.left * nInputWstep + rcPading.top * nInputHstep;
    nOffset = iOutY * nInStrideHstep + iOutX * nInStrideWstep + iLayer - nOffset;
    it.pIn += nOffset;
    it.pInDeviation += nOffset;

    int nStrideHeight = pThis->m_nStrideHeight;
    int iMinCompleteHeight = (rcPading.top + nStrideHeight - 1) / nStrideHeight;
    int iMaxCompleteHeight = nOutHeight - 1 - (rcPading.bottom + nStrideHeight - 1) / nStrideHeight;
    //
    //  输出矩阵能够完整卷积的最大下标，再往右，则需要剪裁了
    //
    int nStrideWidth = pThis->m_nStrideWidth;
    int iMinCompleteWidth = (rcPading.left + nStrideWidth - 1) / nStrideWidth;
    int iMaxCompleteWidth = nOutWidth - 1 - (rcPading.right + nStrideWidth - 1) / nStrideWidth;

    //上面填充了都填充了空值，不能参与运算
    if(iOutY < iMinCompleteHeight) {
        //
        // 卷积核顶部裁剪，相当于将起始坐标下移，同时
        //      1,  输入矩阵的起始坐标也需要下移
        //      2,  权重的其实位置也许要同步下移到实际的其实位置
        //
        rcMaxPool.top = rcPading.top - iOutY * nStrideHeight;
        rcMaxPool.bottom = nMaxPoolHeight;
        it.pIn += rcMaxPool.top * nInputHstep;
        it.pInDeviation += rcMaxPool.top * nInputHstep;
    }else if(iOutY > iMaxCompleteHeight) {
        rcMaxPool.top = 0;
        rcMaxPool.bottom = nMaxPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
    }else{
        rcMaxPool.top = 0;
        rcMaxPool.bottom = nMaxPoolHeight;
    }

    //左边填充了都填充了空值，不能参与运算
    if(iOutX < iMinCompleteWidth) {
        //
        // 卷积核左部裁剪，相当于将起始坐标右移，同时
        //      1，输入矩阵的起始坐标也需要右移
        //      2，权重矩阵其实坐标也需要右移到对应的开始位置
        //
        rcMaxPool.left = rcPading.left - iOutX * nStrideWidth;
        rcMaxPool.right = nMaxPoolWidth;
        it.pIn += rcMaxPool.left * nInputWstep;
        it.pInDeviation += rcMaxPool.left * nInputWstep;
    }else if(iOutX > iMaxCompleteWidth) {
        rcMaxPool.left = 0;
        rcMaxPool.right = nMaxPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
    }else{
        rcMaxPool.left = 0;
        rcMaxPool.right = nMaxPoolWidth;
    }

    float dMax = (*it.pIn);
    global float* pExpectDelta = it.pInDeviation;

    //
    //  从输入中找到最大的那个点，作为反向传到的点
    //  
    for( int iMaxPoolY=rcMaxPool.top; iMaxPoolY<rcMaxPool.bottom; iMaxPoolY++) {
        varConvYBackup.pIn = it.pIn;
        varConvYBackup.pInDeviation = it.pInDeviation;
        for( int iMaxPoolX=rcMaxPool.left; iMaxPoolX<rcMaxPool.right; iMaxPoolX++) {
            if( (*it.pIn) > dMax) {
                dMax = (*it.pIn);
                pExpectDelta = it.pInDeviation;
            }
            it.pIn += nInputWstep;
            it.pInDeviation += nInputWstep;
        }
        it.pIn = varConvYBackup.pIn + nInputHstep;
        it.pInDeviation = varConvYBackup.pInDeviation + nInputHstep;
    }

    //(*pExpectDelta) += (*it.pOutDeviation);
    *pExpectDelta += pOutDevia[gid];
}
