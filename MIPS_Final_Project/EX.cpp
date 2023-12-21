#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
輸出EX階段的一些信號到"result.txt"
在 process() 中，用 EX_ALUSrc 來判斷 R-format 或 I-format
在 passtoMEM() 中，在將EX階段的信號傳給MEM
在 SetEXStage() 中，將EX階段的信號清除歸0
最後設置 STOP_EX STOP_MEM 狀態
*/

void process() {
    if (pipeline.EX_ALUSrc == 0) // R-format
    {
        pipeline.MEM_Result = pipeline.rt + pipeline.rs; // 把目前的結果傳送至MEM階段(MEM_Result就是rd)
        cout << "/////" << pipeline.forwarding << endl;//forwarding的話就要直接把值拿來用，不能等到wb
        if (pipeline.forwarding) {//forwarding的話就要直接把值拿來用，不能等到wb
            if (pipeline.MEM_Forward_rt) {//要執行rt的forward
                pipeline.rt = pipeline.MEM_Forward_result;//rt抓forward值(MEM_Forward_result->前一個結果)
                cout << "1\n";
                pipeline.MEM_Forward_rt = 0;
            }
            else if (pipeline.EX_Forward_rt) {//要執行rt的forward
                pipeline.rt = pipeline.EX_Forward_rd;//rt抓forward值
                cout << "2\n";
                pipeline.EX_Forward_rt = 0;
            }
            if (pipeline.EX_Forward_rs) {//要執行rs的forward
                pipeline.rs = pipeline.EX_Forward_rd;//rs抓forward值
                cout << "3\n";
                pipeline.EX_Forward_rs = 0;
            }
            else if (pipeline.MEM_Forward_rs) {//要執行rs的forward
                pipeline.rs = pipeline.MEM_Forward_result;//rs抓forward值
                cout << "4\n";
                pipeline.MEM_Forward_rs = 0;
            }
            if (pipeline.INSCYCLE[3][0] == 's') {//要做sub
                cout << "進" << endl;
                pipeline.MEM_Result = pipeline.rs - pipeline.rt;//MEM_Result就是rd，直接把結果丟到MEM階段(目前的結果)
                cout << "MEM_Result = " << pipeline.MEM_Result << "\n";
            }
            else
                pipeline.MEM_Result = pipeline.rt + pipeline.rs;//MEM_Result就是rd，直接把結果丟到MEM階段(目前的結果)
            pipeline.forwarding = 0;
        }

        if (!pipeline.bforwarding) {//沒做beq forwarding(但是後續可能有beq，所以存提供判斷)
            pipeline.EX_Forward_rd = pipeline.MEM_Result;//把memory result的結果丟給EX_Forward_rd，為了在beq判斷是否相等
        }
        cout << "EX_Forward_rd: " << pipeline.EX_Forward_rd << endl;
    }
    else // I-format
    {
        pipeline.MEM_Result = pipeline.rs + pipeline.sign_extend;
        if (pipeline.sw_forwarding) {
            if (pipeline.EX_Forward_rt) pipeline.rt = pipeline.EX_Forward_rd;
            pipeline.MEM_Result = pipeline.rs + pipeline.sign_extend;
        }
        if (!pipeline.bforwarding) {
            pipeline.EX_Forward_rd = pipeline.MEM_Result;
        }
    }

    cout << "MEM_Result = " << pipeline.MEM_Result << "\n";

    //beq id判斷，但在ex跳(判斷要跳哪裡)
    if (pipeline.flag == 1) {
        pipeline.line = pipeline.line + pipeline.record;
        // cout << "branch" << endl;
        // beq_comfirm = true;//輸出所需
        pipeline.flag = 0;
    }

    pipeline.MEM_Dest = pipeline.rd;

    cout << "MEMresult: " << pipeline.MEM_Result << " MEM_Destination: " << pipeline.MEM_Dest << endl;
}

void passtoMEM() {
    // 設置MEM的WriteData
    pipeline.WriteData = pipeline.rt;

    // 將EX階段沒有用到的bit傳送至MEM階段
    pipeline.MEM_Branch = pipeline.EX_Branch;
    pipeline.MEM_Reg_Write = pipeline.EX_Reg_Write;
    pipeline.MEM_Mem_Read = pipeline.EX_Mem_Read;
    pipeline.MEM_Mem_Write = pipeline.EX_Mem_Write;
    pipeline.MEM_MemtoReg = pipeline.EX_MemtoReg;
    pipeline.MEM_rd = pipeline.EX_rd;
    pipeline.bMEM_rd = pipeline.bEX_rd;
}

void SetEXStage() {

    // 將所有EXE階段的信號歸0
    pipeline.EX_RegDst = 0;
    pipeline.EX_ALUSrc = 0;
    pipeline.EX_Branch = 0;
    pipeline.EX_Mem_Read = 0;
    pipeline.EX_Mem_Write = 0;
    pipeline.EX_Reg_Write = 0;
    pipeline.EX_MemtoReg = 0;
    pipeline.Zero = 0;
}

void EX() {

    // 輸出至"result.txt"
    if (pipeline.INSCYCLE[2][0] != '0') {
        fstream out;
        out.open("result.txt", ios::out | ios::app);
        out << "    " << pipeline.INSCYCLE[2] << ":EX ";
        if (pipeline.EX_RegDst == 2) {
            out << 'X';
        }
        else {
            out << pipeline.EX_RegDst;
        }
        out << pipeline.EX_ALUSrc << " " << pipeline.EX_Branch << pipeline.EX_Mem_Read << pipeline.EX_Mem_Write << " " << pipeline.EX_Reg_Write;
        if (pipeline.EX_MemtoReg == 2) {
            out << 'X' << endl;
        }
        else {
            out << pipeline.EX_MemtoReg << endl;
        }
        for (int i = 0; i < 4; i++) {
            pipeline.INSCYCLE[3][i] = pipeline.INSCYCLE[2][i];
            pipeline.INSCYCLE[2][i] = '0';
        }
        out.close();
    }

    process();
    
    passtoMEM();

    SetEXStage();

    //EX結束,換MEM
    pipeline.STOP_EX = true;
    pipeline.STOP_MEM = false;

}