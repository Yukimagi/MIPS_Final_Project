#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
輸出WB階段的一些信號到"result.txt"
WB_Reg_Write 判斷要不要將資料寫回register
WB_MemtoReg 判斷寫入ReadData還是WB_Result
在 SetWBStage() 中，將WB階段的信號清除歸0
最後設置 STOP_WB 狀態
*/

void SetWBStage() {
    // 將所有WB階段的信號歸0
    pipeline.WB_Reg_Write = 0;
    pipeline.WB_MemtoReg = 0;
}

void WriteB(){
    // 根據WB_Reg_Write決定是否將資料寫回register
    if (pipeline.WB_Reg_Write == 1) { // WB_Reg_Write == 1 寫入
        if (pipeline.WB_MemtoReg == 1) {
            // 寫入ReadData
            pipeline.reg[pipeline.WB_Dest] = pipeline.ReadData;
        }
        else {
            // 寫入WB_Result
            pipeline.reg[pipeline.WB_Dest] = pipeline.WB_Result;
            cout << "Reg[" << pipeline.WB_Dest << "]=" << pipeline.WB_Result << endl;
        }
    }
}

void WB() {
    cout << "________________________WB stage START____________________" << endl;
    // 輸出至"result.txt"
    if (pipeline.INSCYCLE[4][0] != '0') {
        fstream out;
        //要在文件後繼續寫out.open("result.txt", ios::out | ios::app);此方法ios::out | ios::app
        out.open("result.txt", ios::out | ios::app);

        out << "    " << pipeline.INSCYCLE[4] << ":WB "<< pipeline.WB_Reg_Write;

        if (pipeline.WB_MemtoReg == 2) {
            out << 'X' << '\n';
        }
        else {
            out << pipeline.WB_MemtoReg << '\n';
        }

        for (int i = 0; i < 4; i++) {
            pipeline.INSCYCLE[4][i] = '0';//設為空
        }
        out.close();
    }

    WriteB();

    SetWBStage();

    //WB結束
    pipeline.STOP_WB = true;
    cout << "________________________WB stage END____________________" << endl;
}