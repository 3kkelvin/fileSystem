# File System

## Team member
- 112368006 崔士豪
- 112368402 游亮濤
- 112368075 廖珞昱
- 112368077 吳明憲

## 編譯及執行
```
mingw32-make
//or
make
```
```
.\run.exe      
```

## 執行檔案資料夾
```
(Clone 資料夾)
├── command.c
├── command.h
├── dump.c
├── dump.h
│     .
│     .
│     .
├── run.exe // 執行檔案
├── my_fs.dump // 存檔的.dump檔案
│
└── file_system // 要使用put指令的檔案需放在這個資料夾裏！！！
    ├── photo.png
    ├── a.txt
    └── dump // 使用get指令取得的程式會存在這裏
        └── a.txt
```