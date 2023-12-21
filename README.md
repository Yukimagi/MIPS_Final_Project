# MIPS 期末專題(Forwarding+Branch Predict Not Taken) 
## 程式語言:C++/編譯器版本
### Using built-in specs.
### COLLECT_GCC=g++
### COLLECT_LTO_WRAPPER=c:/mingw/bin/../libexec/gcc/mingw32/6.3.0/lto-wrapper.exe
### Target: mingw32
### Configured with: ../src/gcc-6.3.0/configure --build=x86_64-pc-linux-gnu --host=mingw32 --with-gmp=/mingw --with-mpfr=/mingw --with- mpc=/mingw --with-isl=/mingw --prefix=/mingw --disable-win32-registry --target=mingw32 --with-arch=i586 --enable-languages=c,c++,objc,obj-c++,fortran,ada --with-pkgversion='MinGW.org GCC-6.3.0-1' --enable-static --enable-shared --enable-threads --with-dwarf2 --disable-sjlj-exceptions --enable-version-specific-runtime-libs --with-libiconv-prefix=/mingw --with-libintl-prefix=/mingw --enable-libstdcxx-debug --with-tune=generic --enable-libgomp --disable-libvtv --enable-nls
### Thread model: win32
### gcc version 6.3.0 (MinGW.org GCC-6.3.0-1)
## Include的套件(基本上不需要額外裝)
#### include <iostream>
### include <fstream>
### include <string>

# 關於本程式的class
### Header.h
* 你可以於此程式中看到全部的變數(我們用extern的方法，使我們可以共用變數)，並且基本定義我們會用到的IF、ID、EX、MEM、WB函式
### Header.cpp
* 我們在這個程式中定義了初始化變數的函式
### IF.cpp
* 執行IF階段應完成的事項
* (讀入指令，並存放到變數，其中有可能需要stall
* 只要有進行此stage會印出此stage到result.txt，
* (ex:branch的predict not taken)，另外結束後傳值給ID stage)
### ID.cpp
* 執行ID階段應完成的事項
* (執行decode instruction，判斷指令，並賦予對應的opcode，
* 此階段也有需要針對stall、forwarding判斷
* 只要有進行此stage會印出此stage到result.txt，
* ，另外結束後傳值給EX stage)
### EX.cpp
* 執行EX階段應完成的事項
* (做到運算，並提供forwarding等...功能，branch指令結束後的判斷等等...
* 只要有進行此stage會印出此stage到result.txt，
* ，另外結束後傳直給MEM stage)
### MEM.cpp
* 執行IF階段應完成的事項
* MEM_Mem_Read 判斷要不要將memory的內容傳給ReadData
* MEM_Mem_Write 判斷要不要將register的內容傳給memory
* 傳值給WB，另外結束後傳直給WB stage)
### WB.cpp
* 執行IF階段應完成的事項
* WB_Reg_Write 判斷要不要將資料寫回register
* WB_MemtoReg 判斷寫入ReadData還是WB_Result
* 最後設置 STOP_WB 狀態
#### 下圖是簡單的流程圖:
### ![](./Flowchart.png)

# How to run
### 你可以下載下來後，到mips的資料夾中(要將cmd導入到此資料夾)
### 接著寫入以下於cmd使產生exe檔
### 當產生exe檔後將要操作的題目依照以下指示:
* 在執行程式碼之前，請記得先將要測試的題目複製到memory.txt中，須注意由於我們的程是碼會判斷格式，所以需要依照我們提供的程式碼的格式進行輸入，如果有跑錯，可能會是你的格式有誤
* (memory1.txt對應ppt第一題，以此類推...)
* (在cmd可直接輸入main)
* (ppt輸出格式會於result.txt)
```bash
g++ -c Header.cpp IF.cpp ID.cpp EX.cpp MEM.cpp WB.cpp main.cpp
g++ -o main Header.o IF.o ID.o EX.o MEM.o WB.o main.o

./main
```


# 執行結果中cmd事我們寫程式時的debug內容，相對應的輸出會於result.txt

