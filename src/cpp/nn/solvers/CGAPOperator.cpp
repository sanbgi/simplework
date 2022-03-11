#ifndef __SimpleWork_NN_Operators_CGAPOperator_h__
#define __SimpleWork_NN_Operators_CGAPOperator_h__

#include "operator.h"

static SCtx sCtx("CGAPOperator");
class CGAPOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CGAPOperator* pThis = (CGAPOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;

        Q sumPlane, xPlane = (Q)1.0/nPoolWidth;
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
        CGAPOperator* pThis = (CGAPOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;
        Q dDevia, xPlane = (Q)1.0/nPoolWidth;
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

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        solveParameter.nParamterSize = 0;
        solveParameter.pParameterData = this;
        solveParameter.eClRange = PSolveFunc::POut;
        if(solveCtx.idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            return sCtx.success();
        }else if(solveCtx.idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
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
        createVariable(SDimension(nDims-2, pDimSizes+2),spVarOut);
        return addAtomOperator(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "Gap"; } 
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

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGAPOperator, CGAPOperator::__getClassKey())
static SNnSolverRegister s_Register("gap", CNnSolver::createSolver<CGAPOperator>);

#endif//__SimpleWork_NN_Operators_CGAPOperator_h__