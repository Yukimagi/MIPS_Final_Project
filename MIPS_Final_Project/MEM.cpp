#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
��XMEM���q���@�ǫH����"result.txt"
MEM_Mem_Read �P�_�n���n�Nmemory�����e�ǵ�ReadData
MEM_Mem_Write �P�_�n���n�Nregister�����e�ǵ�memory
�b passtoWB() ���A�b�NMEM���q���H���ǵ�WB
�b SetMEMStage() ���A�NMEM���q���H���M���k0
�̫�]�m STOP_MEM STOP_WB ���A
*/

void passtoWB() {
    // �NMEM���q�S���Ψ쪺bit�ǰe��WB���q
    pipeline.WB_Reg_Write = pipeline.MEM_Reg_Write;
    pipeline.WB_Dest = pipeline.MEM_Dest;
    pipeline.WB_MemtoReg = pipeline.MEM_MemtoReg;
    pipeline.WB_Result = pipeline.MEM_Result;
}

void SetMEMStage() {
    // �N�Ҧ�MEM���q���H���k0
    pipeline.MEM_Branch = 0;
    pipeline.MEM_Mem_Read = 0, pipeline.MEM_Mem_Write = 0;
    pipeline.MEM_Reg_Write = 0;
    pipeline.MEM_MemtoReg = 0;
}

void MEM() {
    cout << "________________________MEM stage START____________________" << endl;
    // ��X��"result.txt"
    if (pipeline.INSCYCLE[3][0] != '0') {
        fstream out;
        //�n�b�����~��gout.open("result.txt", ios::out | ios::app);����kios::out | ios::app
        out.open("result.txt", ios::out | ios::app);

        out << "    " << pipeline.INSCYCLE[3] << ":MEM "<< pipeline.MEM_Branch << pipeline.MEM_Mem_Read << pipeline.MEM_Mem_Write << " " << pipeline.MEM_Reg_Write;

        if (pipeline.MEM_MemtoReg == 2) {
            out << 'X' << '\n';
        }else {
            out << pipeline.MEM_MemtoReg << '\n';
        }

        for (int i = 0; i < 4; i++) {
            pipeline.INSCYCLE[4][i] = pipeline.INSCYCLE[3][i];
            pipeline.INSCYCLE[3][i] = '0';//�]����
        }
        out.close();
    }

    // MEM_Mem_Read
    if (pipeline.MEM_Mem_Read == 1) {
        // �N[pipeline.MEM_Result]�o�Ӧ�m��memory���e�ǵ�ReadData
        pipeline.ReadData = pipeline.mem[pipeline.MEM_Result];
    }
    // MEM_Mem_Write
    if (pipeline.MEM_Mem_Write == 1) {
        // �N[pipeline.MEM_Result]�o�Ӧ�m��register���e�g�J��[pipeline.MEM_Result]�ӰO����
        pipeline.mem[pipeline.MEM_Result] = pipeline.WriteData;
    }

    passtoWB();

    if (pipeline.prepre_forwarding == 1) {
        //�P�_�e�e�����O�O�_forwarding
        cout << "�e�e���O��forwarding(�o�̴N���ζǵ��G)" << endl;
    }
    else if (pipeline.sw_forwarding) {
        //�P�_sw�O�_forwarding
        cout << "sw�e�@�Ӧ�forwarding(�o�̴N���ζǵ��G)" << endl;
    }
    else {
        cout << "�Lforwarding �n�ǵ��G" << endl;
        pipeline.bWB_rd = pipeline.bMEM_rd;
        pipeline.WB_rd = pipeline.MEM_rd;
        pipeline.MEM_Forward_result = pipeline.MEM_Result;
    }

    SetMEMStage();

    //MEM����,��WB
    pipeline.STOP_MEM = true;
    pipeline.STOP_WB = false;
    cout << "________________________MEM stage END____________________" << endl;
}
