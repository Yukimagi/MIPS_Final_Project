#include "header.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
void setEX(int rd, int alusrc, int b, int mr, int mw, int rw, int mtr);
int GetRegValue_I(string instruction, int spaceIndex);
int GetRegValue_R(const std::string& instruction, int digit1, int digit2);
void GetSE_I();
void GetSE_R();
void ins();
int format;
int space[3];
int job;
int hoc_rs, hoc_rt, hoc_rd;
/*
* 這裡是針對指令進行處哩，並賦值
* 主要的方法是夠過空白來判斷格式
* 再進行分析
* 因為sw、lw是同種格式，因此他們會有共用一些function執行操作
* 因為beq、add、sub是同種格式，因此他們會有共用一些function執行操作
* 最難的部分在於要抓數字的部分，會需要做比較多的字串判斷
* 並且針對memory的部分也會需要多抓出來
*/
void ins() {
    // 我們是以sw、lw為I-Format則它會有2個空格，但如果是branch、add、sub則會有3個空格(所以輸入memory.txt需要注意格式，不然會做錯)
    if (format == 2) {//I-format
        if (pipeline.instruction[0] == 'l') {//lw
            setEX(0, 1, 0, 1, 0, 1, 1);
            job = 1;
            // 統一使用 rd 表示要寫回的寄存器，RegDst等同不用
            pipeline.rd = GetRegValue_I(pipeline.instruction, space[0]);
            pipeline.bEX_rd = pipeline.rd;//這裡額外存(for判斷前前指令是甚麼(ex:branch會需要判斷前前指令是否為lw需要stall的))
        }
        else if (pipeline.instruction[0] == 's') {//sw 
            setEX(2, 1, 0, 0, 1, 0, 2);
            job = 2;
            // rt(也是第一個暫存器)
            hoc_rt = GetRegValue_I(pipeline.instruction, space[0]);
            pipeline.rt = pipeline.reg[hoc_rt];
        }
        GetSE_I();
    }
    else if (format == 3) {//R-format(賦值ex)
        if (pipeline.instruction[0] == 'a') {
            setEX(1, 0, 0, 0, 0, 1, 0);
            job = 3;
        }
        else if (pipeline.instruction[0] == 's') {
            setEX(1, 0, 0, 0, 0, 1, 0);
            job = 3;
        }
        else if (pipeline.instruction[0] == 'b') {
            setEX(2, 0, 1, 0, 0, 0, 2);
            job = 4;
        }
        if (job == 3) {//找add、sub
            // find rd
            pipeline.rd = GetRegValue_R(pipeline.instruction, space[0] + 2, space[0] + 3);
            pipeline.EX_rd = pipeline.rd;

            // find rs
            hoc_rs = GetRegValue_R(pipeline.instruction, space[1] + 2, space[1] + 3);
            pipeline.rs = pipeline.reg[hoc_rs];

            GetSE_R();

        }
        else if (job == 4) {//做beq(同之前的方法)
            //因為指令format與r-format相同，所以可直接呼叫
            // find rs
            hoc_rs = GetRegValue_R(pipeline.instruction, space[0] + 2, space[0] + 3);
            pipeline.rs = pipeline.reg[hoc_rs];

            // find rt
            hoc_rt = GetRegValue_R(pipeline.instruction, space[1] + 2, space[1] + 3);
            pipeline.rt = -pipeline.reg[hoc_rt];
            //以下是我們額外用於除錯並判斷forward與branch
            if (pipeline.bforwarding) {
                if (pipeline.bMEM_Forward_rs) {
                    cout << "A\n";
                    pipeline.rs = pipeline.EX_Forward_rd;
                    pipeline.bMEM_Forward_rs = 0;
                }
                if (pipeline.bMEM_Forward_rt) {
                    cout << "B\n";
                    pipeline.rt = pipeline.EX_Forward_rd;
                    pipeline.bMEM_Forward_rt = 0;
                }
                pipeline.bforwarding = 0;
                cout << "rs=" << pipeline.rs << ", rt=" << pipeline.rt << "\n";
                if (pipeline.EX_Branch == 1 && (pipeline.rs + pipeline.rt == 0))//要BRANCH
                {
                    // stop = 1;
                    pipeline.record = pipeline.sign_extend;
                    // line = line + sign_extend;//在這裡直接計算BRANCH的下一個指令要讀第幾行，就不用再IF透過PCsrc做選擇
                    // 因為Branch判斷完會有一個INSCYCLEle的EX stall,避免再次執行Branch,所以清空Control Signal
                    pipeline.flag = 1;
                    cout << "branch" << endl;
                    pipeline.IS_beq = true;//輸出所需
                    // Stall_Count = 2;
                }
            }

            //找signed extend(要跳多遠)
            int temp = 1;
            pipeline.sign_extend = 0;
            if (pipeline.instruction[space[2] + 1] == '-') {
                for (int i = pipeline.instruction.length() - 1; i > space[2] + 1; i--) {
                    pipeline.sign_extend += ((int)pipeline.instruction[i] - '0') * temp;
                }
                pipeline.sign_extend = -pipeline.sign_extend;
            }
            else {
                for (int i = pipeline.instruction.length() - 1; i > space[2]; i--) {
                    pipeline.sign_extend += ((int)pipeline.instruction[i] - '0') * temp;
                }
            }
        }
    }
}
/*
* 在ID階段:
* 我們主要的操作會有decode instruction，所以我們會將對應的control值賦值
* 接著會針對stall、forwarding進行判斷
* 像是本次專題主要目標是forwarding，但針對lw依舊需要stall
* 因此我們會於這裡判斷lw的stall，與其他狀況(ex:stall一次後可能要forwarding)
* 而data hazrd中如果不用stall，就直接做forwarding，這邊會簡單的紀錄一下
* 其他詳細步驟請看以下
*/
void ID() {
    // stall判定,並且令stall期間的程式不會結束
    if (pipeline.Stall_Count > 0) {//stall 期間的輸出(有stall的話上一條指令會在輸出一次)
        pipeline.STOP_ID = false;
        if (pipeline.INSCYCLE[1][0] != '0')//空字符，表示終結前
        {
            fstream out;
            //要在文件後繼續寫out.open("result.txt", ios::out | ios::app);此方法ios::out | ios::app
            out.open("result.txt", ios::out | ios::app);
            out << "	" << pipeline.INSCYCLE[1] << ":ID" << endl;
            out.close();
        }
        return;
    }

    format = 0;
    //space[3];
    job = 0;
    hoc_rs = 0, hoc_rt = 0, hoc_rd = 0;
    //以下是計算空格(ins())判斷是什麼指令
    for (int i = 0; i < pipeline.instruction.length(); i++) {
        if (pipeline.instruction[i] == ' ') {
            space[format] = i;
            format++;
        }
    }
    //判斷指令
    ins();
    //定義對應的值(可以改普通的變數)
    int lw = 1, sw = 2, r_format = 3, beq = 4;

    if (job == lw) {//lw stall
        //*************************
        //16($0)
        // sign_extend->16/4=4
        // rs存原本位置
        pipeline.rs = hoc_rs;
        //hoc_rs->抓到目前的rs
        hoc_rs = hoc_rs + pipeline.sign_extend;//Memory要計算

        //拿現在的暫存器有沒有和前一條的暫存器相等
        //MEM_Dest上一個的rd，和目前的rs是否相等(相等則stall->2次，先歸零，最後還是會被賦值)
        if (pipeline.MEM_Dest == hoc_rs && pipeline.MEM_Reg_Write == 1) {
            pipeline.Stall_Count = 2;
            setEX(0, 0, 0, 0, 0, 0, 0);
        }
    }
    else if (job == sw) {//sw 遇到lw要stall
        // rs存原本位置
        pipeline.rs = hoc_rs;
        //rt直接對應暫存器不用計算
        //MEM_Dest上一個的rd，和目前的rt是否相等(相等則stall->2次，先歸零，最後還是會被賦值)
        if (pipeline.MEM_Dest == hoc_rt && pipeline.MEM_Reg_Write == 1) {
            pipeline.sw_forwarding = 1;//要forwarding(讓它在ex拿到)
            if (pipeline.MEM_Mem_Read == 1) {//上一個是不是lw(lw之MemRead == 1)是的話要stall
                if (pipeline.MEM_Dest == hoc_rs) {//要在hoc_rs(目前的rs暫存器拿值)
                    //但實際上在sw中Memory對應的不會被抓來給賦值
                    pipeline.MEM_Forward_rs = 1;//Memory 的 forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {//實際抓給ex的值只有rt的值
                    pipeline.MEM_Forward_rt = 1;
                }
                pipeline.Stall_Count = 1;//要做stall
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {//沒有stall (非lw)
                if (pipeline.MEM_Dest == hoc_rs) {//做forwarding
                    //要在hoc_rs(目前的rs暫存器拿值)
                    //但實際上在sw中Memory對應的不會被抓來給賦值
                    pipeline.EX_Forward_rs = 1;//EX 的 forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {//實際抓給ex的值只有rt的值
                    pipeline.EX_Forward_rt = 1;
                }
            }
        }
    }
    else if (job == r_format) {//r-format stall
        cout << "MEM_Dest=" << pipeline.MEM_Dest << ", hoc_rs=" << hoc_rs << ", WB_rd=" << pipeline.WB_rd
            << ", hoc_rt" << hoc_rt << "\n";
        //判斷目前的rs、rt會不是前一個的rd&RegWrite == 1
        if ((pipeline.MEM_Dest == hoc_rs || pipeline.MEM_Dest == hoc_rt) && pipeline.MEM_Reg_Write == 1) {
            //forwarding = 1;
            if (pipeline.MEM_Mem_Read == 1) {//判斷是不是lw(lw要做stall)
                if (pipeline.MEM_Dest == hoc_rs) {//Memory 的 forwarding
                    pipeline.MEM_Forward_rs = 1;
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.MEM_Forward_rt = 1;
                }
                pipeline.Stall_Count = 1;//做stall
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {
                pipeline.forwarding = 1;//不用stall做forwarding
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.EX_Forward_rs = 1;//EX 的 forwarding
                    cout << "我是rs\n";
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.EX_Forward_rt = 1;
                }
            }
        }
        //判斷前前條指令(一樣的概念)
        if ((pipeline.WB_rd == hoc_rs || pipeline.WB_rd == hoc_rt) && pipeline.MEM_Reg_Write == 1 && pipeline.WB_rd != pipeline.MEM_Dest) {
            if (pipeline.MEM_Mem_Read == 1) {
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {
                pipeline.prepre_forwarding = 1;
                pipeline.forwarding = 1;
                if (pipeline.WB_rd == hoc_rs) {
                    pipeline.MEM_Forward_rs = 1;
                }
                if (pipeline.WB_rd == hoc_rt) {
                    pipeline.MEM_Forward_rt = 1;
                }
            }
        }
    }
    else if (job == beq) {//job stall
        //前一條的rd=rs或rt且MEM_Reg_Write == 1
        if ((pipeline.MEM_Dest == hoc_rs || pipeline.MEM_Dest == hoc_rt) && pipeline.MEM_Reg_Write == 1) {
            if (pipeline.MEM_Mem_Read == 1) {//lw
                pipeline.Stall_Count = 2;//stall 2次
                pipeline.bforwarding = 1;//做branch forwarding
                pipeline.count_beq = 1;//現在這個回合beq要停幾次
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.MEM_Forward_rs = 1;//Memory 的 forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.MEM_Forward_rt = 1;
                }
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {//stall 1次
                pipeline.Stall_Count = 1;//r-format
                pipeline.bforwarding = 1;//forwarding
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.bMEM_Forward_rs = 1;//Memory 的 forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.bMEM_Forward_rt = 1;
                }
            }
        }
        else {
            cout << "*****" << endl;
            pipeline.instruction = "";
            setEX(2, 0, 1, 0, 0, 0, 2);
        }
    }

    
    if (pipeline.IS_beq) {//如果beq判定要跳轉(本來是在ex才會有判斷結果，但太慢了，
        //所以我們改到id先判斷是否跳(同課本說明)，但ex才執行跳) 則先前IF指令部分不傳入ID
        pipeline.INSCYCLE[1][0] = '0';
        pipeline.IS_beq = false;
        pipeline.Stall_Count = 1;
    }
    //印ID
    if (pipeline.INSCYCLE[1][1] == 'e' && pipeline.Stall_Count == 1)//過程中會遺失值，強制改回(beq)
    {
        pipeline.INSCYCLE[1][0] = 'b';
        fstream out;
        out.open("result.txt", ios::out | ios::app);
        out << "    " << pipeline.INSCYCLE[1] << ":ID" << endl;
        out.close();
    }
    else if (pipeline.INSCYCLE[1][0] != '0') {//正常沒變值
        fstream out;
        out.open("result.txt", ios::out | ios::app);
        out << "    " << pipeline.INSCYCLE[1] << ":ID" << endl;
        out.close();
    }

    //FOR TESTING
    cout << "job: " << job << endl;
    if (pipeline.Stall_Count < 2 && job == beq) {
        pipeline.count_beq++;
        cout << "count_beq: " << pipeline.count_beq << endl;
        if (pipeline.count_beq == 2) {
            for (int i = 0; i < 4; i++)
            {
                pipeline.INSCYCLE[2][i] = pipeline.INSCYCLE[1][i];
                pipeline.INSCYCLE[1][i] = '0';
            }
            pipeline.count_beq = 0;
        }
    }
    else if (pipeline.Stall_Count < 1) {
        for (int i = 0; i < 4; i++)
        {
            pipeline.INSCYCLE[2][i] = pipeline.INSCYCLE[1][i];
            pipeline.INSCYCLE[1][i] = '0';
        }
    }
    else {

    }

    cout << "hoc_rs: " << hoc_rs << " hoc_rt: " << hoc_rt << endl;
    cout << "rd: " << pipeline.rd << " rs: " << pipeline.rs << " rt: " << pipeline.rt << " sign_extend: " << pipeline.sign_extend << endl;
    cout << "writedata: " << pipeline.WriteData << endl;
    //---------------
    //over
    pipeline.STOP_ID = true;  pipeline.STOP_EX = false;
}

//ps.以下賦值中2代表X
//設定 ex control table的值
void setEX(int rd, int alusrc, int b, int mr, int mw, int rw, int mtr) {
    pipeline.EX_RegDst = rd;
    pipeline.EX_ALUSrc = alusrc;
    pipeline.EX_Branch = b;
    pipeline.EX_Mem_Read = mr,
        pipeline.EX_Mem_Write = mw;
    pipeline.EX_Reg_Write = rw;
    pipeline.EX_MemtoReg = mtr;
}
//c++字串轉數字的參考網站
// https://shengyu7697.github.io/cpp-string-to-integer/
//負責執行reg對reg時 I-format找暫存器
int GetRegValue_I(string instruction, int spaceIndex) {

    //3和2是判斷暫存器是十位數還是個位數(ex:16($2))
    // 判斷暫存器是十位數還是個位數(ex:16($2))
    if (instruction[spaceIndex + 3] >= '0' && instruction[spaceIndex + 3] <= '9') {
        // rd找第一個值
        return ((int)instruction[spaceIndex + 3] - '0') + ((int)instruction[spaceIndex + 2] - '0') * 10;
    }
    else if (instruction[spaceIndex + 2] >= '0' && instruction[spaceIndex + 2] <= '9') {
        return ((int)instruction[spaceIndex + 2] - '0');
    }
    return -1;
}

//負責執行reg對reg時 R-format找暫存器
int GetRegValue_R(const std::string& instruction, int digit1, int digit2) {
    if (instruction[digit2] >= '0' && instruction[digit2] <= '9') {
        return ((int)instruction[digit2] - '0') + ((int)instruction[digit1] - '0') * 10;
    }
    else if (instruction[digit1] >= '0' && instruction[digit1] <= '9') {
        return ((int)instruction[digit1] - '0');
    }
    return -1;
}
//負責執行I-format找對應的位移
void GetSE_I() {
    //因為需要抓memory中的暫存器做判斷，但我們主要是暴力解，讓他最多算兩位數，要記得/4(word)
    if (space[1] + 1 == '-') {//有負號的
        if (pipeline.instruction[space[1] + 3] >= '0' && pipeline.instruction[space[1] + 3] <= '9') {
            pipeline.sign_extend = -(((int)pipeline.instruction[space[1] + 3] - 48) + ((int)pipeline.instruction[space[1] + 2] - 48) * 10) / 4;
            if (pipeline.instruction[space[1] + 6] >= '0' && pipeline.instruction[space[1] + 6] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 6] - '0') + ((int)pipeline.instruction[space[1] + 5] - '0') * 10;
                pipeline.rs = pipeline.reg[hoc_rs];
            }
            else if (pipeline.instruction[space[1] + 5] >= '0' && pipeline.instruction[space[1] + 5] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 5] - '0');
                pipeline.rs = pipeline.reg[hoc_rs];
            }
        }
        else if (pipeline.instruction[space[1] + 2] >= '0' && pipeline.instruction[space[1] + 2] <= '9') {
            pipeline.sign_extend = -(((int)pipeline.instruction[space[1] + 2] - '0')) / 4;
            if (pipeline.instruction[space[1] + 5] >= '0' && pipeline.instruction[space[1] + 5] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 5] - '0') + ((int)pipeline.instruction[space[1] + 4] - '0') * 10;
                pipeline.rs = pipeline.reg[hoc_rs];
            }
            else if (pipeline.instruction[space[1] + 4] >= '0' && pipeline.instruction[space[1] + 4] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 4] - '0');
                pipeline.rs = pipeline.reg[hoc_rs];
            }
        }
    }
    else {//沒負號的
        if (pipeline.instruction[space[1] + 2] >= '0' && pipeline.instruction[space[1] + 2] <= '9') {
            //要執行的位移/4
            pipeline.sign_extend = (((int)pipeline.instruction[space[1] + 2] - '0') + ((int)pipeline.instruction[space[1] + 1] - '0') * 10) / 4;
            if (pipeline.instruction[space[1] + 6] >= 48 && pipeline.instruction[space[1] + 6] <= '9') {
                //Memory 中的暫存器的名字
                hoc_rs = ((int)pipeline.instruction[space[1] + 6] - '0') + ((int)pipeline.instruction[space[1] + 5] - '0') * 10;
                //Memory 中的暫存器的實際的值
                pipeline.rs = pipeline.reg[hoc_rs];
            }
            else if (pipeline.instruction[space[1] + 5] >= '0' && pipeline.instruction[space[1] + 5] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 5] - '0');
                pipeline.rs = pipeline.reg[hoc_rs];
            }
        }
        else if (pipeline.instruction[space[1] + 1] >= '0' && pipeline.instruction[space[1] + 1] <= '9') {
            pipeline.sign_extend = (((int)pipeline.instruction[space[1] + 1] - '0')) / 4;
            if (pipeline.instruction[space[1] + 5] >= '0' && pipeline.instruction[space[1] + 5] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 5] - '0') + ((int)pipeline.instruction[space[1] + 4] - '0') * 10;
                pipeline.rs = pipeline.reg[hoc_rs];
            }
            else if (pipeline.instruction[space[1] + 4] >= '0' && pipeline.instruction[space[1] + 4] <= '9') {
                hoc_rs = ((int)pipeline.instruction[space[1] + 4] - '0');
                pipeline.rs = pipeline.reg[hoc_rs];
            }
        }
    }
}
//負責執行R-format找rd並做取正負判斷
void GetSE_R() {
    // find rt 分為正負 add 取正  sub 取負
    if (pipeline.instruction[0] == 'a') {
        if (pipeline.instruction[space[2] + 1] == '$') {
            if (pipeline.instruction[space[2] + 3] >= '0' && pipeline.instruction[space[2] + 3] <= '9') {
                //找第三個(暫存器的名字)
                hoc_rt = ((int)pipeline.instruction[space[2] + 3] - '0') + ((int)pipeline.instruction[space[2] + 2] - '0') * 10;
                //找第三個(暫存器的對應的值)
                pipeline.rt = pipeline.reg[hoc_rt];
            }
            else if (pipeline.instruction[space[2] + 2] >= 48 && pipeline.instruction[space[2] + 2] <= '9') {
                hoc_rt = ((int)pipeline.instruction[space[2] + 2] - '0');
                pipeline.rt = pipeline.reg[hoc_rt];
            }
        }
        else {// find rt=最後的常數
            //判斷add最後一個指令是不是常數
            int temp = 1;
            pipeline.rt = 0;
            if (space[2] + 1 == '-') {
                for (int i = pipeline.instruction.length() - 1; i > space[2] + 1; i--) {
                    //存對應的數字 hoc_rt會存到前一個指令 所以這裡設為-1
                    hoc_rt = -1;
                    pipeline.rt += ((int)pipeline.instruction[i] - '0') * temp;
                }
                pipeline.rt = -pipeline.rt;
            }
            else {
                for (int i = pipeline.instruction.length() - 1; i > space[2]; i--) {
                    hoc_rt = -1;
                    pipeline.rt += ((int)pipeline.instruction[i] - '0') * temp;
                }
            }
        }
    }
    //同上面add(這裡是)sub
    else if (pipeline.instruction[0] == 's') {
        if (pipeline.instruction[space[2] + 1] == '$') {
            if (pipeline.instruction[space[2] + 3] >= '0' && pipeline.instruction[space[2] + 3] <= '9') {
                hoc_rt = ((int)pipeline.instruction[space[2] + 3] - '0') + ((int)pipeline.instruction[space[2] + 2] - '0') * 10;
                pipeline.rt = -pipeline.reg[hoc_rt];
            }
            else if (pipeline.instruction[space[2] + 2] >= 48 && pipeline.instruction[space[2] + 2] <= '9') {
                hoc_rt = ((int)pipeline.instruction[space[2] + 2] - '0');
                pipeline.rt = -pipeline.reg[hoc_rt];
            }
        }
        else {// find rt=最後的常數
            int temp = 1;
            pipeline.rt = 0;
            if (space[2] + 1 == '-') {
                for (int i = pipeline.instruction.length() - 1; i > space[2] + 1; i--) {
                    hoc_rt = -1;
                    pipeline.rt += ((int)pipeline.instruction[i] - '0') * temp;
                }
            }
            else {
                for (int i = pipeline.instruction.length() - 1; i > space[2]; i--) {
                    hoc_rt = -1;
                    pipeline.rt += ((int)pipeline.instruction[i] - '0') * temp;
                }
                pipeline.rt = -pipeline.rt;
            }
        }
    }
}