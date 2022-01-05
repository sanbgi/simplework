#include "../inc/math/math.h"
#include <map>

using namespace sw::core;
using namespace sw::math;

SIMPLEWORK_MATH_NAMESPACE_ENTER


//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CTensorSolver : public CObject, ITensorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int multiply( const PTensor& t1, const PTensor& t2, IVisitor<const PTensor&>* pRecerver) {
        static unsigned int s_double_type_id = SData::getTypeIdentifier<CBasicData<double>>();
        if( t1.idType != s_double_type_id || t2.idType != s_double_type_id ) {
            return sCtx.Error();
        }

        if(t1.nDims < 1 || t2.nDims < 1 ) {
            return sCtx.Error();
        }

        if(t1.pDimSizes[t1.nDims-1] != t2.pDimSizes[0] ) {
            return sCtx.Error();
        }

        int mlength = t1.pDimSizes[t1.nDims-1];
        int nT1vector = t1.nData / mlength;
        int nT2vector = t2.nData / mlength;

        // 计算结果数据
        int nResSize = nT1vector * nT2vector;
        double dResTensor[nResSize];
        for( int i=0; i<nT1vector; i++) {
            for(int j=0; j<nT2vector; j++) {
                double dRes = 0;
                for( int k=0; k<mlength; k++ ) {
                    dRes += t1.pDoubleArray[i*mlength+k] * t2.pDoubleArray[k*nT2vector+j];
                }
                dResTensor[i*nT2vector+j] = dRes;
            }
        }
        
        // 计算结果维度
        int nResDims = t1.nDims+t2.nDims-2;
        int nResDimSizes[nResDims];
        for(int i=0; i<t1.nDims-1; i++) {
            nResDimSizes[i] = t1.pDimSizes[i];
        }
        for(int j=1; j<t2.nDims; j++) {
            nResDimSizes[t1.nDims+j-2] = t2.pDimSizes[j];
        }

        PTensor resTensor;
        resTensor.nData = nResSize;
        resTensor.pDoubleArray = dResTensor;
        resTensor.nDims = nResDims;
        resTensor.pDimSizes = nResDimSizes;
        return pRecerver->visit(resTensor);
    }

    static SCtx sCtx;
};
SCtx CTensorSolver::sCtx("CTensorSolver");

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE