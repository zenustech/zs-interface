# 构建 {#ZpcVerse_Build}


## 配置依赖

拉取项目代码：
```console
git clone https://github.com/zenustech/zs-interface.git --recursive
```

绝大多数依赖通过子模块引入一并构建。若clone时未带--recursive选项，则需手动更新子模块：
```console
git submodule update --init --recursive
```

其中有少量第三方依赖，如[vulkan](https://vulkan.lunarg.com/), llvm等，是以[find_package](https://cmake.org/cmake/help/latest/command/find_package.html)形式引用。
推荐开发者通过包管理器（apt、vcpkg、brew/macports等）安装或手动从源码构建使用。

## 构建项目

```console
cmake -Bbuild
cmake --build build --config Release --parallel 8 --target zs_interface
```
- **注意**：若想启用precompile header来加速编译，可在cmake configure时加入-DZS_ENABLE_PCH=ON选项。

## 文档构建
项目文档可见README.md以及doc目录，而服务于开发的API文档需自行安装doxygen并通过cmake构建。

```console
cmake -Bbuild -DZPC_BUILD_DOC=ON
cmake --build build --target doc
```
