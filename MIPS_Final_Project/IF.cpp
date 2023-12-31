#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
//PipelineRegisters pipeline;  // 定義全域變數 pipeline

/*
開啟 "memory.txt" 並使用 getline 逐行讀取指令
讀取到指令後
將指令的前3個字符複製到 pipeline.INSCYCLE[0] 裡面
並再尾巴加入null終止符
再寫入 "result.txt"
判斷有無stall
將指令傳送至下一階段(ID)
最後設置 STOP_IF STOP_ID 狀態
*/

/*
補充:
1.在IF階段的目的就是fetch instruction，因此把指令取出
2.重點是要判斷有無stall的情況，就像是branch中的predict not taken，
需要先直接執行下一指令的if階段，直到branch說明是下一指令繼續或跳。
3.將指令傳送至下一階段(ID)
4.最後設置 STOP_IF STOP_ID 狀態
*/


void ReadInst(fstream& input) {
    string instemp = pipeline.instruction;
    bool nextline = false;// nextline 用來判斷本次讀取有沒有指令 有:PC+4 無:原PC
    for (int i = 0; i <= pipeline.line ; i++) {

        if (getline(input, pipeline.instruction)) {

            if (i == pipeline.line) {

                nextline = true;

                for (int j = 0; j < 3; j++){

                    pipeline.INSCYCLE[0][j] = pipeline.instruction[j];//傳個別指令進來
                }
                pipeline.INSCYCLE[0][3] = '\0';//設定空字符表示最後一個終結(以幫助後續判斷)


                fstream out;
                //要在文件後繼續寫out.open("result.txt", ios::out | ios::app);此方法ios::out | ios::app
                out.open("result.txt", std::ios::out | std::ios::app);

                out << "	" << pipeline.INSCYCLE[0] << ":IF\n";

                out.close();

                if (pipeline.Stall_Count > 0) {//如果有stall就不會傳新的指令進來(把自己的再輸出一次)
                    pipeline.STOP_ID = false;
                    pipeline.STOP_IF = false;
                    pipeline.instruction = instemp;
                    return;
                }


                for (int i = 0; i < 4; i++) {
                    pipeline.INSCYCLE[1][i] = pipeline.INSCYCLE[0][i]; //將當前指令傳送至 ID 階段 
                    pipeline.INSCYCLE[0][i] = '0';  //清空INSCYCLE[0][i] 準備接收下一條指令
                }
            }
        }
    }
    if (nextline == true) {
        pipeline.line++;
    }
}
void IF() {
    
    fstream input;
    input.open("memory.txt");

    ReadInst(input);
    
    input.close();

    //IF結束,換ID
    pipeline.STOP_IF = true;
    pipeline.STOP_ID = false;
}