#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
輸出MEM階段的一些信號到"result.txt"
MEM_Mem_Read 判斷要不要將memory的內容傳給ReadData
MEM_Mem_Write 判斷要不要將register的內容傳給memory
在 passtoWB() 中，在將MEM階段的信號傳給WB
在 SetMEMStage() 中，將MEM階段的信號清除歸0
最後設置 STOP_MEM STOP_WB 狀態
*/

void passtoWB() {
    // 將MEM階段沒有用到的bit傳送至WB階段
    pipeline.WB_Reg_Write = pipeline.MEM_Reg_Write;
    pipeline.WB_Dest = pipeline.MEM_Dest;
    pipeline.WB_MemtoReg = pipeline.MEM_MemtoReg;
    pipeline.WB_Result = pipeline.MEM_Result;
}

void SetMEMStage() {
    // 將所有MEM階段的信號歸0
    pipeline.MEM_Branch = 0;
    pipeline.MEM_Mem_Read = 0, pipeline.MEM_Mem_Write = 0;
    pipeline.MEM_Reg_Write = 0;
    pipeline.MEM_MemtoReg = 0;
}

void MEM() {
    cout << "________________________MEM stage START____________________" << endl;
    // 輸出至"result.txt"
    if (pipeline.INSCYCLE[3][0] != '0') {
        fstream out;
        //要在文件後繼續寫out.open("result.txt", ios::out | ios::app);此方法ios::out | ios::app
        out.open("result.txt", ios::out | ios::app);

        out << "    " << pipeline.INSCYCLE[3] << ":MEM "<< pipeline.MEM_Branch << pipeline.MEM_Mem_Read << pipeline.MEM_Mem_Write << " " << pipeline.MEM_Reg_Write;

        if (pipeline.MEM_MemtoReg == 2) {
            out << 'X' << '\n';
        }else {
            out << pipeline.MEM_MemtoReg << '\n';
        }

        for (int i = 0; i < 4; i++) {
            pipeline.INSCYCLE[4][i] = pipeline.INSCYCLE[3][i];
            pipeline.INSCYCLE[3][i] = '0';//設為空
        }
        out.close();
    }

    // MEM_Mem_Read
    if (pipeline.MEM_Mem_Read == 1) {
        // 將[pipeline.MEM_Result]這個位置的memory內容傳給ReadData
        pipeline.ReadData = pipeline.mem[pipeline.MEM_Result];
    }
    // MEM_Mem_Write
    if (pipeline.MEM_Mem_Write == 1) {
        // 將[pipeline.MEM_Result]這個位置的register內容寫入第[pipeline.MEM_Result]個記憶體
        pipeline.mem[pipeline.MEM_Result] = pipeline.WriteData;
    }

    passtoWB();

    if (pipeline.prepre_forwarding == 1) {
        //判斷前前條指令是否forwarding
        cout << "前前指令有forwarding(這裡就不用傳結果)" << endl;
    }
    else if (pipeline.sw_forwarding) {
        //判斷sw是否forwarding
        cout << "sw前一個有forwarding(這裡就不用傳結果)" << endl;
    }
    else {
        cout << "無forwarding 要傳結果" << endl;
        pipeline.bWB_rd = pipeline.bMEM_rd;
        pipeline.WB_rd = pipeline.MEM_rd;
        pipeline.MEM_Forward_result = pipeline.MEM_Result;
    }

    SetMEMStage();

    //MEM結束,換WB
    pipeline.STOP_MEM = true;
    pipeline.STOP_WB = false;
    cout << "________________________MEM stage END____________________" << endl;
}
