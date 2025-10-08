
###
- 要求只处理 tif 格式。其它条件我帮你设一个比较合理的默认：
- 文件大小：≥10KB，≤200MB（避免误读空文件或超大异常文件）
- 最多加载：2000 张图（避免一次性读太多）
- 内容总字节上限：8GB（虽然预留了10GB，保险起见不把内存填爆
- hello_windows_memory1.exe "D:\0.人工判定原图\AAC-EVT-V54-0318生产原图" --types=tif --min=10kb --max=200mb --max-files=2000 --max-bytes=8gb
```shell
hello_windows_memory1.exe "D:\0.人工判定原图\AAC-EVT-V54-0318生产原图" ^
  --types=tif ^
  --min=10kb --max=200mb ^
  --max-files=2000 ^
  --max-bytes=8gb

```