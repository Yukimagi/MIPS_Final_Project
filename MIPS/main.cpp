#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

fstream out;
void writeResults() {
    // �g���G��result.txt
    out.open("result.txt", ios::out | ios::app);
    out << "�ݭn�� " << pipeline.cycles << " ��cycles" << endl;

    //��X�Ȧs������----------------------------------------------
    for (int i = 0; i < 32; i++) {
        out << "$" << i << " ";
    }
    out << endl;

    for (int i = 0; i < 32; i++) {
        out << " " << pipeline.reg[i] << " ";
        if (i > 10) { out << " "; }
    }
    out << endl;

    //��X�O���骺��----------------------------------------------
    for (int i = 0; i < 32; i++) {
        out << "W" << i << "  ";
    }
    out << endl;

    for (int i = 0; i < 32; i++) {
        out << " " << pipeline.mem[i] << "  ";
        if (i > 10) { out << " "; }
    }
    out << endl;

    out.close();//������ƿ�X��y
}

int main() {
    //�ѫ᩹�e����A�C�@round cycle++�Astall--
    static int maxline;

    //��l��pipeline�����A
    initialization();

    //while�]�@���N�O�@��cycle
    int temp = 0;
    fstream out;
    out.open("result.txt", ios::out);
    out.close();

    fstream get_maxline;
    get_maxline.open("memory.txt");

    while (getline(get_maxline, pipeline.instruction)) {
        temp++;
    }
    pipeline.instruction = "";
    get_maxline.close();
    maxline = temp;

    while (true)
    {

        out.open("result.txt", ios::out | ios::app);
        out << "Cycle " << pipeline.cycles << endl;
        cout << "Cycle " << pipeline.cycles << endl;
        out.close();

        // WB
        if (!pipeline.STOP_WB) {
            WB();
        }
        // MEM
        if (!pipeline.STOP_MEM) {
            MEM();
        }
        // EX
        if (!pipeline.STOP_EX) {
            EX();
        }
        // ID
        if (!pipeline.STOP_ID || pipeline.Stall_Count > 0) {
            ID();
        }
        //�p�G ��e�����O���==�ɮפ����̤j��ơA�N STOP_IF �]��true
        if (pipeline.line == maxline) {
            pipeline.STOP_IF = true;
        }
        else {
            IF();
        }
        cout << "stall_count: " << pipeline.Stall_Count << endl;
        pipeline.Stall_Count--;

        //�p�G �Ҧ����q������F�A�Nbreak
        if (pipeline.STOP_IF && pipeline.STOP_ID && pipeline.STOP_EX && pipeline.STOP_MEM && pipeline.STOP_WB) {
            break;
        }
        cout << "cycle" << pipeline.cycles << endl;
        cout << "cycle[0]" << pipeline.INSCYCLE[0] << endl;
        cout << "cycle[1]" << pipeline.INSCYCLE[1] << endl;
        cout << "cycle[2]" << pipeline.INSCYCLE[2] << endl;
        cout << "cycle[3]" << pipeline.INSCYCLE[3] << endl;
        cout << "cycle[4]" << pipeline.INSCYCLE[4] << endl;
        cout << "---------------------------------------------------" << endl;
        pipeline.cycles++;
    }

    writeResults();

    system("PAUSE");
    return 0;
}