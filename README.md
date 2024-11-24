OrangeDecryptor
---------------

一个用于解密 100% Orange Juice 资源包的小工具。

参见：
* https://jixun.moe/post/100-orange-decryptor
* https://jixun.uk/posts/2018/100-orange-data-unpack-v1/

## 原始说明

100% Orange Juice - 资源解包工具 v1.1

开发: [Jixun](https://jixun.moe)

支持 XP 或以上操作系统（暂定）。

支持的加密/打包类型:

- `.dat` 解密
- 伪装为 ZIP 压缩包，内部包含后缀名为 .dat 的加密文件 (如 `animation.pak`)
- 将合并后的 ogg 拆分成单独的文件 (如 `bgm.pak`)
- 将合并后的 wav 拆分成单独的文件 (如 `se.pak`)

调用参数：

    OrangeDecryptor <输入文件> <输出路径/目录> [模式]

不带模式的话，将尝试自动识别。
