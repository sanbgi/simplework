#ifndef __SimpleWork_NN_Operators_CConvOperator_h__
#define __SimpleWork_NN_Operators_CConvOperator_h__

#include "../CNnOperator.h"
#include "../CSize.h"

//
// 卷积核切换策略数组，w为宽度方向一步切换多少个卷积核
//
// 疑问？是否可以多少步切换一个卷积核呢？目前还无法确定哪个策略好
//
const int s_nShiftPolicy = 4;
struct CShiftPolicy {
    int wBais;
    int hBais;
    int wWeight;
    int hWeight;
};
struct CShiftPolicies {
    CShiftPolicy s[s_nShiftPolicy];
};

static void s_GetShiftPolicy(CShiftPolicies& shiftPolicies, int nConvs, int nConvSize) {
    /*
    s_shiftPolicies[] = {
        { 1,  1 },  // 45度，右下方向切换卷积核
        { 1, -1 },  //-45度，右上方向切换卷积核
        { 1,  0 },  //  0度，正右方向切换卷积核
        { 0,  1 },  // 90度，正下方向切换卷积核
        { 1,  2 },  // 60度
        { 2, -1 },  //-30度
        { 1, -2 },  //-60度
        { 2,  1 }   // 30度 
    };

    shiftPolicies = {{
        { 1,  0, nConvSize,  0 },
        { 1,  0, nConvSize,  0 },
        { 1,  0, nConvSize,  0 },
        { 1,  0, nConvSize,  0 },
    }};

    shiftPolicies = {{
        { 1,  1, nConvSize,  nConvSize             },
        { 1, -1, nConvSize,  nConvSize*(nConvs-1)  },
        { 1,  0, nConvSize,  0                   },
        { 0,  1, 0,          nConvSize             },
    }};
    */
    shiftPolicies = {{
        { 1,  0, nConvSize,  0          },
        { 0,  1, 0,          nConvSize  },
        { 1,  0, nConvSize,  0          },
        { 0,  1, 0,          nConvSize  },
    }};
};


class CConvOperator : public CNnOperator {
public:
    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
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

    int solve(int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return sCtx.error();
    }

