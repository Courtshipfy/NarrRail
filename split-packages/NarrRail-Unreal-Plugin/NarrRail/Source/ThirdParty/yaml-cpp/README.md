# yaml-cpp 集成说明

## 下载步骤

1. 访问 https://github.com/jbeder/yaml-cpp/releases/tag/0.8.0
2. 下载源码压缩包
3. 解压后，将 `include` 目录复制到此处

## 目录结构

```
ThirdParty/yaml-cpp/
├── README.md (本文件)
└── include/
    └── yaml-cpp/
        ├── yaml.h
        ├── node/
        ├── parser.h
        └── ... (其他头文件)
```

## 或者使用 git

```bash
cd NarrRail/Source/ThirdParty
git clone --depth 1 --branch 0.8.0 https://github.com/jbeder/yaml-cpp.git
```

只需要保留 `include` 目录，其他文件可以删除。
