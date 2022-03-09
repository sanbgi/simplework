#include "equation.h"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CRootMeanSquareEquation");
class CRootMeanSquareEquation : public CObject, ITensorEquation {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorEquation)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.RootMeanSquareEquation"; }

public://ITensorEquation
    int eval(const PData* pData, int nInVars, const STensor pInVars[], STensor& outVar) {
        if(nInVars != 1) {
            return sCtx.error("加法需要两个元素");
        }

        STensor sp1 = pInVars[0];
        int nSizeIn = sp1.size();

        STensor spOut;
        if( STensor::createVector(spOut, sp1.type(), 1) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }
        PVector inVars[] = {
            {0, nSizeIn, sp1.data() },
        };
        if( eval(sp1.type(), nullptr, 1, inVars, {0, 1, spOut.data()} ) != sCtx.success() ) {
            return sCtx.error("数据类型不支持");
        }
        outVar = spOut;
        return sCtx.success();
    }

    static int eval(PDATATYPE idType, void* pParameter, int nInVar, const PVector inVars[], PVector outVar) {
        static map<PDATATYPE, FEval> sMap = {
            BASIC_EVAL_ENTRIES
        };
        map<PDATATYPE, FEval>::iterator it = sMap.find(idType);
        if(it != sMap.end()) {
            (*it->second)(pParameter, nInVar, inVars, outVar);
            return sCtx.success();
        }
        return sCtx.error();
    }

    template<typename Q>
    static void eval(void* pParameter, int nInVar, const PVector inVars[], PVector outVar) {
        Q* pIn = (Q*)inVars[0].data;
        int nSize = inVars[0].size;
        Q* pInEnd = pIn+nSize;
        Q sum = 0;
        while(pIn<pInEnd) {
            sum += *pIn * *pIn;
            pIn++;
        }

        Q* pO = (Q*)outVar.data;
        *pO = (Q)sqrt(sum / nSize);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CRootMeanSquareEquation, CRootMeanSquareEquation::__getClassKey())
