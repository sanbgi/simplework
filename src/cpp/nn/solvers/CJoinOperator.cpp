#ifndef __SimpleWork_NN_Operators_CJoinOperator_h__
#define __SimpleWork_NN_Operators_CJoinOperator_h__

#include "operator.h"
static SCtx sCtx("JoinOperator");
class CJoinOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size+inVars[1].size==outVar.size)
        CJoinOperator* pThis = (CJoinOperator*)pParameters;

        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn1End = pIn1 + inVars[0].size;
        int nIn1 = pThis->m_nSize1;

        Q* pIn2 = (Q*)inVars[1].data;
        Q* pIn2End = pIn2 + inVars[1].size;
        int nIn2 = pThis->m_nSize2;

        int iIn1, iIn2;
        Q* pO = (Q*)outVar.data;
        Q* pOEnd = pO + outVar.size;
        while(pO < pOEnd) {
            iIn1 = nIn1;
            while(iIn1-->0) {
                *pO = *pIn1;
                pO++, pIn1++;
            }

            iIn2 = nIn2;
            while(iIn2-->0) {
                *pO = *pIn2;
                pO++, pIn2++;
            }

            if(pIn1 == pIn1End) {
                pIn1 = (Q*)inVars[0].data;
            }
            if(pIn2 == pIn2End) {
                pIn2 = (Q*)inVars[1].data;
            }
        }
        VERIFY(pO-(Q*)outVar.data==outVar.size)
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        VERIFY(inVars[0].size+inVars[1].size==outVar.size)
        CJoinOperator* pThis = (CJoinOperator*)pParameters;

        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia1End = pDevia1 + inVars[0].size;
        int nIn1 = pThis->m_nSize1;

        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDevia2End = pDevia2 + inVars[1].size;
        int nIn2 = pThis->m_nSize2;

        int iIn1, iIn2;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDeviaOEnd = pDeviaO + outVar.size;
        while(pDeviaO < pDeviaOEnd) {
            iIn1 = nIn1;
            while(iIn1-->0) {
                *pDevia1 += *pDeviaO;
                pDevia1++, pDeviaO++;
            }

            iIn2 = nIn2;
            while(iIn2-->0) {
                *pDevia2 += *pDeviaO;
                pDevia2++, pDeviaO++;
            }

            if(pDevia1 == pDevia1End) {
                pDevia1 = (Q*)inVars[0].devia;
            }
            if(pDevia2 == pDevia2End) {
                pDevia2 = (Q*)inVars[1].devia;
            }
        }
        VERIFY(pDeviaO-(Q*)outVar.devia==outVar.size)
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
        if(nInVars != 2) {
            return sCtx.error("连接需要两个参数");
        }

        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        int nDim1 = spDim1.size();
        int nDim2 = spDim2.size();
        if(nDim1 - nDim2 == 1) {
            nDim2 ++;
            spDim2 = spDim2.upLowDimension(1);
        }else if(nDim2 - nDim1 == 1) {
            nDim1 ++;
            spDim1 = spDim1.upLowDimension(1);
        }
        if(nDim1 != nDim2 || nDim1 < 1) {
            return sCtx.error("连接两个张量，需要两个张量维度数量相同");
        }

        int nSize1 = spDim1.data()[nDim1-1];
        int nSize2 = spDim2.data()[nDim2-1];
        if(nDim1 > 1) {
            spDim1 = spDim1.downLowDimension();
            spDim2 = spDim2.downLowDimension();
            if( !spDim1.isEqual(spDim2) ) {
                return sCtx.error("两个张量高纬度大小不一致，不能连接");
            }
        }

        m_nSize1 = nSize1;
        m_nSize2 = nSize2;
        createVariable(spDim1.upLowDimension(nSize1+nSize2), spVarOut);
        return addAtomSolver(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "JoinSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        ar.arBlock("size1", m_nSize1);
        ar.arBlock("size2", m_nSize2);
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.JoinSolver"; }

private:
    int m_nSize1;
    int m_nSize2;
};

SIMPLEWORK_FACTORY_AUTO_REGISTER(CJoinOperator, CJoinOperator::__getClassKey())
static SNnSolverRegister s_Register("join", CNnSolver::createSolver<CJoinOperator>);

#endif//__SimpleWork_NN_Operators_CJoinOperator_h__