# simplework

提供一些简单的基础工作（基于C++的)，包括：

    1，动态创建一个指定名字的对象，返回通用智能IObjectPtr，而无需链接于对象的动态库；
    2，定义一个可以被其它模块创建的对象；

使用方法：

(一) 创建对象

#include "SimpleWork.h"

int main() {

    IObjectPtr spTensor = getSimpleWorkModule()->createObject("sw.math.tensor");
    if( spTensor ) {
        cout << "Great ! SimpleWork is fine!";
    }else {
        cout << "??？ What happen?";
    }
    return 0;
}

// 注册自己为支持SimpleWork的模块，如果不定义，则无法链接getSimpleWorkModule
SIMPLEWORK_MODULE_REGISTER("TestSimpleWork")

(二)
