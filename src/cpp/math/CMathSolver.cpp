
#include "CMathSolver.h"
#include "map"

using namespace std;

static SCtx sCtx("CMathSolver");

template<typename Q>
class CMathSolverT : public CObject, public IMathSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IMathSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    void set(PVector vec, int value){
        Q* pTarget = (Q*)vec.data;
        while(vec.size-->0) {
            *pTarget = (Q)value;
            pTarget++;
        }
    }

    void copy(int nSize, void* pDesc, void* pSrc){
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize-->0) {
            *pTarget = *pSource;
            pTarget++, pSource++;
        }
    }

    void plus(int nSize, void* pDesc, void* pSrc) {
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize-->0) {
            *pTarget += *pSource;
            pTarget++, pSource++;
        }
   }

    void minus(int nSize, void* pDesc, void* pSrc){
        Q* pTarget = (Q*)pDesc;
        Q* pSource = (Q*)pSrc;
        while(nSize-->0) {
            *pTarget -= *pSource;
            pTarget++, pSource++;
        }
    }

    void plus(int nSize, void* pIn1, void* pIn2, void* pOut) {
        Q* pV1 = (Q*)pIn1;
        Q* pV2 = (Q*)pIn2;
        Q* pVO = (Q*)pOut;
        Q* pVOEnd = pVO + nSize;
        while(pVO != pVOEnd) {
            *pVO = *pV1 + *pV2;
            pVO++, pV1++, pV2++;
        }
    }

    void minus(int nSize, void* pIn1, void* pIn2, void* pOut) {
        Q* pV1 = (Q*)pIn1;
        Q* pV2 = (Q*)pIn2;
        Q* pVO = (Q*)pOut;
        Q* pVOEnd = pVO + nSize;
        while(pVO != pVOEnd) {
            *pVO = *pV1 - *pV2;
            pVO++, pV1++, pV2++;
        }
    }

    void addByWeight(int nSize, void* pIn1, void* pIn2, void* pWeight, void* pOut) {
        Q* pV1 = (Q*)pIn1;
        Q* pV2 = (Q*)pIn2;
        Q* pVO = (Q*)pOut;
        Q* pW = (Q*)pWeight;
        Q* pVOEnd = pVO + nSize;
        while(pVO != pVOEnd) {
            *pVO = *pV1 * (*pW) + *pV2 * (1-(*pW));
            pVO++, pV1++, pV2++, pW++;
        }
    }

    void multiply(int nSize, void* pIn1, void* pIn2, void* pOut) {
        Q* pV1 = (Q*)pIn1;
        Q* pV2 = (Q*)pIn2;
        Q* pVO = (Q*)pOut;
        Q* pVOEnd = pVO + nSize;
        while(pVO != pVOEnd) {
            *pVO = *pV1 * *pV2;
            pVO++, pV1++, pV2++;
        }
    }

    int product(PVector vecIn, PVector vecMatrix, PVector vecOut) {
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

    int join(PVector vecIn1, PVector vecIn2, PVector vecOut) {
        if( vecIn1.size + vecIn2.size != vecOut.size ) {
            return sCtx.error("连接操作的向量尺寸不匹配");
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

    static int getSolver(SMathSolver& spSolver) {
        static SMathSolver g_solver = CObject::createObject<CMathSolverT>();
        spSolver = g_solver;
        return sCtx.success();
    }
};


int CMathSolver::createSolver(PDATATYPE idType, SMathSolver& spSolver) {
    if(idType == CBasicData<double>::getStaticType() ) {
        return CMathSolverT<double>::getSolver(spSolver);
    }else
    if(idType == CBasicData<float>::getStaticType() ) {
        return CMathSolverT<float>::getSolver(spSolver);
    }
    return sCtx.error("不支持数据类型");
}