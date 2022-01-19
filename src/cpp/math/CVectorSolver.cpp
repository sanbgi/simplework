
#include "CVectorSolver.h"
#include "map"

using namespace std;

static SCtx sCtx("CVectorSolver");

template<typename Q>
class CVectorSolverT : public CObject, public IVectorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IVectorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int add(int nSize, void* pDesc, void* pSrc) {
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize>=0) {
            *pTarget += *pSource;
        }
        return sCtx.success();
    }

    int del(int nSize, void* pDesc, void* pSrc){
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize>=0) {
            *pTarget -= *pSource;
        }
        return sCtx.success();
    }

    int copy(int nSize, void* pDesc, void* pSrc){
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize>=0) {
            *pTarget = *pSource;
        }
        return sCtx.success();
    }

    int zero(int nSize, void* pDesc){
        Q* pTarget = (Q*)pDesc;
        while(nSize>=0) {
            *pTarget = 0;
        }
        return sCtx.success();
    }

    int add(int nSize, void* pIn1, void* pIn2, void* pOut) {
        Q* pV1 = (Q*)pIn1;
        Q* pV2 = (Q*)pIn2;
        Q* pVO = (Q*)pOut;
        Q* pVOEnd = pVO + nSize;
        while(pVO != pVOEnd) {
            *pVO = *pV1 + *pV2;
            pVO++, pV1++, pV2++;
        }
        return sCtx.success();
    }

    int multiply(PVector vecIn, PVector vecMatrix, PVector vecOut) {
        if( vecIn.size * vecOut.size != vecMatrix.size ) {
            return sCtx.error("乘法输入输出尺寸不一致");
        }
        Q* pV1 = (Q*)vecIn.data;
        Q* pV2 = (Q*)vecMatrix.data;
        Q* pVO = (Q*)vecOut.data;
        Q* pVOEnd = pVO + vecOut.size;
        Q* pV1End = pV1 + vecIn.size;
        Q* pItIn;
        Q v;
        while(pVO != pVOEnd) {
            v = 0;
            pItIn = pV1;
            while(pItIn < pV1End) {
                v += (*pItIn) * (*pV2);
                pItIn++, pV2++;
            }
            *pVO = v;
            pVO++;
        }
        return sCtx.success();
    }

    int multiplyAccDeviation(PDeviaVector vOut, PDeviaVector vWeights, PDeviaVector vIn) {
        if( vIn.size * vOut.size != vWeights.size ) {
            return sCtx.error("输入输出向量尺寸不相等，求偏导失败");
        }
        Q* pInput1 = (Q*)vIn.data;
        Q* pD1 = (Q*)vIn.devia;
        Q* pWeights = (Q*)vWeights.data;
        Q* pWeightDeviations = (Q*)vWeights.devia;
        Q* pDeviaOut = (Q*)vOut.data;
        Q* pDeviaOutEnd = pDeviaOut + vOut.size;
        Q* pInput1End = pInput1 + vIn.size;
        Q* pIn, *pInDeviation;
        Q deviationOut;
        while(pDeviaOut != pDeviaOutEnd) {
            pIn = pInput1;
            pInDeviation = pD1;
            deviationOut = *pDeviaOut;
            while(pIn < pInput1End) {
                *pInDeviation += deviationOut * (*pWeights);
                *pWeightDeviations += deviationOut * (*pIn);
                pIn++, pInDeviation++, pWeights++, pWeightDeviations++;
            }
            pDeviaOut++;
        }
        return sCtx.success();
    }


    int join(PVector vecIn1, PVector vecIn2, PVector vecOut) {
        if(vecIn1.size + vecIn2.size != vecOut.size) {
            return sCtx.error("输入输出尺寸不一致");
        }
        Q* pV1 = (Q*)vecIn1.data;
        Q* pV2 = (Q*)vecIn2.data;
        Q* pVO = (Q*)vecOut.data;
        Q* pV1End = pV1 + vecIn1.size;
        Q* pV2End = pV2 + vecIn2.size;
        while(pV1 != pV1End) {
            *pVO = *pV1;
            pVO++, pV1++;
        }
        while(pV2 != pV2End) {
            *pVO = *pV2;
            pVO++, pV2++;
        }
        return sCtx.success();
    }

    static int getSolver(SVectorSolver& spSolver) {
        static SVectorSolver g_solver = CObject::createObject<CVectorSolverT>();
        spSolver = g_solver;
        return sCtx.success();
    }
};


int CVectorSolver::createSolver(unsigned int idType, SVectorSolver& spSolver) {
    if(idType == CBasicData<double>::getStaticType() ) {
        return CVectorSolverT<double>::getSolver(spSolver);
    }else
    if(idType == CBasicData<float>::getStaticType() ) {
        return CVectorSolverT<float>::getSolver(spSolver);
    }
    return sCtx.error("不支持数据类型");
}