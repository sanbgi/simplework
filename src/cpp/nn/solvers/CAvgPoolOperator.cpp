#ifndef __SimpleWork_NN_Operators_CAvgPoolOperator_h__
#define __SimpleWork_NN_Operators_CAvgPoolOperator_h__

#include "operator.h"

static SCtx sCtx("CAvgPoolOperator");
class CAvgPoolOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CAvgPoolOperator* pThis = (CAvgPoolOperator*)pParameters;
            
        int nAvgPoolWidth = pThis->m_nAvgPoolWidth;
        int nAvgPoolHeight = pThis->m_nAvgPoolHeight;
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
        CRect2D rcAvgPool, rcPading = pThis->m_padding;
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
                    rcAvgPool.top = rcPading.top - iOutY * nStrideHeight;
                    rcAvgPool.bottom = nAvgPoolHeight;
                    it.pIn += rcAvgPool.top * nInputHstep;
                }else if(iOutY > iMaxCompleteHeight) {
                    rcAvgPool.top = 0;
                    rcAvgPool.bottom = nAvgPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
                }else{
                    rcAvgPool.top = 0;
                    rcAvgPool.bottom = nAvgPoolHeight;
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
                        rcAvgPool.left = rcPading.left - iOutX * nStrideWidth;
                        rcAvgPool.right = nAvgPoolWidth;
                        it.pIn += rcAvgPool.left * nInputWstep;
                    }else if(iOutX > iMaxCompleteWidth) {
                        rcAvgPool.left = 0;
                        rcAvgPool.right = nAvgPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
                    }else{
                        rcAvgPool.left = 0;
                        rcAvgPool.right = nAvgPoolWidth;
                    }

                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pIn = it.pIn;
                        varConvBackup.pOut = it.pOut;

                        Q dSum = 0;
                        int nSum = 0;

                        //
                        //  从输入中找到最大的那个值
                        //  
                        for( int iAvgPoolY=rcAvgPool.top; iAvgPoolY<rcAvgPool.bottom; iAvgPoolY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pIn = it.pIn;
                            for( int iAvgPoolX=rcAvgPool.left; iAvgPoolX<rcAvgPool.right; iAvgPoolX++) {
                                dSum += (*it.pIn);
                                nSum += 1;
                                it.pIn += nInputWstep;
                            }
                            it.pIn = varConvYBackup.pIn + nInputHstep;
                        }

                        (*it.pOut) = dSum/nSum;

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
        CAvgPoolOperator* pThis = (CAvgPoolOperator*)pParameters;
            
        int nLayer = pThis->m_nInputLayer;
        int nInputWidth = pThis->m_nInputWidth;
        int nInputHeight = pThis->m_nInputHeight;
        int nInputLayer = pThis->m_nInputLayer;
        int nAvgPoolWidth = pThis->m_nAvgPoolWidth;
        int nAvgPoolHeight = pThis->m_nAvgPoolHeight;
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
            Q* pInDeviation;
            Q* pOutDeviation;
        }it = {
            (Q*)inVars[0].devia,
            (Q*)outVar.devia,
        };

        CRect2D rcAvgPool, rcPading = pThis->m_padding;
        int nOffset = rcPading.left * nInputWstep + rcPading.top * nInputHstep;
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
                it.pInDeviation,
                it.pOutDeviation
            };
            for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
                CItOutVariables varOYBackup;
                varOYBackup.pInDeviation = it.pInDeviation;
                varOYBackup.pOutDeviation = it.pOutDeviation;

                //上面填充了都填充了空值，不能参与运算
                if(iOutY < iMinCompleteHeight) {
                    //
                    // 卷积核顶部裁剪，相当于将起始坐标下移，同时
                    //      1,  输入矩阵的起始坐标也需要下移
                    //      2,  权重的其实位置也许要同步下移到实际的其实位置
                    //
                    rcAvgPool.top = rcPading.top - iOutY * nStrideHeight;
                    rcAvgPool.bottom = nAvgPoolHeight;
                    it.pInDeviation += rcAvgPool.top * nInputHstep;
                }else if(iOutY > iMaxCompleteHeight) {
                    rcAvgPool.top = 0;
                    rcAvgPool.bottom = nAvgPoolHeight + (nOutHeight - 1 - iOutY) * nStrideHeight - rcPading.bottom;
                }else{
                    rcAvgPool.top = 0;
                    rcAvgPool.bottom = nAvgPoolHeight;
                }

                for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                    CItOutVariables varOXBackup;
                    varOXBackup.pInDeviation = it.pInDeviation;
                    varOXBackup.pOutDeviation = it.pOutDeviation;

                    //左边填充了都填充了空值，不能参与运算
                    if(iOutX < iMinCompleteWidth) {
                        //
                        // 卷积核左部裁剪，相当于将起始坐标右移，同时
                        //      1，输入矩阵的起始坐标也需要右移
                        //      2，权重矩阵其实坐标也需要右移到对应的开始位置
                        //
                        rcAvgPool.left = rcPading.left - iOutX * nStrideWidth;
                        rcAvgPool.right = nAvgPoolWidth;
                        it.pInDeviation += rcAvgPool.left * nInputWstep;
                    }else if(iOutX > iMaxCompleteWidth) {
                        rcAvgPool.left = 0;
                        rcAvgPool.right = nAvgPoolWidth + (nOutWidth - 1 - iOutX) * nStrideWidth - rcPading.right;
                    }else{
                        rcAvgPool.left = 0;
                        rcAvgPool.right = nAvgPoolWidth;
                    }

                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pInDeviation = it.pInDeviation;
                        varConvBackup.pOutDeviation = it.pOutDeviation;

                        Q dDevia = (*it.pOutDeviation) / (rcAvgPool.bottom-rcAvgPool.top) / (rcAvgPool.right - rcAvgPool.left);

                        //
                        //  从输入中找到最大的那个点，作为反向传到的点
                        //  
                        for( int iAvgPoolY=rcAvgPool.top; iAvgPoolY<rcAvgPool.bottom; iAvgPoolY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pInDeviation = it.pInDeviation;
                            for( int iAvgPoolX=rcAvgPool.left; iAvgPoolX<rcAvgPool.right; iAvgPoolX++) {
                                *it.pInDeviation += dDevia;
                            }
                            it.pInDeviation = varConvYBackup.pInDeviation + nInputHstep;
                        }
                        it.pInDeviation = varConvBackup.pInDeviation + 1;
                        it.pOutDeviation = varConvBackup.pOutDeviation + 1;;
                    }
                    it.pInDeviation = varOXBackup.pInDeviation + nInStrideWstep;
                    it.pOutDeviation = varOXBackup.pOutDeviation + nOutWstep;
                }
                it.pInDeviation = varOYBackup.pInDeviation + nInStrideHstep;
                it.pOutDeviation = varOYBackup.pOutDeviation + nOutHstep;
            }

            //  更新迭代参数
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

        const PNnPool* pAvgPool = CData<PNnPool>(*pData);
        if(pAvgPool == nullptr) {
            return sCtx.error("错误的初始化参数");
        }

        m_nAvgPoolWidth = pAvgPool->nWidth;
        m_nAvgPoolHeight = pAvgPool->nHeight;
        m_nStrideWidth = pAvgPool->nStrideWidth;
        m_nStrideHeight = pAvgPool->nStrideHeight;

        SDimension spDim1 = pInVars[0].dimension();
        if(spDim1.size() < 2) {
            return sCtx.error("池化操作输入张量至少需要二维，代表高度和宽度");
        }
        int nDims = spDim1.size();
        const int* pDimSizes = spDim1.data();

        int nBatchs = 1;
        int nInputHeight = pDimSizes[0];
        int nInputWidth = pDimSizes[1];
        if( nInputHeight < m_nAvgPoolHeight || nInputWidth < m_nAvgPoolWidth ) {
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
        if( pAvgPool != nullptr && pAvgPool->szPadding != nullptr && string(pAvgPool->szPadding) == "same" ) {
            m_nOutHeight = (m_nInputHeight - 1) / m_nStrideHeight + 1;
            m_nOutWidth = (m_nInputWidth - 1) / m_nStrideWidth + 1;
            int nPadW = (m_nOutWidth - 1) * m_nStrideWidth + pAvgPool->nWidth - m_nInputWidth;
            int nPadH = (m_nOutHeight - 1) * m_nStrideHeight + pAvgPool->nHeight - m_nInputHeight;
            m_padding.left = nPadW / 2;
            m_padding.right = nPadW - m_padding.left;
            m_padding.top = nPadH / 2;
            m_padding.bottom = nPadH - m_padding.top;
        }else{
            m_nOutHeight = (m_nInputHeight - m_nAvgPoolHeight) / m_nStrideHeight + 1;
            m_nOutWidth = (m_nInputWidth - m_nAvgPoolWidth) / m_nStrideWidth + 1;
            m_padding.left = m_padding.right = m_padding.top = m_padding.bottom = 0;
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
    const char* getClassName() { return "AvgPoolSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("AvgPoolwidth", m_nAvgPoolWidth);
        ar.arBlock("AvgPoolheight", m_nAvgPoolHeight);
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
    static const char* __getClassKey() { return "sw.nn.AvgPoolSolver"; }

private:
    int m_nAvgPoolWidth;
    int m_nAvgPoolHeight;
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

SIMPLEWORK_FACTORY_AUTO_REGISTER(CAvgPoolOperator, CAvgPoolOperator::__getClassKey())
static SNnSolverRegister s_Register("avgpool", CNnSolver::createSolver<CAvgPoolOperator>);

#endif//__SimpleWork_NN_Operators_CAvgPoolOperator_h__