# 代码示例 {#ZpcVerse_Example}

## 该文档目标人群
建议在阅读该文档前，先浏览完[设计理念](@ref ZpcVerse_Design)文档。

##  开发主题

### 在C++中进行Python开发

#### 与Python C-APIs混用

本框架已封装了大多数常用的Python CAPIs到类（如PyVar、ZsObject等）函数成员和全局C链接的自由函数（zs_tuple_obj_pack_zsobjs、zs_string_obj等）中。若有对其他Python C API的使用刚需，依旧可以借助PyVar来处理。

```cpp

```

#### 拓展函数

#### 拓展类

#### 拓展模块

### 在c++中维护python对象

### 节点实现