    static int createOperator( const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnOperator& spOutVar) {
        CPointer<CConvOperator> spOut;
        CObject::createObject(spOut);
        if( int retcode = spOut->initConvVariable(szPadding, nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }

    int initConvVariable(const char* szPadding, int nInVars, const SNnVariable pInVars[]) {

        if( int retcode = initOperator(nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }

        if(nInVars != 3) {
            return sCtx.error("卷积操作需要三个输入数据");
        }

        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        SDimension spDimBais = pInVars[2].dimension();
        if(spDim2.size() != 5) {
            return sCtx.error("卷积核需要一个五维矩阵nLayer*nShiftConvs*nWidth*nHeight*nDeep");
        }
        const int* pDimSize2 = spDim2.data();

        if(spDim1.size() < 2) {
            return sCtx.error("卷积操作输入张量至少需要二维，代表高度和宽度");
        }
        int nDim1 = spDim1.size();
        const int* pDimSize1 = spDim1.data();
        int nBatchs = 1;
        int nInputLayers = 1;
        for(int i=2; i<nDim1; i++) {
            nInputLayers *= pDimSize1[i];
        }

        if(nInputLayers != pDimSize2[4]) {
            return sCtx.error("卷积核深度于输入张量深度不一致");
        }

        if(pDimSize2[2] > pDimSize1[0] || pDimSize2[3] > pDimSize1[1]) {
            return sCtx.error("卷积核尺寸不应该大于输入尺寸");
        }

        if(spDimBais.dataSize() != pDimSize2[0] * pDimSize2[1]) {
            return sCtx.error("偏置量需要与卷积核数相等");
        }

        m_nStrideHeight = m_nStrideWidth = 1;
        m_nLayers = pDimSize2[0];
        m_nInputLayers = pDimSize2[4];
        m_sizeConv = {
            pDimSize2[1],
            pDimSize2[2],
            pDimSize2[3],
            m_nInputLayers,
        };
        m_sizeIn = {
            1,
            pDimSize1[0],
            pDimSize1[1],
            m_nInputLayers
        };

        //
        //  填充模式有两种：
        //      same -- 步长为1时，保持输出尺寸与输入尺寸一致
        //      valid(默认) -- 步长为1时，只输出完整卷积结果尺寸，所以，输出尺寸会缩小（卷积核宽度-1)
        //
        if( szPadding && string(szPadding) == "same" ) {
            m_sizeOut = {
                nBatchs,
                (m_sizeIn.height - 1) / m_nStrideHeight + 1,
                (m_sizeIn.width - 1) / m_nStrideWidth + 1,
                m_nLayers
            };
            int nPadW = m_sizeOut.width * m_nStrideWidth - (m_sizeIn.width - m_sizeConv.width + 1);
            int nPadH = m_sizeOut.height * m_nStrideHeight - (m_sizeIn.height - m_sizeConv.height + 1);
            m_padding.left = nPadW / 2;
            m_padding.right = nPadW - m_padding.left;
            m_padding.top = nPadH / 2;
            m_padding.bottom = nPadH - m_padding.top;
        }else{
            //
            //  默认为不填充
            //
            m_sizeOut = {
                nBatchs,
                (m_sizeIn.height - m_sizeConv.height) / m_nStrideHeight + 1,
                (m_sizeIn.width - m_sizeConv.width) / m_nStrideWidth + 1,
                m_nLayers
            };
            m_padding = { 0, 0, 0, 0 };
        }

        m_stepInConv = {
            m_sizeIn.height * m_sizeIn.width * nInputLayers,
            m_sizeIn.width * nInputLayers,
            nInputLayers
        };

        m_stepInMove = {  
            m_stepInConv.batch,
            m_stepInConv.height * m_nStrideHeight,
            m_stepInConv.width * m_nStrideWidth
        };

        m_stepOut = {
            m_sizeOut.height * m_sizeOut.width * m_nLayers,
            m_sizeOut.width * m_nLayers,
            m_nLayers
        };

        m_stepConv = {
            m_sizeConv.height * m_sizeConv.width * nInputLayers,
            m_sizeConv.width * nInputLayers,
            nInputLayers
        };

        int pOutDimSizes[3] = { m_sizeOut.height, m_sizeOut.width, m_sizeOut.layers };
        if( SDimension::createDimension(m_spDimension, 3, pOutDimSizes) != sCtx.success() ) {
            return sCtx.error("创建输出张量的维度向量失败");
        }

        return sCtx.success();
    }


    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CConvOperator* pThis = (CConvOperator*)pParameters;
        
        //
        // 关于尺寸说明
        //  1, size表示具体的尺寸
        //  2，step表示在尺寸维度上移动指针，需要的步数
        //
        //  比如：
        //      sizeIn.width        输入矩阵的宽度
        //      stepInMove.width    输入矩阵在移动时，在宽度上移动一步，对于指针需要移动的距离（考虑了步长）
        //      stepInConv.width    输入矩阵在卷积时，在宽度上移动一步时，对于指针需要移动的距离
        //      stepConv.width      卷积矩阵，在宽度上移动一步时，卷积矩阵指针需要移动的距离
        //
        CBatchSize3D sizeIn = pThis->m_sizeIn;
        CBatchSize3D sizeOut = pThis->m_sizeOut;
        CBatchSize3D sizeConv = pThis->m_sizeConv;
        CBatchSize2D stepInMove = pThis->m_stepInMove;
        CBatchSize2D stepInConv = pThis->m_stepInConv;
        CBatchSize2D stepOut = pThis->m_stepOut;
        CBatchSize2D stepConv = pThis->m_stepConv;
        struct CItOutVariables {
            Q* pIn;
            Q* pOut;
            Q* pWeights;
            Q* pBais;
            int index;
        }itVars, itVars0, itVars2, itVars3, itVars1, itVars4, itVars5, itVars6, it = {
            (Q*)inVars[0].data,
            (Q*)outVar.data,
            (Q*)inVars[1].data,
            (Q*)inVars[2].data,
        };

        CShiftPolicy stepPolicy;
        CShiftPolicies shiftPolicies;
        s_GetShiftPolicy(shiftPolicies, sizeConv.batch, stepConv.batch );

        //
        //
        //  注意，
        //      权重卷积核大小 : nLayers * nShiftConvs * h * w * l
        //      偏置大小：nLayers * nShiftConvs
        //
        int nLayers = pThis->m_nLayers;
        int nLayerStepShifts = sizeConv.batch * stepConv.batch;
        Q dConv, *pWeightEnd;
        itVars = it;

        //
        //  注意，关于填充问题，算法相对比较抽象。
        //      
        //      首先将数据矩阵首先按照填充规则，进行四周填充，然后，将输入指针指向填充后的最左上角，
        //  此时，指针实际上是一个非法指针。在遍历输出矩阵宽度高度时，如果发现其实位置小于填充尺寸
        //  则，卷积矩阵调整到有效的开始位置，而输入矩阵指针也同时指向有效的地址，与卷积矩阵开始位
        //  置一致。
        //
        //  比如：
        //      如果左边填充了两个像素，则卷积矩阵从第二各值开始（rcConv.left），输入指针向右平
        //  移2各像素（it.pIn += rcConv.left * stepInConv.width)
        //      在高度方向，策略核宽度一致
        //
        CRect2D rcConv, rcPading = pThis->m_padding;
        int nOffset = rcPading.left * stepInConv.width + rcPading.top * stepInConv.height;
        it.pIn = it.pIn - nOffset;
        //
        //  输出矩阵能够完整卷积的最大下标，再往下，则需要剪裁了
        //
        int iCompleteConvHeightIndex = sizeOut.height - rcPading.bottom - 1;
        //
        //  输出矩阵能够完整卷积的最大下标，再往右，则需要剪裁了
        //
        int iCompleteConvWidthIndex = sizeOut.width - rcPading.right - 1;

        for(itVars0.index=0; itVars0.index < sizeIn.batch; itVars0.index++) {
            itVars0.pIn = it.pIn;
            itVars0.pOut = it.pOut;
            it.pWeights = itVars.pWeights;  //重置weight
            it.pBais = itVars.pBais;        //重置bais
            for(itVars1.index = 0; itVars1.index < nLayers; itVars1.index++) {
                itVars1.pIn = it.pIn;
                itVars1.pOut = it.pOut;
                itVars1.pWeights = it.pWeights;
                itVars1.pBais = it.pBais;
                pWeightEnd = it.pWeights + nLayerStepShifts;
                stepPolicy = shiftPolicies.s[itVars1.index%s_nShiftPolicy];
                for(itVars2.index=0; itVars2.index < sizeOut.height; itVars2.index++) {
                    itVars2.pIn = it.pIn;
                    itVars2.pOut = it.pOut;
                    itVars2.pWeights = it.pWeights;
                    itVars2.pBais = it.pBais;

                    //左边填充了都填充了空值，不能参与运算
                    if(itVars2.index < rcPading.top) {
                        //
                        // 卷积核顶部裁剪，相当于将起始坐标下移，同时
                        //      1,  输入矩阵的起始坐标也需要下移
                        //      2,  权重的其实位置也许要同步下移到实际的其实位置
                        //
                        rcConv.top = rcPading.top;
                        rcConv.bottom = sizeConv.height;
                        it.pIn += rcConv.top * stepInConv.height;
                        it.pWeights += rcConv.top * stepConv.height;
                    }else if(itVars2.index > iCompleteConvHeightIndex) {
                        rcConv.top = 0;
                        rcConv.bottom = sizeConv.height - (itVars2.index - iCompleteConvHeightIndex);
                    }else{
                        rcConv.top = 0;
                        rcConv.bottom = sizeConv.height;
                    }

                    for(itVars3.index=0; itVars3.index < sizeOut.width; itVars3.index++) {
                        itVars3.pIn = it.pIn;
                        itVars3.pOut = it.pOut;
                        itVars3.pWeights = it.pWeights;
                        itVars3.pBais = it.pBais;

                        //左边填充了都填充了空值，不能参与运算
                        if(itVars3.index < rcPading.left) {
                            //
                            // 卷积核左部裁剪，相当于将起始坐标右移，同时
                            //      1，输入矩阵的起始坐标也需要右移
                            //      2，权重矩阵其实坐标也需要右移到对应的开始位置
                            //
                            rcConv.left = rcPading.left;
                            rcConv.right = sizeConv.width;
                            it.pIn += rcConv.left * stepInConv.width;
                            it.pWeights += rcConv.left * stepConv.width;
                        }else if(itVars3.index > iCompleteConvWidthIndex) {
                            rcConv.left = 0;
                            rcConv.right = sizeConv.width - (itVars3.index - iCompleteConvWidthIndex);
                        }else{
                            rcConv.left = 0;
                            rcConv.right = sizeConv.width;
                        }

                        //初始化卷积结果
                        dConv = 0;
                        for( itVars4.index=rcConv.top; itVars4.index<rcConv.bottom; itVars4.index++) {
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
                        (*it.pOut) = dConv - (*it.pBais);

                        it.pIn = itVars3.pIn + stepInMove.width;
                        it.pOut = itVars3.pOut + stepOut.width;

                        //
                        // 如果权重超出范围，则重新开始
                        //  注意权重矩阵    尺寸：nShiftConvs * w * h * l
                        //                 步长：w * h * l
                        //
                        it.pWeights = itVars3.pWeights + stepPolicy.wWeight;
                        it.pBais = itVars3.pBais + stepPolicy.wBais;
                        if(it.pWeights >= pWeightEnd ) {
                            it.pWeights -= nLayerStepShifts;
                            it.pBais -= sizeConv.batch;
                        }
                    }

                    it.pIn = itVars2.pIn + stepInMove.height;
                    it.pOut = itVars2.pOut + stepOut.height;
                    it.pWeights = itVars2.pWeights + stepPolicy.hWeight;
                    it.pBais = itVars2.pBais + stepPolicy.hBais;
                    if(it.pWeights >= pWeightEnd ) {
                        it.pWeights -= nLayerStepShifts;
                        it.pBais -= sizeConv.batch;
                    }
                }
                it.pIn = itVars1.pIn;
                it.pOut = itVars1.pOut + 1;
                it.pWeights = itVars1.pWeights + nLayerStepShifts;
                it.pBais = itVars1.pBais + sizeConv.batch;
            }
            
            it.pIn = itVars0.pIn + stepInMove.batch;
            it.pOut = itVars0.pOut + stepOut.batch;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CConvOperator* pThis = (CConvOperator*)pParameters;

        //
        // 关于尺寸说明，注意参考evalT里面的说明
        //
        CBatchSize3D sizeIn = pThis->m_sizeIn;
        CBatchSize3D sizeOut = pThis->m_sizeOut;
        CBatchSize3D sizeConv = pThis->m_sizeConv;
        CBatchSize2D stepInMove = pThis->m_stepInMove;
        CBatchSize2D stepInConv = pThis->m_stepInConv;
        CBatchSize2D stepOut = pThis->m_stepOut;
        CBatchSize2D stepConv = pThis->m_stepConv;

        CShiftPolicy stepPolicy;
        CShiftPolicies shiftPolicies;
        s_GetShiftPolicy(shiftPolicies, sizeConv.batch, stepConv.batch);

        struct CItOutVariables {
            Q* pIn;
            Q* pInDeviation;
            Q* pOut;
            Q* pOutDeviation;
            Q* pWeights;
            Q* pWeightDevivation;
            Q* pBaisDeviation;
            Q* pZDeviatioin;
            int index;
        }it = {
            (Q*)inVars[0].data,
            (Q*)inVars[0].devia,
            (Q*)outVar.data,
            (Q*)outVar.devia,
            (Q*)inVars[1].data,
            (Q*)inVars[1].devia,
            (Q*)inVars[2].devia
        }, itVars, itVars0,itVars2,itVars3,itVars1,itVars4,itVars5,itVars6;

        Q deviationZ;
        int nLayers = pThis->m_nLayers;
        int nLayerStepShifts = sizeConv.batch * stepConv.batch;
        Q* pWeightEnd;

        //
        //
        //  注意，
        //      权重卷积核大小 : nLayers * nShiftConvs * h * w * l
        //      偏置大小：nLayers * nShiftConvs 
        //
        itVars = it;

        //
        //  注意，关于填充问题，参考evalT说明
        //
        CRect2D rcConv, rcPading = pThis->m_padding;
        //将输入指针向Padding后的起点偏移，变成一个非法指针
        int nOffsetIn = rcPading.left * stepInConv.width + rcPading.top * stepInConv.height;
        int nOffsetConv;
        //输出矩阵能够完整卷积的下标，再往下，则需要剪裁了
        int iCompleteConvHeightIndex = sizeOut.height - rcPading.bottom - 1;
        //输出矩阵能够完整卷积的下标，再往右，则需要剪裁了
        int iCompleteConvWidthIndex = sizeOut.width - rcPading.right - 1;

        it.pIn = it.pIn - nOffsetIn;
        it.pInDeviation = it.pInDeviation - nOffsetIn;
        for(itVars0.index=0; itVars0.index<sizeIn.batch; itVars0.index++) {
            itVars0.pIn = it.pIn;
            itVars0.pInDeviation = it.pInDeviation;
            itVars0.pOut = it.pOut;
            itVars0.pOutDeviation = it.pOutDeviation; 

            it.pZDeviatioin = it.pOutDeviation;             
            it.pWeights = itVars.pWeights;                  //重置Weight
            it.pWeightDevivation = itVars.pWeightDevivation;//重置偏差
            it.pBaisDeviation = itVars.pBaisDeviation;      //重置Bais
            for(itVars1.index = 0; itVars1.index < nLayers; itVars1.index++) {
                itVars1.pIn = it.pIn;
                itVars1.pInDeviation = it.pInDeviation;
                itVars1.pZDeviatioin = it.pZDeviatioin;
                itVars1.pWeights = it.pWeights;
                itVars1.pWeightDevivation = it.pWeightDevivation;
                itVars1.pBaisDeviation = it.pBaisDeviation;
                pWeightEnd = it.pWeights + nLayerStepShifts;
                stepPolicy = shiftPolicies.s[itVars1.index%s_nShiftPolicy];
                for(itVars2.index=0; itVars2.index < sizeOut.height; itVars2.index++) {
                    itVars2.pIn = it.pIn;
                    itVars2.pInDeviation = it.pInDeviation;
                    itVars2.pZDeviatioin = it.pZDeviatioin;
                    itVars2.pWeights = it.pWeights;
                    itVars2.pWeightDevivation = it.pWeightDevivation;
                    itVars2.pBaisDeviation = it.pBaisDeviation;

                    //上下填充了都填充了控空制，不能参与运算
                    if(itVars2.index < rcPading.top) {
                        //
                        // 卷积核顶部裁剪，相当于将起始坐标下移，同时
                        //      1,  输入矩阵的起始坐标也需要下移
                        //      2,  权重的其实位置也许要同步下移到实际的其实位置
                        //
                        rcConv.top = rcPading.top;
                        rcConv.bottom = sizeConv.height;
                        nOffsetIn = rcConv.top * stepInConv.height;
                        nOffsetConv = rcConv.top * stepConv.height;
                        it.pIn += nOffsetIn;
                        it.pInDeviation += nOffsetIn;
                        it.pWeights += nOffsetConv;
                        it.pWeightDevivation += nOffsetConv;
                    }else if(itVars2.index > iCompleteConvHeightIndex) {
                        rcConv.top = 0;
                        rcConv.bottom = sizeConv.height - (itVars2.index - iCompleteConvHeightIndex);
                    }else{
                        rcConv.top = 0;
                        rcConv.bottom = sizeConv.height;
                    }

                    for(itVars3.index=0; itVars3.index < sizeOut.width; itVars3.index++) {
                        itVars3.pIn = it.pIn;
                        itVars3.pInDeviation = it.pInDeviation;
                        itVars3.pZDeviatioin = it.pZDeviatioin;
                        itVars3.pWeights = it.pWeights;
                        itVars3.pWeightDevivation = it.pWeightDevivation;
                        itVars3.pBaisDeviation = it.pBaisDeviation;

                        //
                        //  计算目标函数对当前输出值的偏导数
                        //      X = 输入
                        //      Z = 权重*X-偏置
                        //      Y = activation(Z)
                        //      delta = 目标 - Y
                        //      E = delta*delta/2 目标函数
                        //      deviationZ = d(E) / d(Z) = d(E)/d(delta) * d(delta)/d(Y) * d(F)/d(Z)
                        //      其中：
                        //          d(E)/d(delta) = pOutDeviation[iOutput]
                        //          d(delta)/d(Y) = 1
                        //          d(Y)/d(Z) = deactivate(Y)
                        //
                        deviationZ = (*it.pZDeviatioin);
                        if(deviationZ > 1.0e-16 || deviationZ < -1.0e-16) 
                        {
                            //左右填充了都填充了空制，不能参与运算
                            if(itVars3.index < rcPading.left) {
                                //
                                // 卷积核左部裁剪，相当于将起始坐标右移，同时
                                //      1，输入矩阵的起始坐标也需要右移
                                //      2，权重矩阵其实坐标也需要右移到对应的开始位置
                                //
                                rcConv.left = rcPading.left;
                                rcConv.right = sizeConv.width;
                                nOffsetIn = rcConv.left * stepInConv.width;
                                nOffsetConv = rcConv.left * stepConv.width;
                                it.pIn += nOffsetIn;
                                it.pInDeviation += nOffsetIn;
                                it.pWeights += nOffsetConv;
                                it.pWeightDevivation += nOffsetConv;
                            }else if(itVars3.index > iCompleteConvWidthIndex) {
                                rcConv.left = 0;
                                rcConv.right = sizeConv.width - (itVars3.index - iCompleteConvWidthIndex);
                            }else{
                                rcConv.left = 0;
                                rcConv.right = sizeConv.width;
                            }

                            //
                            //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
                            //  
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
                                        (*it.pInDeviation) += deviationZ * (*it.pWeights);
                                        (*it.pWeightDevivation) += deviationZ * (*it.pIn);

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

                            //
                            //  偏置的偏导数刚好是输出的偏导数的负数，所以，下降梯度值为(-deviationZ)
                            //
                            (*it.pBaisDeviation) += (-deviationZ);
                        }

                        it.pIn = itVars3.pIn + stepInMove.width;
                        it.pInDeviation = itVars3.pInDeviation + stepInMove.width;
                        it.pZDeviatioin = itVars3.pZDeviatioin + stepOut.width;

                        //
                        // 如果权重超出范围，则重新开始
                        //  注意权重矩阵    尺寸：nShiftConvs * w * h * l
                        //                 步长：w * h * l
                        //
                        it.pWeights = itVars3.pWeights + stepPolicy.wWeight;
                        it.pWeightDevivation = itVars3.pWeightDevivation + stepPolicy.wWeight;
                        it.pBaisDeviation = itVars3.pBaisDeviation + stepPolicy.wBais;
                        if(it.pWeights >= pWeightEnd ) {
                            it.pWeights -= nLayerStepShifts;
                            it.pWeightDevivation -= nLayerStepShifts;
                            it.pBaisDeviation -= sizeConv.batch;
                        }
                    }
                    it.pIn = itVars2.pIn + stepInMove.height;
                    it.pInDeviation = itVars2.pInDeviation + stepInMove.height;
                    it.pZDeviatioin = itVars2.pZDeviatioin + stepOut.height;
                    it.pWeights = itVars2.pWeights + stepPolicy.hWeight;
                    it.pWeightDevivation = itVars2.pWeightDevivation + stepPolicy.hWeight;
                    it.pBaisDeviation = itVars2.pBaisDeviation + stepPolicy.hBais;
                    if(it.pWeights >= pWeightEnd ) {
                        it.pWeights -= nLayerStepShifts;
                        it.pWeightDevivation -= nLayerStepShifts;
                        it.pBaisDeviation -= sizeConv.batch;
                    }
                }

                it.pIn = itVars1.pIn;
                it.pInDeviation = itVars1.pInDeviation;
                it.pZDeviatioin = itVars1.pZDeviatioin + 1 ;
                it.pWeights = itVars1.pWeights + nLayerStepShifts;
                it.pWeightDevivation = itVars1.pWeightDevivation + nLayerStepShifts;
                it.pBaisDeviation = itVars1.pBaisDeviation + sizeConv.batch;
            }

            //  更新迭代参数
            it.pIn = itVars0.pIn + stepInMove.batch;
            it.pInDeviation = itVars0.pInDeviation + stepInMove.batch;
            it.pOut = itVars0.pOut + stepOut.batch;
            it.pOutDeviation = itVars0.pOutDeviation + stepOut.batch;
        }
    }

private:
    int m_nStrideWidth;
    int m_nStrideHeight;

    //网络参数
    int m_nLayers;
    int m_nInputLayers;

    //输入、输出、卷积尺寸
    CBatchSize3D m_sizeIn;
    CBatchSize3D m_sizeConv;
    CBatchSize3D m_sizeOut;

    //填充尺寸
    CRect2D m_padding;

    //输入、输出、卷积步长
    CBatchSize2D m_stepInMove;
    CBatchSize2D m_stepInConv;
    CBatchSize2D m_stepOut;
    CBatchSize2D m_stepConv;
};

#endif//__SimpleWork_NN_Operators_CConvOperator_h__