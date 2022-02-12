#ifndef __SimpleWork_NN_Operators_CPoolOperator_h__
#define __SimpleWork_NN_Operators_CPoolOperator_h__

#include "operator.h"

static SCtx sCtx("PoolOperator");
class CPoolOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CPoolOperator* pThis = (CPoolOperator*)pParameters;
            
        int nPoolWidth = pThis->m_nPoolWidth;
        int nPoolHeight = pThis->m_nPoolHeight;
        int nOutWidth = pThis->m_nOutWidth;
        int nOutHeight = pThis->m_nOutHeight;
        int nTensor = nBatchs;
        int nLayer = pThis->m_nInputLayer;
        int nInputTensorSize = pThis->m_nInputTensorSize;
        int nOutputTensorSize = pThis->m_nOutTensorSize;

        int nInputHstep = pThis->m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
        int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;

        int nOutHstep = pThis->m_nOutWidth * nLayer;
        int nOutWstep = nLayer;

        struct CItOutVariables {
            Q* pIn;
            Q* pOut;
        }it = {
            (Q*)inVars[0].data,
            (Q*)outVar.data,
        };


        //
        //  注意，关于填充问题，算法相对比较抽象
        //      
        //      首先将数据矩阵首先按照填充规则，进行四周填充，然后，将输入指针指向填充后的最左上角，
        //  此时，指针实际上是一个非法指针。在遍历输出矩阵宽度高度时，如果发现其实位置小于填充尺寸
        //  则，卷积矩阵调整到有效的开始位置，而输入矩阵指针也同时指向有效的地址，与卷积矩阵开始位
        //  置一致。
        //
        CRect2D rcPool, rcPading = pThis->m_padding;
        int nOffset = rcPading.left * nInputWstep + rcPading.top * nInputHstep;
        it.pIn = it.pIn - nOffset;
        int nStrideHeight = pThis->m_nStrideHeight;
        int iMinCompleteHeight = (rcPading.top + nStrideHeight - 1) / nStrideHeight;
        int iMaxCompleteHeight = nOutHeight - 1 - (rcPading.bottom + nStrideHeight - 1) / nStrideHeight;
        //
        //  输出矩阵能够完整卷积的最大下标，再往右，则需要剪裁了
        //
        int nStrideWidth = pThis->m_nStrideWidth;
        int iMinCompleteWidth = (rcPading.left + nStrideWidth - 1) / nStrideWidth;
        int iMaxCompleteWidth = nOutWidth - 1 - (rcPading.right + nStrideWidth - 1) / nStrideWidth;
        for(int iTensor=0; iTensor<nTensor; iTensor++) {
            CItOutVariables varTBackup = {
                it.pIn,
                it.pOut,
            };
            for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
                CItOutVariables varOYBackup;
                varOYBackup.pIn = it.pIn;
                varOYBackup.pOut = it.pOut;

                //上面填充了都填充了空值，不能参与运算
                if(iOutY < iMinCompleteHeight) {
                    //
                    // 卷积核顶部裁剪，相当于将起始坐标下移，同时
                    //      1,  输入矩阵的起始坐标也需要下移
                    //      2,  权重的其实位置也许要同步下移到实际的其实位置
                    //
                    rcPool.top = rcPading.top - iOutY * nStrideHeight;
                    rcPool.bottom = nPoolHeight;
                    it.pIn += rcPool.top * nInputHstep;
                }else if(iOutY > iMaxCompleteHeight) {
                    rcPool.top = 0;
                    rcPool.bottom = nPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
                }else{
                    rcPool.top = 0;
                    rcPool.bottom = nPoolHeight;
                }

                for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                    CItOutVariables varOXBackup;
                    varOXBackup.pIn = it.pIn;
                    varOXBackup.pOut = it.pOut;

                    //左边填充了都填充了空值，不能参与运算
                    if(iOutX < iMinCompleteWidth) {
                        //
                        // 卷积核左部裁剪，相当于将起始坐标右移，同时
                        //      1，输入矩阵的起始坐标也需要右移
                        //      2，权重矩阵其实坐标也需要右移到对应的开始位置
                        //
                        rcPool.left = rcPading.left - iOutX * nStrideWidth;
                        rcPool.right = nPoolWidth;
                        it.pIn += rcPool.left * nInputWstep;
                    }else if(iOutX > iMaxCompleteWidth) {
                        rcPool.left = 0;
                        rcPool.right = nPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
                    }else{
                        rcPool.left = 0;
                        rcPool.right = nPoolWidth;
                    }

                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pIn = it.pIn;
                        varConvBackup.pOut = it.pOut;

                        Q dMax = (*it.pIn);

                        //
                        //  从输入中找到最大的那个值
                        //  
                        for( int iPoolY=rcPool.top; iPoolY<rcPool.bottom; iPoolY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pIn = it.pIn;
                            for( int iPoolX=rcPool.left; iPoolX<rcPool.right; iPoolX++) {
                                if( (*it.pIn) > dMax) {
                                    dMax = (*it.pIn);
                                }
                                it.pIn += nInputWstep;
                            }
                            it.pIn = varConvYBackup.pIn + nInputHstep;
                        }

                        (*it.pOut) = dMax;

                        it.pIn = varConvBackup.pIn + 1;
                        it.pOut = varConvBackup.pOut + 1;;
                    }
                    it.pIn = varOXBackup.pIn + nInStrideWstep;
                    it.pOut = varOXBackup.pOut + nOutWstep;
                }
                it.pIn = varOYBackup.pIn + nInStrideHstep;
                it.pOut = varOYBackup.pOut + nOutHstep;
            }
            it.pIn = varTBackup.pIn + nInputTensorSize;
            it.pOut = varTBackup.pOut + nOutputTensorSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CPoolOperator* pThis = (CPoolOperator*)pParameters;
            
        int nLayer = pThis->m_nInputLayer;
        int nInputWidth = pThis->m_nInputWidth;
        int nInputHeight = pThis->m_nInputHeight;
        int nInputLayer = pThis->m_nInputLayer;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nPoolHeight = pThis->m_nPoolHeight;
        int nOutWidth = pThis->m_nOutWidth;
        int nOutHeight = pThis->m_nOutHeight;

        int nTensor = nBatchs;
        int nInputTensorSize = pThis->m_nInputTensorSize;
        int nOutputTensorSize = pThis->m_nOutTensorSize;

        int nInputHstep = pThis->m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
        int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;
        
        int nOutHstep = nOutWidth * nLayer;
        int nOutWstep = nLayer;

        struct CItOutVariables {
            Q* pIn;
            Q* pInDeviation;
            Q* pOutDeviation;
        }it = {
            (Q*)inVars[0].data,
            (Q*)inVars[0].devia,
            (Q*)outVar.devia,
        };

        CRect2D rcPool, rcPading = pThis->m_padding;
        int nOffset = rcPading.left * nInputWstep + rcPading.top * nInputHstep;
        it.pIn = it.pIn - nOffset;
        it.pInDeviation = it.pInDeviation - nOffset;
        int nStrideHeight = pThis->m_nStrideHeight;
        int iMinCompleteHeight = (rcPading.top + nStrideHeight - 1) / nStrideHeight;
        int iMaxCompleteHeight = nOutHeight - 1 - (rcPading.bottom + nStrideHeight - 1) / nStrideHeight;
        //
        //  输出矩阵能够完整卷积的最大下标，再往右，则需要剪裁了
        //
        int nStrideWidth = pThis->m_nStrideWidth;
        int iMinCompleteWidth = (rcPading.left + nStrideWidth - 1) / nStrideWidth;
        int iMaxCompleteWidth = nOutWidth - 1 - (rcPading.right + nStrideWidth - 1) / nStrideWidth;
        for(int iTensor=0; iTensor<nTensor; iTensor++) {
            CItOutVariables varTBackup = {
                it.pIn,
                it.pInDeviation,
                it.pOutDeviation
            };
            for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
                CItOutVariables varOYBackup;
                varOYBackup.pIn = it.pIn;
                varOYBackup.pInDeviation = it.pInDeviation;
                varOYBackup.pOutDeviation = it.pOutDeviation;

                //上面填充了都填充了空值，不能参与运算
                if(iOutY < iMinCompleteHeight) {
                    //
                    // 卷积核顶部裁剪，相当于将起始坐标下移，同时
                    //      1,  输入矩阵的起始坐标也需要下移
                    //      2,  权重的其实位置也许要同步下移到实际的其实位置
                    //
                    rcPool.top = rcPading.top - iOutY * nStrideHeight;
                    rcPool.bottom = nPoolHeight;
                    it.pIn += rcPool.top * nInputHstep;
                    it.pInDeviation += rcPool.top * nInputHstep;
                }else if(iOutY > iMaxCompleteHeight) {
                    rcPool.top = 0;
                    rcPool.bottom = nPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
                }else{
                    rcPool.top = 0;
                    rcPool.bottom = nPoolHeight;
                }

                for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                    CItOutVariables varOXBackup;
                    varOXBackup.pIn = it.pIn;
                    varOXBackup.pInDeviation = it.pInDeviation;
                    varOXBackup.pOutDeviation = it.pOutDeviation;

                    //左边填充了都填充了空值，不能参与运算
                    if(iOutX < iMinCompleteWidth) {
                        //
                        // 卷积核左部裁剪，相当于将起始坐标右移，同时
                        //      1，输入矩阵的起始坐标也需要右移
                        //      2，权重矩阵其实坐标也需要右移到对应的开始位置
                        //
                        rcPool.left = rcPading.left - iOutX * nStrideWidth;
                        rcPool.right = nPoolWidth;
                        it.pIn += rcPool.left * nInputWstep;
                        it.pInDeviation += rcPool.left * nInputWstep;
                    }else if(iOutX > iMaxCompleteWidth) {
                        rcPool.left = 0;
                        rcPool.right = nPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
                    }else{
                        rcPool.left = 0;
                        rcPool.right = nPoolWidth;
                    }

                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pIn = it.pIn;
                        varConvBackup.pInDeviation = it.pInDeviation;
                        varConvBackup.pOutDeviation = it.pOutDeviation;

                        Q dMax = (*it.pIn);
                        Q* pExpectDelta = it.pInDeviation;

                        //
                        //  从输入中找到最大的那个点，作为反向传到的点
                        //  
                        for( int iPoolY=rcPool.top; iPoolY<rcPool.bottom; iPoolY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pIn = it.pIn;
                            varConvYBackup.pInDeviation = it.pInDeviation;
                            for( int iPoolX=rcPool.left; iPoolX<rcPool.right; iPoolX++) {
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

                        (*pExpectDelta) = (*it.pOutDeviation);

                        it.pIn = varConvBackup.pIn + 1;
                        it.pInDeviation = varConvBackup.pInDeviation + 1;
                        it.pOutDeviation = varConvBackup.pOutDeviation + 1;;
                    }

                    it.pIn = varOXBackup.pIn + nInStrideWstep;
                    it.pInDeviation = varOXBackup.pInDeviation + nInStrideWstep;
                    it.pOutDeviation = varOXBackup.pOutDeviation + nOutWstep;
                }

                it.pIn = varOYBackup.pIn + nInStrideHstep;
                it.pInDeviation = varOYBackup.pInDeviation + nInStrideHstep;
                it.pOutDeviation = varOYBackup.pOutDeviation + nOutHstep;
            }

            //  更新迭代参数
            it.pIn = varTBackup.pIn + nInputTensorSize;
            it.pInDeviation = varTBackup.pInDeviation + nInputTensorSize;
            it.pOutDeviation = varTBackup.pOutDeviation + nOutputTensorSize;
        }
    }

    int prepareSolver(unsigned int idType, PSolveParameter& solveParameter) {
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        if(nInVars != 1) {
            return sCtx.error("卷积操作需要1个输入数据");
        }
        if(pData == nullptr) {
            return sCtx.error("缺少初始化参数");
        }

        const PNnPool* pPool = CData<PNnPool>(*pData);
        if(pPool == nullptr) {
            return sCtx.error("错误的初始化参数");
        }

        m_nPoolWidth = pPool->nWidth;
        m_nPoolHeight = pPool->nHeight;
        m_nStrideWidth = pPool->nStrideWidth;
        m_nStrideHeight = pPool->nStrideHeight;

        SDimension spDim1 = pInVars[0].dimension();
        if(spDim1.size() < 2) {
            return sCtx.error("池化操作输入张量至少需要二维，代表高度和宽度");
        }
        int nDims = spDim1.size();
        const int* pDimSizes = spDim1.data();

        int nBatchs = 1;
        int nInputHeight = pDimSizes[0];
        int nInputWidth = pDimSizes[1];
        if( nInputHeight < m_nPoolHeight || nInputWidth < m_nPoolWidth ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        int nLayers = 1;
        for( int i=2; i<nDims; i++ ) {
            nLayers *= pDimSizes[i];
        }

        int nInputCells = nInputWidth * nInputHeight * nLayers;
        m_nInputHeight = pDimSizes[0];
        m_nInputWidth = pDimSizes[1];
        m_nInputLayer = 1;
        int pOutDimSizes[nDims];
        for( int i=2; i<nDims; i++) {
            pOutDimSizes[i] = pDimSizes[i];
            m_nInputLayer *= pDimSizes[i];
        }
        if( pPool != nullptr && pPool->szPadding != nullptr && string(pPool->szPadding) == "same" ) {
            m_nOutHeight = (m_nInputHeight - 1) / m_nStrideHeight + 1;
            m_nOutWidth = (m_nInputWidth - 1) / m_nStrideWidth + 1;
            int nPadW = (m_nOutWidth - 1) * m_nStrideWidth + pPool->nWidth - m_nInputWidth;
            int nPadH = (m_nOutHeight - 1) * m_nStrideHeight + pPool->nHeight - m_nInputHeight;
            m_padding.left = nPadW / 2;
            m_padding.right = nPadW - m_padding.left;
            m_padding.top = nPadH / 2;
            m_padding.bottom = nPadH - m_padding.top;
        }else{
            m_nOutHeight = (m_nInputHeight - m_nPoolHeight) / m_nStrideHeight + 1;
            m_nOutWidth = (m_nInputWidth - m_nPoolWidth) / m_nStrideWidth + 1;
        }
        m_nOutTensorSize = m_nOutHeight * m_nOutWidth * m_nInputLayer;
        m_nInputTensorSize = m_nInputHeight * m_nInputWidth * m_nInputLayer;
        pOutDimSizes[0] = m_nOutHeight;
        pOutDimSizes[1] = m_nOutWidth;
        createVariable(SDimension(nDims, pOutDimSizes),spVarOut);
        return addAtomSolver(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "PoolSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("poolwidth", m_nPoolWidth);
        ar.arBlock("poolheight", m_nPoolHeight);
        ar.arBlock("stridewidth", m_nStrideWidth);
        ar.arBlock("strideheight", m_nStrideHeight);
        ar.arBlock("padding", m_padding);
        ar.arBlock("inputwidth", m_nInputWidth);
        ar.arBlock("inputheight", m_nInputHeight);
        ar.arBlock("inputlayer", m_nInputLayer);
        ar.arBlock("inputtensorsize", m_nInputTensorSize);
        ar.arBlock("outputwidth", m_nOutWidth);
        ar.arBlock("outputheight", m_nOutHeight);
        ar.arBlock("outputtnesorsize", m_nOutTensorSize);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.PoolSolver"; }

private:
    int m_nPoolWidth;
    int m_nPoolHeight;
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
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CPoolOperator, CPoolOperator::__getClassKey())
static SNnSolverRegister s_Register("pool", CNnSolver::createSolver<CPoolOperator>);

#endif//__SimpleWork_NN_Operators_CPoolOperator_h__