#ifndef __SimpleWork_NN_Operators_CBatchNormalizeOperator_h__
#define __SimpleWork_NN_Operators_CBatchNormalizeOperator_h__

#include "math.h"
#include "operator.h"
static SCtx sCtx("BatchNormalizeOperator");
class CBatchNormalizeOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)

public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==1)
        CBatchNormalizeOperator* pThis = (CBatchNormalizeOperator*)pParameters;
        int nLayers = pThis->m_nLayers;
        int nTensors = outVar.size / nLayers;
        Q esp = pThis->m_dEsp;
        Q* pIn = (Q*)inVars[0].data;
        Q* pAvg = (Q*)pThis->m_pAvg;
        Q* pVariance = (Q*)pThis->m_pVariance;
        Q* pOut = (Q*)outVar.data;
        Q* pItInEnd = pIn + outVar.size;
        Q* pItIn, *pItAvg, *pItVariance, *pItOut;
        Q x;
        int nItLayer;

        //
        // 计算批量均值和方差
        //
        if(nBatchs >= pThis->m_nMinBatch) {
            Q avg, delta, variance;
            pItAvg = pAvg;
            pItVariance = pVariance;
            nItLayer = nLayers;
            while(nItLayer-->0) {
                //求均值
                avg = 0;
                pItIn = pIn;
                while(pItIn < pItInEnd) {
                    avg += *pItIn;
                    pItIn += nLayers;
                }
                avg = avg / nTensors;

                //求方差
                variance = 0;
                pItIn = pIn;
                while(pItIn < pItInEnd) {
                    delta = *pItIn - avg;
                    variance += delta*delta;
                    pItIn += nLayers;
                }
                variance = variance / nTensors;

                *pItAvg = avg;
                *pItVariance = variance;
                pIn++, pItAvg++, pItVariance++;
            }

            // 恢复变量
            nLayers = pThis->m_nLayers;
            pIn = (Q*)inVars[0].data;
        }

        //求解标准化结果
        pItAvg = pAvg;
        pItVariance = pVariance;
        nItLayer = nLayers;
        while(nItLayer-->0) {
            x = 1.0 / sqrt(*pItVariance + esp);
            pItIn = pIn;
            pItOut = pOut;
            while(pItIn < pItInEnd) {
                *pItOut = (*pItIn - *pItAvg) * x;
                pItIn += nLayers;
                pItOut += nLayers;
            }
            pIn++, pOut++, pItAvg++, pItVariance++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        CBatchNormalizeOperator* pThis = (CBatchNormalizeOperator*)pParameters;
        int nLayers = pThis->m_nLayers;
        Q esp = pThis->m_dEsp;
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pVariance = (Q*)pThis->m_pVariance;
        Q* pOutDevia = (Q*)outVar.devia;
        Q* pItInDeviaEnd = pInDevia + outVar.size;
        Q* pItInDevia, *pItVariance, *pItOutDevia;
        Q x;
        int nItLayer;

        //求解标注化结果
        pItVariance = pVariance;
        nItLayer = nLayers;
        while(nItLayer-->0) {
            x = 1.0 / sqrt(*pItVariance + esp);
            pItInDevia = pInDevia;
            pItOutDevia = pOutDevia;
            while(pItInDevia < pItInDeviaEnd) {
                *pItInDevia += x * (*pItOutDevia);
                pItInDevia += nLayers;
                pItOutDevia += nLayers;
            }
            pInDevia++, pOutDevia++, pItVariance++;
        }
    }

    int prepareSolver(unsigned int idType, PSolveParameter& solveParameter) {
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameter = this;
            if(!(m_spAvgTensor)) {
                if( STensor::createVector<float>(m_spAvgTensor, m_nLayers) != sCtx.success() ||
                    STensor::createVector<float>(m_spVarianceTensor, m_nLayers) != sCtx.success() ) {
                    return sCtx.error("创建张量失败");
                }
                m_pAvg = (m_spAvgTensor).data<float>();
                m_pVariance = (m_spVarianceTensor).data<float>();
                memset(m_pAvg, 0, sizeof(float)*m_nLayers);
                memset(m_pVariance, 1, sizeof(float)*m_nLayers);
            }else{
                m_pAvg = (m_spAvgTensor).data<float>();
                m_pVariance = (m_spVarianceTensor).data<float>();
            }
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameter = this;
            if(!(m_spAvgTensor)) {
                if( STensor::createVector<double>(m_spAvgTensor, m_nLayers) != sCtx.success() ||
                    STensor::createVector<double>(m_spVarianceTensor, m_nLayers) != sCtx.success() ) {
                    return sCtx.error("创建张量失败");
                }
                m_pAvg = (m_spAvgTensor).data<double>();
                m_pVariance = (m_spVarianceTensor).data<double>();
                memset(m_pAvg, 0, sizeof(double)*m_nLayers);
                memset(m_pVariance, 1, sizeof(double)*m_nLayers);
            }else{
                m_pAvg = (m_spAvgTensor).data<double>();
                m_pVariance = (m_spVarianceTensor).data<double>();
            }
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        const PNnBatchNormalize* pParameter = CData<PNnBatchNormalize>(pData);
        if( pParameter == nullptr ) {
            return sCtx.error("缺少构造参数");
        }

        if(nInVars != 1) {
            return sCtx.error("批量标准化操作需要一个输入数据");
        }

        SDimension spDim = pInVars[0].dimension();
        if(spDim.size() < 2) {
            return sCtx.error("池化操作输入张量至少需要二维，代表高度和宽度");
        }
        int nDims = spDim.size();
        const int* pDimSizes = spDim.data();
        int nInputHeight = pDimSizes[0];
        int nInputWidth = pDimSizes[1];
        int nLayers = 1;
        for( int i=2; i<nDims; i++ ) {
            nLayers *= pDimSizes[i];
        }

        m_nLayers = nLayers;
        m_dEsp = pParameter->dEsp;
        m_nMinBatch = pParameter->nMinBatch;
        createVariable(spDim,spVarOut);
        return addAtomSolver(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "BatchNormalizeSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("nlayer", m_nLayers);
        ar.arBlock("esp", m_dEsp);
        ar.arObject("avg", m_spAvgTensor);
        ar.arObject("variance", m_spVarianceTensor);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.BatchNormalizeSolver"; }

private:
    int m_nLayers;
    double m_dEsp;
    void* m_pAvg;
    void* m_pVariance;
    int m_nMinBatch;
    STensor m_spAvgTensor;
    STensor m_spVarianceTensor;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CBatchNormalizeOperator, CBatchNormalizeOperator::__getClassKey())
static SNnSolverRegister s_Register("batchnormalize", CNnSolver::createSolver<CBatchNormalizeOperator>);

#endif//__SimpleWork_NN_Operators_CBatchNormalizeOperator_h__