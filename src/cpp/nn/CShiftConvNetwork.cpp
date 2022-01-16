#include "CShiftConvNetwork.h"
#include "CType.h"
#include "math.h"

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


static SCtx sCtx("CShiftConvNetwork");
int CShiftConvNetwork::createNetwork(int nWidth, int nHeight, int nLayers, int nShiftConvs, const char* szPadding, const char* szActivator, SNnNetwork& spNetwork) {

    if(nWidth < 1 || nHeight < 1 || nShiftConvs < 1 || nLayers < 1) {
        return sCtx.error("卷积核参数错误");
    }

    CPointer<CShiftConvNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_nLayers = nLayers;
    spConvolution->m_sizeConv = {
        nShiftConvs,
        nHeight,
        nWidth,
        0
    };
    spConvolution->m_nStrideWidth = 1;
    spConvolution->m_nStrideHeight = 1;
    if( szActivator!=nullptr )
        spConvolution->m_strActivator = szActivator;
    if( szPadding != nullptr) {
        spConvolution->m_strPadding = szPadding;
    }
    spNetwork.setPtr(spConvolution.getPtr());
    return sCtx.success();
}


int CShiftConvNetwork::prepareNetwork(const STensor& spBatchIn) {
    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn.size();
    unsigned int idType = spBatchIn.type();
    if( nInputSize == m_nInputSize && m_idDataType == idType) {
        return sCtx.success();
    }

    //
    // 计算参数
    //
    int nBatchs, nInputCells, nInputWidth, nInputHeight, nInputLayers;
    {
        STensor& spInDimVector = spBatchIn->getDimVector();
        int nInputDims = spInDimVector->getDataSize();
        int* pInDimSizes = spInDimVector->getDataPtr<int>();

        //
        // 维度小于3的张量，无法进行卷积运算
        //
        if(nInputDims < 3) {
            return sCtx.error("卷积网络的输入张量维度，必须大于等于3，其中第一个维度为张量个数，第二三个维度为卷积运算高和宽");
        }

        nBatchs = pInDimSizes[0];
        nInputHeight = pInDimSizes[1];
        nInputWidth = pInDimSizes[2];
        if( nInputHeight < m_sizeConv.height || nInputWidth < m_sizeConv.width ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        nInputLayers = 1;
        for( int i=3; i<nInputDims; i++ ) {
            nInputLayers *= pInDimSizes[i];
        }
        nInputCells = nInputWidth*nInputHeight*nInputLayers;
        if(nBatchs * nInputCells != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 判断是否需要初始化网络
    //
    if(m_nInputLayers == 0) {

        if( m_sizeConv.batch <= 0 || m_sizeConv.width < 1 || m_sizeConv.height < 1 ) {
            return sCtx.error("卷积核参数错误");
        }

        int nConvs = m_nLayers * m_sizeConv.batch;
        int nWeights =  nConvs * m_sizeConv.width * m_sizeConv.height * nInputLayers;
        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nWeights, nConvs);
        }else if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nWeights, nConvs);
        }else{
            return sCtx.error("不支持的数据类型");
        }

        m_nInputSize = 0; //通过这个值的设置，实现之后的运行时参数必须重新初始化
        m_nInputLayers = nInputLayers;
        m_idDataType = idType;
    }else {
        if( m_nInputLayers != nInputLayers ) {
            //
            // TODO: 重新初始化权值?(如果重新初始化，则已有权值将被清空，需要小心)
            //
            return sCtx.error("输入张量的层数与初始化时不一致");
        }

        if(m_idDataType != idType) {
            //
            // TODO：权值类型改变？
            //
            return sCtx.error("输入张量的类型与初始化时不一致");
        }
    }

    m_sizeConv.layers = nInputLayers;
    m_sizeIn = {
        nBatchs,
        nInputHeight,
        nInputWidth,
        nInputLayers
    };

    if(m_strPadding == "same" ) {
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

    STensor spOutDimVector;
    if( STensor::createVector<int>(spOutDimVector, 4, (int*)&m_sizeOut) != sCtx.success() ) {
        return sCtx.error("创建输出张量的维度向量失败");
    }

    if( STensor::createTensor(m_spBatchOut, spOutDimVector, idType, nBatchs * m_stepOut.batch) != sCtx.success() ){
        return sCtx.error("创建输出张量失败");
    }
    
    m_pActivator = CActivator::getActivation(idType, m_strActivator.c_str());
    if(m_pActivator == nullptr) {
        return sCtx.error((std::string("不支持的激活函数名: ") + m_strActivator).c_str());
    }

    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    m_spBatchInDeviation.release();
    m_nInputSize = nInputSize;
    return sCtx.success();
}

