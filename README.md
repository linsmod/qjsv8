# QJSV8 (keywords: miniv8, mini-v8, v8-mini,v8-lite, qjs-backend-v8, quickjs-backend-v8)
本项目旨在使用quickjs实现v8.h的公共接口。

This project is intent to implement v8 api with qjs(quickjs)

初始代码提取自MiniV8, 从 miniblink：https://github.com/weolar/miniblink49.

Project initial codes is coming from MiniV8 of miniblink：https://github.com/weolar/miniblink49.

## 注意
这只是一个实验性项目，它只是为了展示一种可能性而实现了v8的一小部分Api。
你可以看到很多代码并不合乎最佳编码实践或者最佳架构实践,这是项目特色之一。

# 编译运行 on Ubuntu
```
cd src
make qjsv8-shell
./qjsv8-shell
```

# 调试 qjsv8-shell
vscode: F5

terminal:
```
make qjsv8-shelld
gdb qjsv8-shelld
```

# 已知问题
本来qjsv8-shell刚加入到项目里面的时候，是可以运行并显示错误消息的。
但是经过本人后续的胡乱修改，qjsv8-shelld 已经无法反馈错误消息^^



