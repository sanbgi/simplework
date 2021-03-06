#ifndef __SimpleWork_NN_Operators_CProductOperator_h__
#define __SimpleWork_NN_Operators_CProductOperator_h__

#include "operator.h"
static SCtx sCtx("ProductOperator");
class CProductOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        PSolveData* pThis = (PSolveData*)pParameters;
        int nIn = pThis->nIn;
        int nMat = pThis->nMat;
        int nOut = pThis->nOut;
        Q* pIn = (Q*)inVars[0].data;
        Q* pMat = (Q*)inVars[1].data;
        Q* pOut = (Q*)outVar.data;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pMatEnd = pMat + inVars[1].size;
        Q* pItIn, *pItMat;
        Q* pItInEnd, *pItOutEnd;
        Q v;
        while(nBatchs-->0) {
            pItMat = pMat;
            pItOutEnd = pOut + nOut;
            while(pOut != pItOutEnd) {
                v = 0;
                pItIn = pIn;
                pItInEnd = pIn + nIn;
                while(pItIn < pItInEnd) {
                    v += (*pItIn) * (*pItMat);
                    pItIn++, pItMat++;
                }
                *pOut = v;
                pOut++;
            }
            pIn += nIn;
            pMat += nMat;
            if(pIn == pInEnd) {
                pIn = (Q*)inVars[0].data;
            }
            if(pMat == pMatEnd) {
                pMat = (Q*)inVars[1].data;
            }
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        PSolveData* pThis = (PSolveData*)pParameters;
        int nIn = pThis->nIn;
        int nMat = pThis->nMat;
        int nOut = pThis->nOut;
        Q* pIn = (Q*)inVars[0].data;
        Q* pInDeiva = (Q*)inVars[0].devia;
        Q* pMat = (Q*)inVars[1].data;
        Q* pMatDevia = (Q*)inVars[1].devia;
        Q* pOutDevia = (Q*)outVar.devia;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pMatEnd = pMat + inVars[1].size;

        Q* pItInEnd, *pOutDeviaEnd;
        Q* pItIn, *pItInDevia;
        Q deviationOut;
        Q* pItMat;
        Q* pItMatDevia;
        while(nBatchs-->0) {
            pItMat = pMat;
            pItMatDevia = pMatDevia;
            pOutDeviaEnd = pOutDevia + nOut;
            while(pOutDevia != pOutDeviaEnd) {
                pItIn = pIn;
                pItInDevia = pInDeiva;
                deviationOut = *pOutDevia;
                pItInEnd = pIn + nIn;
                while(pItIn < pItInEnd) {
                    *pItInDevia += deviationOut * (*pItMat);
                    *pItMatDevia += deviationOut * (*pItIn);
                    pItIn++, pItInDevia++, pItMat++, pItMatDevia++;
                }
                pOutDevia++;
            }
            pIn += nIn;
            pInDeiva += nIn;
            pMat += nMat;
            pMatDevia += nMat;
            if(pIn == pInEnd) {
                pIn = (Q*)inVars[0].data;
                pInDeiva = (Q*)inVars[0].devia;
            }
            if(pMat == pMatEnd) {
                pMat = (Q*)inVars[1].data;
                pMatDevia = (Q*)inVars[1].devia;
            }
        }
        //VERIFY(pMatDevia - (Q*)inVars[1].devia == inVars[1].size )
    }

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        solveParameter.nParamterSize = sizeof(PSolveData);
        solveParameter.pParameterData = &m_sData;
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
        return sCtx.error("????????????");
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        if(spDim2.size() != 2 ) {
            return sCtx.error("????????????????????????????????????");
        }

        int nInElementSize = spDim1.dataSize();
        const int* pDimSize2 = spDim2.data();
        if(nInElementSize != pDimSize2[1]) {
            return sCtx.error("???????????????????????????????????????");
        }
        m_sData.nIn = spDim1.dataSize();
        m_sData.nMat = spDim2.dataSize();
        m_sData.nOut = m_sData.nMat / m_sData.nIn;
        createVariable(SDimension(1,pDimSize2), spVarOut);
        return addAtomOperator(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "Product"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("data", m_sData);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.ProductSolver"; }

private:
    struct PSolveData{
        int nIn;
        int nMat;
        int nOut;
    }m_sData;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CProductOperator, CProductOperator::__getClassKey())
static SNnSolverRegister s_Register("product", CNnSolver::createSolver<CProductOperator>);

#endif//__SimpleWork_NN_Operators_CProductOperator_h__