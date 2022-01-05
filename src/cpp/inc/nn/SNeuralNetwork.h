#ifndef __SimpleWork_NeuralNetwork_h__
#define __SimpleWork_NeuralNetwork_h__

#include "nn.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NeuralNetwork)
public:
    //
    // 激活函数
    //
    enum EACTIVATION {
        ACTIVATION_ReLU,
        ACTIVATION_LeakyReLU,
        ACTIVATION_Softmax
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INeuralNetwork", 211223)

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

    SIMPLEWORK_INTERFACECLASS_ENTER(NeuralNetworkFactory, "sw.nn.NeuralNetworkFactory")
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INeuralNetworkFactory", 211223)

            //
            //  创建池化神经网络
            //
            virtual int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建直连神经网络
            //
            virtual int createDense(int nCells, EACTIVATION eActivation, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建卷积神经网络
            //
            virtual int createConvolution(int nWidth, int nHeight, int nConv, SNeuralNetwork& spNetwork) = 0;

            //
            //  创建顺序处理神经网络
            //
            virtual int createSequence(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) = 0;

            //
            //  读取整个IDX格式的文件，格式参考：http://yann.lecun.com/exdb/mnist/
            //
            virtual int readIdxFile(const char* szFileName, STensor& spTensor) = 0;

            //
            //  打开IDX格式的文件读取器，格式参考：http://yann.lecun.com/exdb/mnist/
            //
            virtual int openIdxFileReader(const char* szFileName, SNeuralPipe& spPipe) = 0;

            //
            //  创建归一化管道
            //
            virtual int createNormallizePipe(SNeuralPipe& spPipe) = 0;

            //
            //  归一化张量，将字节数据归一到[0,1]范围
            //
            virtual int normalizeTensor(const STensor& spIn, STensor& spOut) = 0;

            //
            //  分类化张量，将张量中的每一个分类下标，都转化为一个分类向量，新向量对应下标位置为1.0，其它位置为0.0
            //
            virtual int classifyTensor(int nClassify, const STensor& spIn, STensor& spOut) = 0;

        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetworkFactory)

public:
    static SNeuralNetworkFactory& getFactory() {
        static SNeuralNetworkFactory g_factory = SObject::createObject<SNeuralNetworkFactory>();
        return g_factory;
    }

    static SNeuralNetwork createDense(int nCells, EACTIVATION eActivation=ACTIVATION_ReLU) {
        SNeuralNetwork nn;
        getFactory()->createDense(nCells,eActivation,nn);
        return nn;
    }

    static SNeuralNetwork createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight) {
        SNeuralNetwork nn;
        getFactory()->createPool(nWidth, nHeight, nStrideWidth, nStrideHeight, nn);
        return nn;
    }

    static SNeuralNetwork createConv(int nWidth, int nHeight, int nConv) {
        SNeuralNetwork nn;
        getFactory()->createConvolution(nWidth, nHeight, nConv, nn);
        return nn;
    }

    static SNeuralNetwork createSequence(int nNetworks, SNeuralNetwork* pNetworks) {
        SNeuralNetwork nn;
        getFactory()->createSequence(nNetworks, pNetworks, nn);
        return nn;
    }

    static SNeuralPipe openIdxFileReader(const char* szFilename) {
        SNeuralPipe pipe;
        getFactory()->openIdxFileReader(szFilename, pipe);
        return pipe;
    }

    static STensor loadIdxFile(const char* szFilename) {
        STensor data;
        getFactory()->readIdxFile(szFilename, data);
        return data;
    }

    static SNeuralPipe createNormalizePipe() {
        SNeuralPipe pipe;
        getFactory()->createNormallizePipe(pipe);
        return pipe;
    }

    static STensor normalizeTensor(const STensor& spIn) {
        STensor spOut;
        getFactory()->normalizeTensor(spIn, spOut);
        return spOut;
    }

    static STensor classifyTensor(int nClassify, const STensor& spIn) {
        STensor spOut;
        getFactory()->classifyTensor(nClassify, spIn, spOut);
        return spOut;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralNetwork)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NeuralNetwork_h__