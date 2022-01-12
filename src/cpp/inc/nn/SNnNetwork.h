#ifndef __SimpleWork_NnNetwork_h__
#define __SimpleWork_NnNetwork_h__

#include "nn.h"
#include "SNnFactory.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnNetwork)


    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnNetwork", 211223)

        //
        //  计算
        //      @spInTensor 输入张量，多维张量，其中第一个维度为实际张量的个数
        //      @spOutTensor 输出张量
        //
        virtual int eval(const STensor& spInTensor, STensor& spOutTensor) = 0;

        //
        //  学习
        //      @spOutTensor 由eval计算输出的结果
        //      @spOutDeviation 计算结果与实际期望的偏差 = 计算值 - 期望值
        //      @spInTensor 返回上次计算的输入值
        //      @spInDeviation 输入值与期望输入值的偏差 = 输入值 - 期望值
        //
        //  注意：
        //      spOutTensor必须是由eval最有一次计算出来的张量，否则，学习会失败；
        //
        virtual int learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SNnNetwork createDense(int nCells, const char* szActivator = nullptr) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createDense(nCells,szActivator,nn);
        return nn;
    }

    static SNnNetwork createTwoPoleDense(int nCells, const char* szActivator = nullptr) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createTwoPoleDense(nCells,szActivator,nn);
        return nn;
    }

    static SNnNetwork createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createPool(nWidth, nHeight, nStrideWidth, nStrideHeight, nn);
        return nn;
    }

    static SNnNetwork createConv(int nWidth, int nHeight, int nConv, const char* szActivator = nullptr ) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createConvolution(nWidth, nHeight, nConv, szActivator, nn);
        return nn;
    }

    static SNnNetwork createRotConv(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator = nullptr) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createRotConvolution(nWidth, nHeight, nConv, dWidthRotAngle, dHeightRotAngle, szActivator, nn);
        return nn;
    }

    static SNnNetwork createTwoPoleConv(int nWidth, int nHeight, int nConv, const char* szActivator = nullptr ) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createTwoPoleConvolution(nWidth, nHeight, nConv, szActivator, nn);
        return nn;
    }

    static SNnNetwork createSequence(int nNetworks, SNnNetwork* pNetworks) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createSequence(nNetworks, pNetworks, nn);
        return nn;
    }

    static SNnNetwork createParallel(int nNetworks, SNnNetwork* pNetworks) {
        SNnNetwork nn;
        SNnFactory::getFactory()->createParallel(nNetworks, pNetworks, nn);
        return nn;
    }

    static SNeuralPipe openIdxFileReader(const char* szFilename) {
        SNeuralPipe pipe;
        SNnFactory::getFactory()->openIdxFileReader(szFilename, pipe);
        return pipe;
    }

    static STensor loadIdxFile(const char* szFilename) {
        STensor data;
        SNnFactory::getFactory()->readIdxFile(szFilename, data);
        return data;
    }

    static SNeuralPipe createNormalizePipe() {
        SNeuralPipe pipe;
        SNnFactory::getFactory()->createNormallizePipe(pipe);
        return pipe;
    }

    static STensor normalizeTensor(const STensor& spIn) {
        STensor spOut;
        SNnFactory::getFactory()->normalizeTensor(spIn, spOut);
        return spOut;
    }

    static STensor classifyTensor(int nClassify, const STensor& spIn) {
        STensor spOut;
        SNnFactory::getFactory()->classifyTensor(nClassify, spIn, spOut);
        return spOut;
    }

public:
    STensor eval(const STensor& spIn) {
        STensor spOut;
        (*this)->eval(spIn, spOut);
        return spOut;
    }

    int learn(const STensor& spOut, const STensor& spOutDeviation) {
        STensor spIn, spInDeviation;
        return (*this)->learn(spOut, spOutDeviation, spIn, spInDeviation);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnNetwork)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnNetwork_h__