fantuan

参考chen shuo的muduo所写的epoll LT简化版,代码量少很多,主要包括以下几块:

1,network.h/cpp
包裹了比较重要的socket函数

2,handler.h
包含了connection的几种回调定义,和context的几种回调函数定义

3,buffer.h
定义了一个简易的定长buffer,可以append也可以提取

4,context.h/cpp
包含了一个connection或acceptor的IO事件

5,acceptor.h/cpp
定义了整个服务端,监听所有的connection,以及所有epoll的操作及对context的更改操作

6,connection.h/cpp
定义了connection,包含了connection的read/write/close/error handling

目前版本是单线程的, 可能还存在一些小bug, 不过大致上是可以成功运行了, 做了一些简单的测试.
局域网内6000个连接,每个连接每秒发2KB数据收2KB数据,服务器运行稳定,CPU使用率15%左右(软中断CPU消耗不算但很高),单核2.6GHZ, L1 256KB.
