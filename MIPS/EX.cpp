#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
��XEX���q���@�ǫH����"result.txt"
�b process() ���A�� EX_ALUSrc �ӧP�_ R-format �� I-format
�b passtoMEM() ���A�b�NEX���q���H���ǵ�MEM
�b SetEXStage() ���A�NEX���q���H���M���k0
�̫�]�m STOP_EX STOP_MEM ���A
*/

void process() {
    if (pipeline.EX_ALUSrc == 0) // R-format
    {
        pipeline.MEM_Result = pipeline.rt + pipeline.rs; // ��ثe�����G�ǰe��MEM���q(MEM_Result�N�Ord)
        cout << "/////" << pipeline.forwarding << endl;//forwarding���ܴN�n������Ȯ��ӥΡA���൥��wb
        if (pipeline.forwarding) {//forwarding���ܴN�n������Ȯ��ӥΡA���൥��wb
            if (pipeline.MEM_Forward_rt) {//�n����rt��forward
                pipeline.rt = pipeline.MEM_Forward_result;//rt��forward��(MEM_Forward_result->�e�@�ӵ��G)
                cout << "1\n";
                pipeline.MEM_Forward_rt = 0;
            }
            else if (pipeline.EX_Forward_rt) {//�n����rt��forward
                pipeline.rt = pipeline.EX_Forward_rd;//rt��forward��
                cout << "2\n";
                pipeline.EX_Forward_rt = 0;
            }
            if (pipeline.EX_Forward_rs) {//�n����rs��forward
                pipeline.rs = pipeline.EX_Forward_rd;//rs��forward��
                cout << "3\n";
                pipeline.EX_Forward_rs = 0;
            }
            else if (pipeline.MEM_Forward_rs) {//�n����rs��forward
                pipeline.rs = pipeline.MEM_Forward_result;//rs��forward��
                cout << "4\n";
                pipeline.MEM_Forward_rs = 0;
            }
            if (pipeline.INSCYCLE[3][0] == 's') {//�n��sub
                cout << "�i" << endl;
                pipeline.MEM_Result = pipeline.rs - pipeline.rt;//MEM_Result�N�Ord�A�����⵲�G���MEM���q(�ثe�����G)
                cout << "MEM_Result = " << pipeline.MEM_Result << "\n";
            }
            else
                pipeline.MEM_Result = pipeline.rt + pipeline.rs;//MEM_Result�N�Ord�A�����⵲�G���MEM���q(�ثe�����G)
            pipeline.forwarding = 0;
        }

        if (!pipeline.bforwarding) {//�S��beq forwarding(���O����i�঳beq�A�ҥH�s���ѧP�_)
            pipeline.EX_Forward_rd = pipeline.MEM_Result;//��memory result�����G�ᵹEX_Forward_rd�A���F�bbeq�P�_�O�_�۵�
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

    //beq id�P�_�A���bex��(�P�_�n������)
    if (pipeline.flag == 1) {
        pipeline.line = pipeline.line + pipeline.record;
        // cout << "branch" << endl;
        // beq_comfirm = true;//��X�һ�
        pipeline.flag = 0;
    }

    pipeline.MEM_Dest = pipeline.rd;

    cout << "MEMresult: " << pipeline.MEM_Result << " MEM_Destination: " << pipeline.MEM_Dest << endl;
}

void passtoMEM() {
    // �]�mMEM��WriteData
    pipeline.WriteData = pipeline.rt;

    // �NEX���q�S���Ψ쪺bit�ǰe��MEM���q
    pipeline.MEM_Branch = pipeline.EX_Branch;
    pipeline.MEM_Reg_Write = pipeline.EX_Reg_Write;
    pipeline.MEM_Mem_Read = pipeline.EX_Mem_Read;
    pipeline.MEM_Mem_Write = pipeline.EX_Mem_Write;
    pipeline.MEM_MemtoReg = pipeline.EX_MemtoReg;
    pipeline.MEM_rd = pipeline.EX_rd;
    pipeline.bMEM_rd = pipeline.bEX_rd;
}

void SetEXStage() {

    // �N�Ҧ�EXE���q���H���k0
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

    // ��X��"result.txt"
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

    //EX����,��MEM
    pipeline.STOP_EX = true;
    pipeline.STOP_MEM = false;

}