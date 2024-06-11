环境
linux环境，内核版本要求3.5+（sendmsg/recvmsg要求）

编译和启动：
Makefile目录下执行 make
cd bin
./sw-server

默认监听127.0.0.1:8883
热更新信号sigusr2，kill -USR2 $pid可以发送信号
tellnet 127.0.0.1 8883 可以建立长连接并发送数据

注意：本代码在linux下测试通过，本次mac重写时epoll相关库缺失因此未重试，可能有部分错漏
