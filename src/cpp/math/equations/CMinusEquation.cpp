#include "equation.h"
#include "map"

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
static SCtx sCtx("CMinusEquation");
class CMinusEquation : public CObject, ITensorEquation {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorEquation)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://Factory
    static const char* __getClassKey() { return "sw.math.MinusEquation"; }

public://ITensorEquation
    int eval(const PData* pData, int nInVars, const STensor pInVars[], STensor& outVar) {
        if(nInVars != 2) {
            return sCtx.error("加法需要两个元素");
        }

        STensor sp1 = pInVars[0];
        STensor sp2 = pInVars[1];
        if(sp1.type() != sp2.type()) {
            return sCtx.error("两个类型不同的张量不能相加");
        }

        int nSizeIn = sp1.size();
        if(nSizeIn != sp2.size()) {
            return sCtx.error("两个大小不同的张量不能相加");
        }

        STensor spOut;
        if( STensor::createTensor(spOut, sp1.dimension(), sp1.type(), nSizeIn) != sCtx.success() ) {
            return sCtx.error("创建张量失败");
        }

        PVector inVars[] = {
            {nSizeIn, sp1.data() },
            {nSizeIn, sp2.data() }
        };
        if( eval(sp1.type(), nullptr, 2, inVars, {nSizeIn, spOut.data()} ) != sCtx.success() ) {
            return sCtx.error("数据类型不支持");
        }
        outVar = spOut;
        return sCtx.success();
    }

    static int eval(unsigned int idType, void* pParameter, int nInVar, const PVector inVars[], PVector outVar) {
        static map<unsigned int, FEval> sMap = {
            BASIC_EVAL_ENTRIES
        };
        map<unsigned int, FEval>::iterator it = sMap.find(idType);
        if(it != sMap.end()) {
            (*it->second)(pParameter, nInVar, inVars, outVar);
            return sCtx.success();
        }
        return sCtx.error();
    }

    template<typename Q>
    static void eval(void* pParameter, int nInVar, const PVector inVars[], PVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pIn1End = pIn1+inVars[0].size;
        Q* pIn2End = pIn2+inVars[1].size;
        while(outVar.size-->0) {
            *pO = *pIn1 - *pIn2;
            pO++, pIn1++, pIn2++;
            //if(pIn1 == pIn1End) {
            //    pIn1 = (Q*)inVars[0].data;
            //}
            //if(pIn2 == pIn2End) {
            //    pIn2 = (Q*)inVars[1].data;
            //}
        }
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMinusEquation, CMinusEquation::__getClassKey())