template<typename Q> int CShiftConvNetwork::initWeightT(int nWeights, int nConvs) {
    Q* pWeights = new Q[nWeights];
    Q* pBais = new Q[nConvs];
    m_spWeights.take( (char*)pWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spBais.take((char*)pBais, [](char* pBais){
        delete[] (Q*)pBais;
    });

    //
    // TODO：权重初始化采用什么策略？nadam优化算法下，即便所有权重初始值为零，仍然不影响迭代
    //
    Q xWeight = 0.1;//sqrt(1.0/(m_nConvWidth*m_nConvHeight*nInLayers));
    for(int i=0; i<nWeights; i++) {
        //pWeights[i] = 0;
        pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
    }

    for(int i=0; i<m_sizeConv.batch; i++ ){
        pBais[i] = 0;
    }

    return 0;
}

template<typename Q> int CShiftConvNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {

    CBatchSize3D sizeIn = m_sizeIn;
    CBatchSize3D sizeOut = m_sizeOut;
    CBatchSize3D sizeConv = m_sizeConv;
    CBatchSize2D stepInMove = m_stepInMove;
    CBatchSize2D stepInConv = m_stepInConv;
    CBatchSize2D stepOut = m_stepOut;
    CBatchSize2D stepConv = m_stepConv;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pWeights;
        Q* pBais;
        int index;
    }itVars, itVars0, itVars2, itVars3, itVars1, itVars4, itVars5, itVars6, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        (Q*)(void*)m_spBais,
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
    int nLayers = m_nLayers;
    int nLayerStepShifts = sizeConv.batch * stepConv.batch;
    Q dConv, *pWeightEnd;
    itVars = it;

    CRect2D rcConv, rcPading = m_padding;

    //将输入指针向Padding后的起点偏移，变成一个非法指针
    int nOffset = m_padding.left * stepInConv.width - m_padding.top * stepInConv.height;
    it.pIn = it.pIn - nOffset;
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

                //左边填充了都填充了控制，不能参与运算
                if(itVars2.index < rcPading.top) {
                    rcConv.top = rcPading.top;
                    rcConv.bottom = sizeConv.height;
                }else if(itVars2.index >= sizeOut.height - rcPading.bottom) {
                    rcConv.top = 0;
                    rcConv.bottom = sizeConv.height - (itVars2.index - sizeOut.height + 1 + rcPading.bottom);
                }else{
                    rcConv.top = 0;
                    rcConv.bottom = sizeConv.height;
                }

                for(itVars3.index=0; itVars3.index < sizeOut.width; itVars3.index++) {
                    itVars3.pIn = it.pIn;
                    itVars3.pOut = it.pOut;
                    itVars3.pWeights = it.pWeights;
                    itVars3.pBais = it.pBais;

                    //左边填充了都填充了控制，不能参与运算
                    if(itVars3.index < rcPading.left) {
                        rcConv.left = rcPading.left;
                        rcConv.right = sizeConv.width;
                    }else if(itVars3.index >= sizeOut.width - rcPading.right) {
                        rcConv.left = 0;
                        rcConv.right = sizeConv.width - (itVars3.index - sizeOut.width + 1 + rcPading.right);
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
        
        //
        // 激活结果值
        //
        m_pActivator->activate(stepOut.batch, itVars0.pOut, itVars0.pOut);

        it.pIn = itVars0.pIn + stepInMove.batch;
        it.pOut = itVars0.pOut + stepOut.batch;
    }

    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CShiftConvNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( prepareNetwork(spBatchIn) != sCtx.success() ) {
        return sCtx.error();
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }

    return sCtx.error("数据类型不支持");
}


/*
    #include <sys/time.h>
    using namespace std;
    timeval t1, t2, t3; 
    gettimeofday(&t1, nullptr);
    gettimeofday(&t2, nullptr);
    gettimeofday(&t3, nullptr);
    long dt1 = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
    long dt2 = (t3.tv_sec-t2.tv_sec)*1000000+(t3.tv_usec-t2.tv_usec);
*/
template<typename Q> int CShiftConvNetwork::learnT(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {

    if(!m_spBatchInDeviation) {
        if( int errCode = STensor::createTensor<Q>(m_spBatchInDeviation, m_spBatchIn->getDimVector(), m_spBatchIn.size()) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输入偏差张量失败");
        }
    }
    spBatchIn = m_spBatchIn;
    spInDeviation = m_spBatchInDeviation;

    CBatchSize3D sizeIn = m_sizeIn;
    CBatchSize3D sizeOut = m_sizeOut;
    CBatchSize3D sizeConv = m_sizeConv;
    CBatchSize2D stepInMove = m_stepInMove;
    CBatchSize2D stepInConv = m_stepInConv;
    CBatchSize2D stepOut = m_stepOut;
    CBatchSize2D stepConv = m_stepConv;

    int nConvs = sizeConv.batch * m_nLayers;
    int nWeights = stepConv.batch * sizeConv.batch * m_nLayers;
    Q* pWeightDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nWeights+nConvs);
    Q* pBaisDeviationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(Q)*(nWeights+nConvs));

    CShiftPolicy stepPolicy;
    CShiftPolicies shiftPolicies;
    s_GetShiftPolicy(shiftPolicies, sizeConv.batch, stepConv.batch);
    int wWeight, hWeight;

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
        spBatchIn->getDataPtr<Q>(),
        spInDeviation->getDataPtr<Q>(),
        spBatchOut->getDataPtr<Q>(),
        spOutDeviation->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        pWeightDerivationArray,
        pBaisDeviationArray
    }, itVars, itVars0,itVars2,itVars3,itVars1,itVars4,itVars5,itVars6;
    memset(it.pInDeviation, 0, sizeof(Q)*stepInConv.batch * sizeIn.batch);

    Q deviationZ;
    Q pDeviationZArray[stepOut.batch];
    int nLayers = m_nLayers;
    int nLayerStepShifts = sizeConv.batch * stepConv.batch;
    Q* pWeightEnd;

    //
    //
    //  注意，
    //      权重卷积核大小 : nLayers * nShiftConvs * h * w * l
    //      偏置大小：nLayers * nShiftConvs 
    //
    itVars = it;
    CRect2D rcConv, rcPading = m_padding;

    //将输入指针向Padding后的起点偏移，变成一个非法指针
    int nOffset = m_padding.left * stepInConv.width + m_padding.top * stepInConv.height;
    it.pIn = it.pIn - nOffset;
    it.pInDeviation = it.pInDeviation - nOffset;
    for(itVars0.index=0; itVars0.index<sizeIn.batch; itVars0.index++) {
        itVars0.pIn = it.pIn;
        itVars0.pInDeviation = it.pInDeviation;
        itVars0.pOut = it.pOut;
        itVars0.pOutDeviation = it.pOutDeviation; 

        m_pActivator->deactivate(stepOut.batch, it.pOut, it.pOutDeviation, pDeviationZArray);
        it.pZDeviatioin = pDeviationZArray;             
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

                //左边填充了都填充了控制，不能参与运算
                if(itVars2.index < rcPading.top) {
                    rcConv.top = rcPading.top;
                    rcConv.bottom = sizeConv.height;
                }else if(itVars2.index >= sizeOut.height - rcPading.bottom) {
                    rcConv.top = 0;
                    rcConv.bottom = sizeConv.height - (itVars2.index - sizeOut.height + 1 + rcPading.bottom);
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
                        //左右填充了都填充了控制，不能参与运算
                        if(itVars3.index < rcPading.left) {
                            rcConv.left = rcPading.left;
                            rcConv.right = sizeConv.width;
                        }else if(itVars3.index >= sizeOut.width - rcPading.right) {
                            rcConv.left = 0;
                            rcConv.right = sizeConv.width - (itVars3.index - sizeOut.width + 1 + rcPading.right);
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
    it = itVars;
    
    //
    // 用优化器优化偏差值
    //
    m_spOptimizer->updateDeviation(sizeIn.batch);

    // 
    // 权重值更新，需要在输入偏导数计算完成后进行，否则，中间会影响输入偏导数值
    //
    while(it.pWeights != pWeightEnd) {
        *it.pWeights -= *it.pWeightDevivation;
        it.pWeights++, it.pWeightDevivation++;
    }
    Q* pBais = (Q*)(void*)m_spBais;
    Q* pBaisEnd = pBais+nConvs;
    while(pBais != pBaisEnd) {
        *pBais -= *it.pBaisDeviation;
        pBais++, it.pBaisDeviation++;
    }
    return sCtx.success();
}

int CShiftConvNetwork::learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    if(spOutDeviation.type() != m_idDataType) {
        return sCtx.error("数据类型错误");
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return learnT<double>(spBatchOut, spOutDeviation, spBatchIn, spInDeviation);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return learnT<float>(spBatchOut, spOutDeviation, spBatchIn, spInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

int CShiftConvNetwork::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("layers", m_nLayers);
    ar.visit("shiftConvs", m_sizeConv.batch);
    ar.visit("dropout", m_sizeConv.batch);
    ar.visit("width", m_sizeConv.width);
    ar.visit("height", m_sizeConv.height);
    ar.visit("strideWidth", m_nStrideWidth);
    ar.visit("strideHeight", m_nStrideHeight);
    ar.visitString("activator", m_strActivator);
    ar.visitString("optimizer", m_strOptimizer);
    ar.visitString("padding", m_strPadding);

    //运行参数
    ar.visit("inputLayers", m_nInputLayers);
    ar.visit("dataType", m_idDataType);
    if(m_nInputLayers) {
        int nBytes = CType::getTypeBytes(m_idDataType);
        ar.visitTaker("weights", nBytes * m_nInputLayers * m_sizeConv.width * m_sizeConv.height * m_sizeConv.batch * m_nLayers, m_spWeights);
        ar.visitTaker("bais", nBytes * m_sizeConv.batch * m_nLayers, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CShiftConvNetwork, CShiftConvNetwork::__getClassKey())