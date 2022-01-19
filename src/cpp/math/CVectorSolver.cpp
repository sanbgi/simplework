
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

    int add(const PVectorArray& inVars, const PVector& outVar){
        if(inVars.size != 2) {
            return sCtx.error("向量相加的输入参数，必须为两个向量");
        }
        PVector v1 = inVars.data[0];
        PVector v2 = inVars.data[1];
        PVector vO = outVar;
        if( v1.size != v2.size || v2.size != vO.size ) {
            return sCtx.error("输入输出向量尺寸不相等，不能相加");
        }
        Q* pV1 = (Q*)v1.data;
        Q* pV2 = (Q*)v2.data;
        Q* pVO = (Q*)vO.data;
        Q* pVOEnd = pVO + vO.size;
        while(pVO != pVOEnd) {
            *pVO = *pV1 + *pV2;
            pVO++, pV1++, pV2++;
        }
        return sCtx.success();
    }
    int addAccDeviation(const PVector& outDVar, const PVectorArray& inDVars){
        if(inDVars.size != 2) {
            return sCtx.error("向量相加的输入参数，必须为两个向量");
        }
        PVector v1 = inDVars.data[0];
        PVector v2 = inDVars.data[1];
        PVector vO = outDVar;
        if( v1.size != v2.size || v2.size != vO.size ) {
            return sCtx.error("输入输出向量尺寸不相等，求偏导失败");
        }
        Q* pV1 = (Q*)v1.data;
        Q* pV2 = (Q*)v2.data;
        Q* pVO = (Q*)vO.data;
        Q* pVOEnd = pVO + vO.size;
        while(pVO != pVOEnd) {
            *pV1 += *pVO;
            *pV2 += *pVO;
            pVO++, pV1++, pV2++;
        }
        return sCtx.success();
    }

    int multiply(const PVectorArray& inVars, const PVector& outVar){
        if(inVars.size != 2) {
            return sCtx.error("矩阵乘法输入必须为两个输入，一个向量，一个矩阵");
        }
        PVector v1 = inVars.data[0];
        PVector v2 = inVars.data[1];
        PVector vO = outVar;
        if( v1.size * vO.size != v2.size ) {
            return sCtx.error("乘法输入输出尺寸不一致");
        }
        Q* pV1 = (Q*)v1.data;
        Q* pV2 = (Q*)v2.data;
        Q* pVO = (Q*)vO.data;
        Q* pVOEnd = pVO + vO.size;
        Q* pV1End = pV1 + v1.size;
        Q* pIn;
        Q v;
        while(pVO != pVOEnd) {
            v = 0;
            pIn = pV1;
            while(pIn < pV1End) {
                v += (*pIn) * (*pV2);
                pIn++, pV2++;
            }
            *pVO = v;
            pVO++;
        }
        return sCtx.success();
    }

    int multiplyAccDeviation(const PVector& outDVar, const PVectorArray& inDVars, const PVectorArray& inVars){
        if(inDVars.size != 2 || inVars.size != 2) {
            return sCtx.error("向量相加的输入参数，必须为两个向量");
        }
        PVector dinput1 = inDVars.data[0];
        PVector dinput2 = inDVars.data[1];
        PVector input1 = inVars.data[0];
        PVector input2 = inVars.data[1];
        PVector deviaOut = outDVar;
        if( dinput1.size != input1.size || dinput2.size != input2.size ||
            input1.size * deviaOut.size != input2.size ) {
            return sCtx.error("输入输出向量尺寸不相等，求偏导失败");
        }
        Q* pInput1 = (Q*)input1.data;
        Q* pWeights = (Q*)input2.data;
        Q* pD1 = (Q*)dinput1.data;
        Q* pWeightDeviations = (Q*)dinput2.data;
        Q* pDeviaOut = (Q*)deviaOut.data;
        Q* pDeviaOutEnd = pDeviaOut + deviaOut.size;
        Q* pInput1End = pInput1 + input1.size;
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

    int join(const PVectorArray& inVars, const PVector& outVar){
        if(inVars.size != 2) {
            return sCtx.error("向量连接，必须为两个向量");
        }
        PVector v1 = inVars.data[0];
        PVector v2 = inVars.data[1];
        PVector vO = outVar;
        if( v1.size + v2.size != vO.size ) {
            return sCtx.error("输入输出向量尺寸不相等，不能相加");
        }
        Q* pV1 = (Q*)v1.data;
        Q* pV2 = (Q*)v2.data;
        Q* pVO = (Q*)vO.data;
        Q* pV1End = pV1 + v1.size;
        Q* pV2End = pV2 + v2.size;
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
    int joinAccDeviation(const PVector& outDVar, const PVectorArray& inDVars){
        if(inDVars.size != 2) {
            return sCtx.error("向量连接，必须为两个向量");
        }
        PVector v1 = inDVars.data[0];
        PVector v2 = inDVars.data[1];
        PVector vO = outDVar;
        if( v1.size + v2.size != vO.size ) {
            return sCtx.error("输入输出向量尺寸不相等，不能相加");
        }
        Q* pV1 = (Q*)v1.data;
        Q* pV2 = (Q*)v2.data;
        Q* pVO = (Q*)vO.data;
        Q* pV1End = pV1 + v1.size;
        Q* pV2End = pV2 + v2.size;
        while(pV1 != pV1End) {
            *pV1 += *pVO;
            pVO++, pV1++;
        }
        while(pV2 != pV2End) {
            *pV2 += *pVO;
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