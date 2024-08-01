# 项目初始化&启动

## 环境准备

确保正确安装 [Node.js](https://nodejs.org/en/) 且版本为 16+ 即可。

```bash
$ node -v
v16.14.2
```

确保已安装[pnpm](https://pnpm.io/installation#using-npm)

```bash
$ pnpm -v
8.8.0
```

## trustflow_verifier.js 准备
参考 [TrustFlow Attestation Library CMake Build](../../../../REMADME.md#CMake) 构建 `trustflow_verifier.js`

将 `trustflow_verifier.js` 拷贝到 `src/pages/index` 目录下

## 初始化项目

在项目根目录运行

```bash
$ pnpm i
```

## 启动项目

在项目根目录运行

```bash
$ pnpm dev
```

此时访问(http://localhost:8000) 即可进入
