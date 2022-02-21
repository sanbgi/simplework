#ifndef __SimpleWork_NN_Operators_CGMPOperator_h__
#define __SimpleWork_NN_Operators_CGMPOperator_h__

#include "operator.h"

static SCtx sCtx("CGMPOperator");
class CGMPOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CGMPOperator* pThis = (CGMPOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;

        Q maxPlane;
        Q* pItIn = (Q*)inVars[0].data;
        Q* pItPlaneIn;
        Q* pItOut = (Q*)outVar.data;
        Q* pItOutEnd;
        while(nBatchs-->0) {
            pItOutEnd = pItOut + nOutBatchSize;
            while(pItOut < pItOutEnd) {
                maxPlane = *pItIn;
                pItPlaneIn = pItIn+nOutBatchSize;
                itPoolWidth = nPoolWidth-1;
                while(itPoolWidth-->0) {
                    if(*pItPlaneIn > maxPlane)
                        maxPlane = *pItPlaneIn;
                    pItPlaneIn += nOutBatchSize;
                }
                *pItOut = maxPlane;
                pItIn++,pItOut++;
            }
            pItIn += nInBatchSize - nOutBatchSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CGMPOperator* pThis = (CGMPOperator*)pParameters;
        int itPoolWidth;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nOutBatchSize = outVar.size / nBatchs;
        int nInBatchSize = nPoolWidth * nOutBatchSize;
        Q maxPlane;
        Q* pInDevia;

        Q* pItIn = (Q*)inVars[0].data;
        Q* pItPlaneIn;
        Q* pItInDevia = (Q*)inVars[0].devia;
        Q* pItPlaneInDevia;
        Q* pItOutDevia = (Q*)outVar.devia;
        Q* pItOutDeviaEnd;
        while(nBatchs-->0) {
            pItOutDeviaEnd = pItOutDevia + nOutBatchSize;
            while(pItOutDevia<pItOutDeviaEnd) {
                maxPlane = *pItIn;
                pInDevia = pItInDevia;
                pItPlaneIn = pItIn + nOutBatchSize;
                pItPlaneInDevia = pItInDevia + nOutBatchSize;
                itPoolWidth = nPoolWidth-1;
                while(itPoolWidth-->0) {
                    if(*pItPlaneIn > maxPlane){
                        maxPlane = *pItPlaneIn;
                        pInDevia = pItPlaneInDevia;
                    }

                    pItPlaneIn += nOutBatchSize;
                    pItPlaneInDevia += nOutBatchSize;
                }

                *pInDevia += *pItOutDevia;
                pItIn++, pItInDevia++, pItOutDevia++;
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
        createVariable(SDimension(nDims-2, pDimSizes+2),spVarOut);
        return addAtomOperator(this, nInVars, pInVars, spVarOut);
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

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGMPOperator, CGMPOperator::__getClassKey())
static SNnSolverRegister s_Register("gmp", CNnSolver::createSolver<CGMPOperator>);

#endif//__SimpleWork_NN_Operators_CGMPOperator_h__