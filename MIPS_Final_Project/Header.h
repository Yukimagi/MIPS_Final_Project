#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct PipelineRegisters {

	int mem[32] = { 0 };
	int reg[32] = { 0 };

	int line = 0;
	//EX-------------------------------------------
	int EX_RegDst, EX_ALUOp1, EX_ALUOp0, EX_ALUSrc;//ex
	int EX_Branch, EX_Mem_Read, EX_Mem_Write;//mem
	int EX_Reg_Write, EX_MemtoReg;//wb
	int rs, rt, rd, sign_extend;



	//MEM-------------------------------------------
	int MEM_Branch, MEM_Mem_Read, MEM_Mem_Write;//mem
	int MEM_Reg_Write, MEM_MemtoReg;//wb
	int MEM_Result, WriteData, Zero, MEM_Dest;



	//WB-------------------------------------------
	int WB_Reg_Write, WB_MemtoReg;//wb
	int WB_Result, ReadData, WB_Dest;
	int EX_rd, MEM_rd, WB_rd;
	
	int bEX_rd, bMEM_rd, bWB_rd;




	string instruction; //指令
	char INSCYCLE[5][4]; //儲存指令的陣列
	int cycles = 1; //cycle數
	int Stall_Count = 0; //判斷stall

	//各種forward判斷 b開頭:branch pre:前一個
	int forwarding = 0, prepre_forwarding = 0, bforwarding = 0, bprepre_forwarding = 0;
	int beq_forwarding = 0, sw_forwarding = 0;
	int bMEM_Forward_rt, bMEM_Forward_rs, MEM_Forward_rd, MEM_Forward_rt, MEM_Forward_rs, EX_Forward_rd, EX_Forward_rt, EX_Forward_rs, MEM_Forward_result;
	
	//判斷該階段(IF ID EX MEM WB)是否結束
	bool STOP_IF = false, STOP_ID = false, STOP_EX = false, STOP_MEM = false, STOP_WB = false;
	bool IS_beq = false;

	int count_beq = 0; //該回合beq要停幾次
	int record = 0; //紀錄sign-extend(ID)
	int flag = 0;
};

extern PipelineRegisters pipeline;  // 全域變數 pipeline 的宣告

void WB();
void MEM();
void EX();
void ID();
void IF();
void initialization();

#endif  

