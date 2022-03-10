#ifndef __SimpleWork_NnNetwork_h__
#define __SimpleWork_NnNetwork_h__

#include "nn.h"
#include "SNnModule.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER



//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnNetwork)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnNetwork", 211223)

        //
        //  计算值
        //      @spBatchIn 输入张量，多维张量，其中第一个维度为实际张量的个数
        //      @spBatchOut 输出张量
        //
        virtual int eval(const STensor& spBatchIn, STensor& spBatchOut) = 0;

        //
        //  计算偏差（偏导）
        //      @spBatchOut 由eval计算输出的结果
        //      @spBatchOutDeviation 计算结果与实际期望的偏差 = 计算值 - 期望值
        //      @spBatchIn 返回上次计算的输入值
        //      @spBatchInDeviation 输入值与期望输入值的偏差 = 输入值 - 期望值
        //
        //  注意：
        //      spOutTensor必须是由eval最有一次计算出来的张量，否则，计算会失败
        //
        virtual int devia(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) = 0;

        //
        // 更新网络
        //      @spBatchInDevia 由devia计算出来的输入偏差值
        //
        virtual int update(const STensor& spBatchInDeviation) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SNnNetwork createNetwork(const PNnNetwork& rNet) {
        return SObject::createObject("sw.nn.LayerNetwork", CData<PNnNetwork>(rNet));
    }
    static SNnNetwork createOpenCLNetwork(const PNnNetwork& rNet) {
        return SObject::createObject("sw.nn.OpenCLNetwork", CData<PNnNetwork>(rNet));
    }
    static SNnNetwork createNetwork(const SDimension spInDimension, const SNnModule& spModule) {
        class CNnNetworkSolver : public INnNetworkSolver {
            int solve(const SNnVariable& spIn, SNnVariable& spOut) {
                return spModule->eval(1, &spIn, spOut);
            }
        public:
            SNnModule spModule;
        }sSolver;
        sSolver.spModule = spModule;
        return createNetwork({spInDimension, &sSolver});
    }

    struct PNnCompositeNetwork {
        SIMPLEWORK_PDATAKEY(PNnCompositeNetwork, "sw.nn.PNnCompositeNetwork")

        int nNetworks;
        SNnNetwork* pNetworks;
    };
    static SNnNetwork createCompositeNetwork(const PNnCompositeNetwork& rData) {
        return SObject::createObject("sw.nn.CompositeNetwork", CData<PNnCompositeNetwork>(rData));
    }

    static int saveFile(const char* szFileName, const SNnNetwork& spNet) {
        return SNnFactory::getFactory()->saveNetwork(szFileName, spNet);
    }

    static SNnNetwork loadFile(const char* szFileName) {
        SNnNetwork spNet;
        SNnFactory::getFactory()->loadNetwork(szFileName, spNet);
        return spNet;
    }

public:
    STensor eval(const STensor& spIn) {
        STensor spOut;
        if(*this){
            (*this)->eval(spIn, spOut);
        }
        return spOut;
    }

    STensor devia(const STensor& spOut, const STensor& spOutDeviation) {
        STensor spIn, spInDeviation;
        if(*this) {
            (*this)->devia(spOut, spOutDeviation, spIn, spInDeviation);
        }
        return spInDeviation;
    }

    STensor devia(const STensor& spOut, const STensor& spOutDeviation, STensor& spIn) {
        STensor spInDeviation;
        if(*this) {
            (*this)->devia(spOut, spOutDeviation, spIn, spInDeviation);
        }
        return spInDeviation;
    }

    int update(const STensor& spInDeviation) {
        if(*this){
            return (*this)->update(spInDeviation);
        }
        return SError::ERRORTYPE_FAILURE;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnNetwork)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnNetwork_h__