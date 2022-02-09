#ifndef __SimpleWork_NN_Operators_CGPoolOperator_h__
#define __SimpleWork_NN_Operators_CGPoolOperator_h__

#include "operator.h"

static SCtx sCtx("CGPoolOperator");
class CGPoolOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CGPoolOperator* pThis = (CGPoolOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;

        Q sumPlane, xPlane = 1.0/nPoolWidth;
        Q* pItIn = (Q*)inVars[0].data;
        Q* pItPlaneIn;
        Q* pItOut = (Q*)outVar.data;
        Q* pItOutEnd;
        while(nBatchs-->0) {
            pItOutEnd = pItOut + nOutBatchSize;
            while(pItOut < pItOutEnd) {
                sumPlane = 0;
                pItPlaneIn = pItIn;
                itPoolWidth = nPoolWidth;
                while(itPoolWidth-->0) {
                    sumPlane += *pItPlaneIn;
                    pItPlaneIn += nOutBatchSize;
                }
                *pItOut = sumPlane*xPlane;
                pItIn++,pItOut++;
            }
            pItIn += nInBatchSize - nOutBatchSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CGPoolOperator* pThis = (CGPoolOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;
        Q dDevia, xPlane = 1.0/nPoolWidth;
        Q* pItInDevia = (Q*)inVars[0].devia;
        Q* pItPlaneDevia;
        Q* pItOutDevia = (Q*)outVar.devia;
        Q* pItOutDeviaEnd;
        while(nBatchs-->0) {
            pItOutDeviaEnd = pItOutDevia + nOutBatchSize;
            while(pItOutDevia<pItOutDeviaEnd) {
                itPoolWidth = nPoolWidth;
                dDevia = *pItOutDevia * xPlane;
                pItPlaneDevia = pItInDevia;
                while(itPoolWidth-->0) {
                    *pItPlaneDevia += dDevia;
                    pItPlaneDevia += nOutBatchSize;
                }
                pItInDevia++, pItOutDevia++;
            }
            pItInDevia += nInBatchSize - nOutBatchSize;
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
            return sCtx.error("全局池化操作需要一个输入数据");
        }

        SDimension spDim = pInVars[0].dimension();
        if(spDim.dataSize() < 2) {
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

        m_nPoolWidth = nInputHeight*nInputHeight;
        return createVariable(SDimension(nDims-2, pDimSizes+2),spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "GPoolSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("poolwidth", m_nPoolWidth);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.GPoolSolver"; }

private:
    int m_nPoolWidth;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGPoolOperator, CGPoolOperator::__getClassKey())
static SNnSolverRegister s_Register("gap", CNnSolver::createSolver<CGPoolOperator>);

#endif//__SimpleWork_NN_Operators_CGPoolOperator_h__