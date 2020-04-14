## IoT Electric Power Edge Agent SDK for C ##

此代码库包含以下组件：

- c-uility(Microsoft Azure C Shared Utility) 有好多比基础库好用安全的函数
- umqtt(Microsoft mqtt client)
- iothub_client(物接入Edge SDK for C) 帮助设备快速接入mqtt平台
- liblightmodbus(A lightweight, cross-platform Modbus library)
- libdlt698(A lightweight 698.45 library)
- parson(A lighweight json library)

## 物接入Edge SDK for C ##
- 代码使用ANSI C（C99）规范，从而使代码更方便移植到不同的平台中
- 请避免使用编译器扩展选项，防止在不同平台上编译的不同行为表现
- 在物接入Edge SDK中，使用了一个平台抽象层，以隔离操作系统相关性（线程和互斥机制，通信协议，例如HTTP等）。

## SDK目录结构 ##

- /c-utility

	引用的git子模块，使用的第三方工具库。

- /umqtt

	引用的git子模块，使用的第三方MQTT客户端。

- /parson

	引用的git子模块，使用的第三方的JSON库。

- /certs

	包含与物接入进行通信所需的证书。

- /build

	包含客户端库和相关组件的针对指定平台的编译脚本。

- /iothub_client

	包含物接入IoT Hub客户端组件，对umqtt的封装。

- /liblightmodbus

	包含modbus rtu组包解析相关库函数

- /libdlt698

	包含使用DLT698.45 组包解析相关库函数

- /iot_edge_agent

	包含电力相关数据采集业务逻辑


### 设置Linux开发环境 ###

这一节会介绍如何设置C SDK在ubuntu下面的开发环境。可以使用CMake来创建makefiles，执行命令make调用gcc来将他们编译成为C语言版本的SDK

- 安装IDE开发工具，你可以下载Clion工具，链接地址：https://www.jetbrains.com/clion/, 可以直接导入现有项目，不要覆盖当前的CMake项目

- 在编译SDK之前确认所有的依赖库都已经安装好，例如ubuntu平台，你可以执行apt-get这个命令区安装对应的安装包

			sudo apt-get update sudo apt-get install -y git cmake build-essential curl libcurl4-openssl-dev libssl-dev uuid-dev 

- 验证CMake是不是最低允许的版本2.8.12

			cmake --version
 
	关于如何在ubuntu 14.04上将Cmake升级到3.x，可以阅读 [How to install CMake 3.2 on Ubuntu 14.04](http://askubuntu.com/questions/610291/how-to-install-cmake-3-2-on-ubuntu-14-04 "How to install CMake 3.2 on Ubuntu 14.04")
- 验证gcc是不是最低允许的版本呢4.4.7

			gcc  --version

- 关于如何在ubuntu 14.04上将gcc升级的信息可以阅读 [How do I use the latest GCC 4.9 on Ubuntu 14.04](http://askubuntu.com/questions/466651/how-do-i-use-the-latest-gcc-4-9-on-ubuntu-14-04 ).


- 编译C版本的SDK

	执行下面命令编译SDK：

		cd build
		cmake .. 
		cmake --build .  # append '-- -j <n>' to run <n> jobs in parallel 
