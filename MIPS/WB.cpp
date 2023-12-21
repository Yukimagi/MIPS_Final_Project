#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
��XWB���q���@�ǫH����"result.txt"
WB_Reg_Write �P�_�n���n�N��Ƽg�^register
WB_MemtoReg �P�_�g�JReadData�٬OWB_Result
�b SetWBStage() ���A�NWB���q���H���M���k0
�̫�]�m STOP_WB ���A
*/

void SetWBStage() {
    // �N�Ҧ�WB���q���H���k0
    pipeline.WB_Reg_Write = 0;
    pipeline.WB_MemtoReg = 0;
}

void WB() {
    // ��X��"result.txt"
    if (pipeline.INSCYCLE[4][0] != '0') {
        fstream out;
        out.open("result.txt", ios::out | ios::app);
        out << "    " << pipeline.INSCYCLE[4] << ":WB ";
        out << pipeline.WB_Reg_Write;
        if (pipeline.WB_MemtoReg == 2) {
            out << 'X' << endl;
        }
        else {
            out << pipeline.WB_MemtoReg << endl;
        }

        for (int i = 0; i < 4; i++) {
            pipeline.INSCYCLE[4][i] = '0';
        }
        out.close();
    }

    // �ھ�WB_Reg_Write�M�w�O�_�N��Ƽg�^register
    if (pipeline.WB_Reg_Write == 1) { // WB_Reg_Write == 1 �g�J
        if (pipeline.WB_MemtoReg == 1) {
            // �g�JReadData
            pipeline.reg[pipeline.WB_Dest] = pipeline.ReadData;
        }
        else {
            // �g�JWB_Result
            pipeline.reg[pipeline.WB_Dest] = pipeline.WB_Result;
            cout << "Reg[" << pipeline.WB_Dest << "]=" << pipeline.WB_Result << endl;
        }
    }

    void SetWBStage();

    //WB����
    pipeline.STOP_WB = true;
}