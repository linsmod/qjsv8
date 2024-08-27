# 使用quickjs实现v8.h的公共接口。

初始代码提取自 miniblink：https://github.com/weolar/miniblink49.


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
经过本人的胡乱修改，qjsv8-shelld 已经无法反馈错误消息^^



