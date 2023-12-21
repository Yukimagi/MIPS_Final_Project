#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
PipelineRegisters pipeline;  // 定義全域變數 pipeline

void initialization() {
    //初始化 pipeline 裡面的變數

    //EX-------------------------------------------
    pipeline.EX_RegDst = 0, pipeline.EX_ALUOp1 = 0, pipeline.EX_ALUOp0 = 0, pipeline.EX_ALUSrc = 0;
    pipeline.EX_Branch = 0, pipeline.EX_Mem_Read = 0, pipeline.EX_Mem_Write = 0;
    pipeline.EX_Reg_Write = 0, pipeline.EX_MemtoReg = 0;
    pipeline.rs = 0, pipeline.rt = 0, pipeline.rd = 0, pipeline.sign_extend = 0;
    //MEM-------------------------------------------
    pipeline.MEM_Branch = 0, pipeline.MEM_Mem_Read = 0, pipeline.MEM_Mem_Write = 0;
    pipeline.MEM_Reg_Write = 0, pipeline.MEM_MemtoReg = 0;
    pipeline.MEM_Result = 0, pipeline.WriteData = 0, pipeline.Zero = 0, pipeline.MEM_Dest = 0;
    //WB-------------------------------------------
    pipeline.WB_Reg_Write = 0, pipeline.WB_MemtoReg = 0;
    pipeline.WB_Result = 0, pipeline.ReadData = 0, pipeline.WB_Dest = 0;

    // 將32個暫存器的值都設為1
    for (int i = 0; i < 32; ++i) {
        pipeline.mem[i] = 1;
        pipeline.reg[i] = 1;
    }
    pipeline.reg[0] = 0;

    // 初始化INSCYCLE為零
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            pipeline.INSCYCLE[i][j] = '0';
        }
    }
}