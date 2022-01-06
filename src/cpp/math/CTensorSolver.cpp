#include "../inc/math/math.h"
#include <map>
#include "CTensorFactory.h"

using namespace sw::core;
using namespace sw::math;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static unsigned int s_double_type_id = CBasicData<double>::getStaticType();

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CTensorSolver : public CObject, ITensorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int minus( const STensor& t1, const STensor& t2, STensor& spOut) {
        int nSize = t1->getDataSize();
        if( nSize != t2->getDataSize()) {
            return sCtx.error("大小不同的两个张量不能相减");
        }

        if(t1->getDataType() != s_double_type_id || t2->getDataType() != s_double_type_id ) {
            return sCtx.error("目前张量运算只支持双精度类型");
        }

        if( STensor::createTensor<double>(spOut, t1->getDimVector(), nSize) != sCtx.success() ) {
            return sCtx.error("创建结果张量失败");
        }

        double* pT1 = t1->getDataPtr<double>();
        double* pT2 = t2->getDataPtr<double>();
        double* pOut = spOut->getDataPtr<double>();
        for(int i=0; i<nSize; i++) {
            pOut[i] = pT1[i] - pT2[i];
        }
        return sCtx.success();
    }

    int multiply( const PTensor& t1, const PTensor& t2, IVisitor<const PTensor&>* pRecerver) {
        if( t1.idType != s_double_type_id || t2.idType != s_double_type_id ) {
            return sCtx.error();
        }

        if(t1.nDims < 1 || t2.nDims < 1 ) {
            return sCtx.error();
        }

        if(t1.pDimSizes[t1.nDims-1] != t2.pDimSizes[0] ) {
            return sCtx.error();
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

    int createVector(STensor& spTensor, unsigned int eElementType, int nElementSize, void* pElementData = nullptr) {
        return CTensorFactory::createVector(spTensor, eElementType, nElementSize, pElementData);
    }
    
    int createTensor(STensor& spTensor, const STensor& spDimVector, unsigned int eElementType, int nElementSize, void* pElementData = nullptr ){
        return CTensorFactory::createTensor(spTensor, spDimVector, eElementType, nElementSize, pElementData);
    } 

    static SCtx sCtx;
};
SCtx CTensorSolver::sCtx("CTensorSolver");

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE