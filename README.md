提供一些简单的基础工作（基于C++的)，包括：

    1，动态创建一个指定名字的对象，返回通用智能指针Object，而无需链接于对象的动态库；
    2，定义一个可以被创建的对象；
    3，后续如果想做，会继续提供的能力包括：
        3.1，基础数学计算能力(sw.math)，特别是多维矩阵（张量）
        3.2，基础输入输出功能(sw.io)，特别是视频采集（文件或摄像头）、视频输出（显示屏）、音频采集（文件或麦克风）、音频输出（音响）
        3.3，基础神经网络功能，包括：目前现有神经网络的封装，以及自定义神经网络层（想实现的一些高级功能等）

为啥要做这个，原因如下：
    1，（还是暂时不说了，此处略）

警告，目前项目还处于非常非常早期阶段，github只是为了方便存储代码，所以，建议暂时不要使用，除非想和我一起共建（吴冰，85222460@qq.co）：

(一) 编译链接SimpleWork库

    目前还没有整理，毕竟还在开发阶段，后续应该是类似在命令行的如下操作
    git clone "https://github.com/sanbgi/simplework"
    cd simplework && mkdir build
    cd build && cmake ..
    make .
    make install

(二) 创建对象

    #include "SimpleWork.h" // 目前在src/cpp/inc目录中
    int main() {
        Tensor tensor = getSimpleWork()->createObject("sw.math.tensor");
        if( tensor ) {
            cout << "Great ! SimpleWork is fine!";
        }else {
            cout << "??？ What happen?";
        }
        return 0;
    }
    // 注册自己为支持SimpleWork的模块，如果不定义，则无法链接全局唯一的getSimpleWork函数
    SIMPLEWORK_MODULE_REGISTER("TestSimpleWork")

(三) 定义对象类

    #include "SimpleWork.h" // 目前在src/cpp/inc目录中

    class CMyObject : public IObject {

    }
    SIMPLEWORK_CLASS_REGISTER("TestSimpleWork.MyObject")

    int main() {
        Object myObject = Object::createObject("TestSimpleWork.MyObject");
        if( myObject ) {
            std::cout << "Great ! I have created my first object!";
        }else {
            std::cout << "??？ What happen?";
        }
        return 0;
    }
    SIMPLEWORK_MODULE_REGISTER("TestSimpleWork")

(四) 实现对象接口

    #include "SimpleWork.h" // 目前在src/cpp/inc目录中
    SIMPLEWORK_INTERFACECLASS_ENTER(MyObject)
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IMyObject", 211202)
            virtual void sayHi() = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(MyObject)

    class CMyObject : public CObject, public IMyObject {
        SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
            SIMPLEWORK_INTERFACE_ENTRY(IMyObject)
        SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

    public:
        void sayHi() {
            std::cout << "Great ! Hi everyone!";
        }
    };
    SIMPLEWORK_FACTORY_REGISTER(CMyObject, "TestSimpleWork.MyObject")

    int main() {
        MyObject spMyObject = Object::createObject("TestSimpleWork.MyObject");
        if( spMyObject ) {
            spMyObject->sayHi();
        }else {
            std::cout << "??？ What happen?";
        }
        return 0;
    }
    SIMPLEWORK_MODULE_REGISTER("TestSimpleWork")
