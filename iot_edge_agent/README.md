## IoT Electric Power Edge Agent 核心业务逻辑代码

* ./src/serial_io.c         对串口io读写的封装
* ./src/modbus.c            对modbus协议的封装
* ./src/dlt_698.c           对DLT698.45协议的封装
* ./src/iotea_client.c      对mqtt io逻辑的封装
* ./src/protocol_manager.c  对协议管理的封装
* ./src/iot_main.c          主要业务逻辑的实现
* ./CMakeLists.txt          cmake编译说明文件

###编译命令

```
    cd build
    cmake ..
    cmake --build .
```

###测试命令
```
    ./dlt_socket_test.py && mosquitto_pub -h 127.0.0.1 -p 1883 -u test -P hahaha -i Client_test -t /devMgr/get/response/Modbus/dev -m "$(cat tunnel_model.json)";mosquitto_pub -h 127.0.0.1 -p 1883 -u test -P hahaha -i Client_test -t /ModelManager/get/reponse/MODBUS/Southmodel -m "$(cat thing_model.json)"
```