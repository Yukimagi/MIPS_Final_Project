#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
//PipelineRegisters pipeline;  // �w�q�����ܼ� pipeline

/*
�}�� "memory.txt" �èϥ� getline �v��Ū�����O
Ū������O��
�N���O���e3�Ӧr�Žƻs�� pipeline.INSCYCLE[0] �̭�
�æA���ڥ[�Jnull�פ��
�A�g�J "result.txt"
�P�_���Lstall
�N���O�ǰe�ܤU�@���q(ID)
�̫�]�m STOP_IF STOP_ID ���A
*/

void ReadInst(fstream& input) {
    string instemp = pipeline.instruction;
    bool nextline = false;// nextline �ΨӧP�_����Ū�����S�����O ��:PC+4 �L:��PC
    for (int i = 0; i <= pipeline.line; i++) {

        if (getline(input, pipeline.instruction)) {

            if (i == pipeline.line) {

                nextline = true;

                for (int i = 0; i < 3; i++){

                    pipeline.INSCYCLE[0][i] = pipeline.instruction[i];//�ǭӧO���O�i��
                }
                pipeline.INSCYCLE[0][3] = '\0';//�]�w�Ŧr�Ū�̫ܳ�@�Ӳ׵�(�H���U����P�_)
                fstream out;
                //�n�b�����~��gout.open("result.txt", ios::out | ios::app);����kios::out | ios::app
                out.open("result.txt", std::ios::out | std::ios::app);
                out << "	" << pipeline.INSCYCLE[0] << ":IF" << std::endl;
                out.close();
                if (pipeline.Stall_Count > 0) {//�p�G��stall�N���|�Ƿs�����O�i��(��ۤv���A��X�@��)
                    pipeline.STOP_ID = false;
                    pipeline.STOP_IF = false;
                    pipeline.instruction = instemp;
                    return;
                }
                for (int i = 0; i < 4; i++) {
                    pipeline.INSCYCLE[1][i] = pipeline.INSCYCLE[0][i]; //�N��e���O�ǰe�� ID ���q 
                    pipeline.INSCYCLE[0][i] = '0';  //�M��INSCYCLE[0][i] �ǳƱ����U�@�����O
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

    //IF����,��ID
    pipeline.STOP_IF = true;
    pipeline.STOP_ID = false;
}