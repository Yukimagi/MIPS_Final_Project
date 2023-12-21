#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

fstream out;
void writeResults() {
    // 寫結果到result.txt
    out.open("result.txt", ios::out | ios::app);
    out << "需要花 " << pipeline.cycles << " 個cycles" << endl;

    //輸出暫存器的值----------------------------------------------
    for (int i = 0; i < 32; i++) {
        out << "$" << i << " ";
    }
    out << endl;

    for (int i = 0; i < 32; i++) {
        out << " " << pipeline.reg[i] << " ";
        if (i > 10) { out << " "; }
    }
    out << endl;

    //輸出記憶體的值----------------------------------------------
    for (int i = 0; i < 32; i++) {
        out << "W" << i << "  ";
    }
    out << endl;

    for (int i = 0; i < 32; i++) {
        out << " " << pipeline.mem[i] << "  ";
        if (i > 10) { out << " "; }
    }
    out << endl;

    out.close();//關閉資料輸出串流
}

int main() {
    //由後往前執行，每一round cycle++，stall--
    static int maxline;

    //初始化pipeline的狀態
    initialization();

    //while跑一輪就是一個cycle
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
        //如果 當前的指令行數==檔案中的最大行數，將 STOP_IF 設為true
        if (pipeline.line == maxline) {
            pipeline.STOP_IF = true;
        }
        else {
            IF();
        }
        cout << "stall_count: " << pipeline.Stall_Count << endl;
        pipeline.Stall_Count--;

        //如果 所有階段都停止了，就break
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