#ifndef __SimpleWork_NN_Operators_CLinearOperator_h__
#define __SimpleWork_NN_Operators_CLinearOperator_h__

#include "operator.h"
static SCtx sCtx("LinearOperator");
class CLinearOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
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
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
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
    }

    int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) {
        if(solveCtx.idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }else if(solveCtx.idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameterData = this;
            return sCtx.success();
        }
        return sCtx.error("????????????");
    }

    int createMat(const PNnLinear* pLinear, const SNnVariable& spIn, SNnVariable& spMat) {
        SDimension spDim = spIn.dimension();
        if(spDim.size() < 1) {
            return sCtx.error("???????????????????????????????????????????????????");
        }

        int pWeightDimSizes[2] = {pLinear->nCells, spDim.data()[spDim.size()-1]};
        spMat = SNnVariable::createWeight({SDimension(2, pWeightDimSizes),0});
        if( !spMat ) {
            return sCtx.error("??????????????????");
        }
        return sCtx.success();
    }

    int createBais(const PNnLinear* pLinear, SNnVariable& spBais) {
        spBais = SNnVariable::createWeight( {SDimension(1, &pLinear->nCells), 0});
        if( !spBais ) {
            return sCtx.error("??????????????????");
        }
        return sCtx.success();
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        const PNnLinear* pLinear = CData<PNnLinear>(pData);
        if(pLinear == nullptr) {
            return sCtx.error("??????????????????");
        }
        SNnVariable spIn, spMat, spBais;
        if(nInVars == 1) {
            spIn = pInVars[0];
            if( createMat(pLinear, spIn, spMat) != sCtx.success() ) {
                return sCtx.error("??????????????????????????????");
            }
            if(pLinear->bBais) {
                if( createBais(pLinear, spBais) != sCtx.success() ) {
                    return sCtx.error("??????????????????");
                }
            }
        }else if(nInVars == 2) {
            spIn = pInVars[0];
            spMat = pInVars[1];
        }else{
            return sCtx.error("????????????????????????");
        }
        SDimension spDim1 = spIn.dimension();
        SDimension spDim2 = spMat.dimension();
        if(spDim2.size() != 2 ) {
            return sCtx.error("????????????????????????????????????????????????");
        }

        int nDim1 = spDim1.size();
        int nDim2 = spDim2.size();
        if(nDim1 < 1) {
            return sCtx.error("???????????????????????????????????????");
        }

        int nInputSize = spDim1.data()[nDim1-1];
        int nOutputSize = spDim2.data()[0];
        if( spDim1.data()[nDim1-1] != spDim2.data()[1] ) {
            return sCtx.error("????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????");
        }

        nIn = nInputSize;
        nOut = nOutputSize;
        nMat = nIn * nOut;
        SDimension spOutDim = spDim1.downLowDimension().upLowDimension(nOutputSize);
        createVariable(spOutDim, spVarOut);

        SNnVariable pLinearVars[] = { spIn, spMat };
        addAtomOperator(this, 2, pLinearVars, spVarOut);
        if(spBais) {
            spVarOut = spVarOut + spBais;
        }
        if(pLinear->szActivator && pLinear->szActivator[0] != 0){
            spVarOut = spVarOut.solveOp(pLinear->szActivator);
        }
        return sCtx.success();
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getName() { return "LinearSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("nin", nIn);
        ar.arBlock("nmat", nMat);
        ar.arBlock("nout", nOut);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.LinearSolver"; }


private:
    //???????????????????????????????????????????????????????????????????????????????????????????????????
    int nIn;
    int nMat;
    int nOut;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CLinearOperator, CLinearOperator::__getClassKey())
static SNnSolverRegister s_Register("linear", CNnSolver::createSolver<CLinearOperator>);

#endif//__SimpleWork_NN_Operators_CLinearOperator_h__