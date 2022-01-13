#ifndef __SimpleWork_NnFactory_h__
#define __SimpleWork_NnFactory_h__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnNetwork;
class SNnPipe;

SIMPLEWORK_INTERFACECLASS_ENTER(NnFactory, "sw.nn.NnFactory")
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnFactory", 211223)

        //
        //  创建池化神经网络
        //
        virtual int createPool(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNnNetwork& spNetwork) = 0;

        //
        //  创建直连神经网络
        //
        virtual int createDense(int nCells, const char* szActivator, SNnNetwork& spNetwork) = 0;

        //
        //  创建直连神经网络
        //
        virtual int createTwoPoleDense(int nCells, const char* szActivator, SNnNetwork& spNetwork) = 0;

        //
        //  创建卷积神经网络
        //
        virtual int createConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNnNetwork& spNetwork) = 0;

        //
        //  创建旋转卷积神经网络，（卷积核每移动一步，会旋转一个角度）
        //
        virtual int createRotConvolution(int nWidth, int nHeight, int nConv, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNnNetwork& spNetwork) = 0;

        //
        //  创建卷积神经网络
        //
        virtual int createTwoPoleConvolution(int nWidth, int nHeight, int nConv, const char* szActivator, SNnNetwork& spNetwork) = 0;

        //
        //  创建顺序处理神经网络
        //
        virtual int createSequence(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) = 0;

        //
        //  创建并行处理神经网络
        //
        virtual int createParallel(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) = 0;

        //
        //  读取整个IDX格式的文件，格式参考：http://yann.lecun.com/exdb/mnist/
        //
        virtual int readIdxFile(const char* szFileName, STensor& spTensor) = 0;

        //
        //  打开IDX格式的文件读取器，格式参考：http://yann.lecun.com/exdb/mnist/
        //
        virtual int openIdxFileReader(const char* szFileName, SNnPipe& spPipe) = 0;

        //
        //  创建归一化管道
        //
        virtual int createNormallizePipe(SNnPipe& spPipe) = 0;

        //
        //  归一化张量，将字节数据归一到[0,1]范围
        //
        virtual int normalizeTensor(const STensor& spIn, STensor& spOut) = 0;

        //
        //  分类化张量，将张量中的每一个分类下标，都转化为一个分类向量，新向量对应下标位置为1.0，其它位置为0.0
        //
        virtual int classifyTensor(int nClassify, const STensor& spIn, STensor& spOut) = 0;

        //
        //  网络序列化
        //
        virtual int saveNetwork(const char* szFileName, const SNnNetwork& spNet) = 0;
        virtual int loadNetwork(const char* szFileName, SNnNetwork& spNet) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SNnFactory& getFactory() {
        static SNnFactory g_factory = SObject::createObject<SNnFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnFactory)


SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnFactory_h__