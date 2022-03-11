#ifndef __SimpleWork_NN_Operators_CBatchNormalizeOperator_h__
#define __SimpleWork_NN_Operators_CBatchNormalizeOperator_h__

#include "math.h"
#include "operator.h"
static SCtx sCtx("BatchNormalizeOperator");
class CBatchNormalizeOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)

public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==1)
        PSolveData* pThis = (PSolveData*)pParameters;
        int nLayers = pThis->m_nLayers;
        int nTensors = outVar.size / nLayers;
        Q esp = (Q)pThis->m_dEsp;
        Q* pIn = (Q*)inVars[0].data;
        Q* pAvg = (Q*)pThis->m_pAvg;
        Q* pVariance = pAvg + pThis->m_nLayers;
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
            x = 1.0 / (Q)sqrt(*pItVariance + esp);
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
        PSolveData* pThis = (PSolveData*)pParameters;
        int nLayers = pThis->m_nLayers;
        Q esp = (Q)pThis->m_dEsp;
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pAvg = (Q*)pThis->m_pAvg;
        Q* pVariance = pAvg + pThis->m_nLayers;
        Q* pOutDevia = (Q*)outVar.devia;
        Q* pItInDeviaEnd = pInDevia + outVar.size;
        Q* pItInDevia, *pItVariance, *pItOutDevia;
        Q x;
        int nItLayer;

        //求解标注化结果
        pItVariance = pVariance;
        nItLayer = nLayers;
        while(nItLayer-->0) {
            x = 1.0 / (Q)sqrt(*pItVariance + esp);
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

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        if(solveCtx.idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            PSolveData* pSolveData = (PSolveData*)m_spBuffer.data<char>();
            int nBuffer = sizeof(PSolveData) + 2 * pSolveData->m_nLayers * sizeof(float);
            if( nBuffer != m_spBuffer.size() ) {
                STensor spBuffer;
                if( STensor::createVector<char>(spBuffer, nBuffer) != sCtx.success() ) {
                    return sCtx.error("创建张量失败");
                }

                PSolveData* pBuffer = (PSolveData*)spBuffer.data<char>();
                (*pBuffer) = *pSolveData;
                pSolveData = pBuffer;
                m_spBuffer = spBuffer;
                
                float* pAvg = (float*)pBuffer->m_pAvg;
                float* pVariance = pAvg + pBuffer->m_nLayers;
                memset(pAvg, 0, sizeof(float)*pSolveData->m_nLayers);
                memset(pVariance, 1, sizeof(float)*pSolveData->m_nLayers);
            }
            solveParameter.nParamterSize = sizeof(PSolveData);
            solveParameter.pParameterData = pSolveData;
            solveParameter.eClRange = PSolveFunc::PCustomer;
            solveParameter.nCustomerRange = pSolveData->m_nLayers;
            return sCtx.success();
        }else if(solveCtx.idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            PSolveData* pSolveData = (PSolveData*)m_spBuffer.data<char>();
            int nBuffer = sizeof(PSolveData) + 2 * pSolveData->m_nLayers * sizeof(double);
            if( nBuffer != m_spBuffer.size() ) {
                STensor spBuffer;
                if( STensor::createVector<char>(spBuffer, nBuffer) != sCtx.success() ) {
                    return sCtx.error("创建张量失败");
                }

                PSolveData* pBuffer = (PSolveData*)spBuffer.data<char>();
                (*pBuffer) = *pSolveData;
                pSolveData = pBuffer;
                m_spBuffer = spBuffer;
                
                double* pAvg = (double*)pBuffer->m_pAvg;
                double* pVariance = pAvg + pBuffer->m_nLayers;
                memset(pAvg, 0, sizeof(double)*pSolveData->m_nLayers);
                memset(pVariance, 1, sizeof(double)*pSolveData->m_nLayers);
            }
            solveParameter.nParamterSize = sizeof(PSolveData);
            solveParameter.pParameterData = pSolveData;
            solveParameter.eClRange = PSolveFunc::PCustomer;
            solveParameter.nCustomerRange = pSolveData->m_nLayers;
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

        if( STensor::createVector<char>(m_spBuffer, sizeof(PSolveData)) != sCtx.success() ) {
            return sCtx.error("创建缓冲错误");
        }

        PSolveData* pSolveData = (PSolveData*)m_spBuffer.data<char>();
        pSolveData->m_nLayers = nLayers;
        pSolveData->m_dEsp = pParameter->dEsp;
        pSolveData->m_nMinBatch = pParameter->nMinBatch;
        createVariable(spDim,spVarOut);
        return addAtomOperator(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "BatchNormalize"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arObject("buffer", m_spBuffer);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.BatchNormalizeSolver"; }

private:
    STensor m_spBuffer;
    struct PSolveData{
        int m_nLayers;
        double m_dEsp;
        int m_nMinBatch;
        char m_pAvg[0];
    };
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CBatchNormalizeOperator, CBatchNormalizeOperator::__getClassKey())
static SNnSolverRegister s_Register("batchnormalize", CNnSolver::createSolver<CBatchNormalizeOperator>);

#endif//__SimpleWork_NN_Operators_CBatchNormalizeOperator_h__