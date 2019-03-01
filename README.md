# git自动更新hook
libevent练手项目，可以参考源码学习

## 依赖
+ gcc
+ cmake
+ libevent

## 编译安装

```
cmake .
make
mv git-hookd /usr/local/bin/
```

## 使用

```
git-hookd -h
```

## 钩子HTTP地址

```
http://ip:port?name=项目名
```