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
* �o�̬O�w����O�i��B���A�ý��
* �D�n����k�O���L�ťըӧP�_�榡
* �A�i����R
* �]��sw�Blw�O�P�خ榡�A�]���L�̷|���@�Τ@��function����ާ@
* �]��beq�Badd�Bsub�O�P�خ榡�A�]���L�̷|���@�Τ@��function����ާ@
* �����������b��n��Ʀr�������A�|�ݭn������h���r��P�_
* �åB�w��memory�������]�|�ݭn�h��X��
*/
void ins() {
    // �ڭ̬O�Hsw�Blw��I-Format�h���|��2�ӪŮ�A���p�G�Obranch�Badd�Bsub�h�|��3�ӪŮ�(�ҥH��Jmemory.txt�ݭn�`�N�榡�A���M�|����)
    if (format == 2) {//I-format
        if (pipeline.instruction[0] == 'l') {//lw
            setEX(0, 1, 0, 1, 0, 1, 1);
            job = 1;
            // �Τ@�ϥ� rd ��ܭn�g�^���H�s���ARegDst���P����
            pipeline.rd = GetRegValue_I(pipeline.instruction, space[0]);
            pipeline.bEX_rd = pipeline.rd;//�o���B�~�s(for�P�_�e�e���O�O�ƻ�(ex:branch�|�ݭn�P�_�e�e���O�O�_��lw�ݭnstall��))
        }
        else if (pipeline.instruction[0] == 's') {//sw 
            setEX(2, 1, 0, 0, 1, 0, 2);
            job = 2;
            // rt(�]�O�Ĥ@�ӼȦs��)
            hoc_rt = GetRegValue_I(pipeline.instruction, space[0]);
            pipeline.rt = pipeline.reg[hoc_rt];
        }
        GetSE_I();
    }
    else if (format == 3) {//R-format(���ex)
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
        if (job == 3) {//��add�Bsub
            // find rd
            pipeline.rd = GetRegValue_R(pipeline.instruction, space[0] + 2, space[0] + 3);
            pipeline.EX_rd = pipeline.rd;

            // find rs
            hoc_rs = GetRegValue_R(pipeline.instruction, space[1] + 2, space[1] + 3);
            pipeline.rs = pipeline.reg[hoc_rs];

            GetSE_R();

        }
        else if (job == 4) {//��beq(�P���e����k)
            //�]�����Oformat�Pr-format�ۦP�A�ҥH�i�����I�s
            // find rs
            hoc_rs = GetRegValue_R(pipeline.instruction, space[0] + 2, space[0] + 3);
            pipeline.rs = pipeline.reg[hoc_rs];

            // find rt
            hoc_rt = GetRegValue_R(pipeline.instruction, space[1] + 2, space[1] + 3);
            pipeline.rt = -pipeline.reg[hoc_rt];
            //�H�U�O�ڭ��B�~�Ω󰣿��çP�_forward�Pbranch
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
                if (pipeline.EX_Branch == 1 && (pipeline.rs + pipeline.rt == 0))//�nBRANCH
                {
                    // stop = 1;
                    pipeline.record = pipeline.sign_extend;
                    // line = line + sign_extend;//�b�o�̪����p��BRANCH���U�@�ӫ��O�nŪ�ĴX��A�N���ΦAIF�z�LPCsrc�����
                    // �]��Branch�P�_���|���@��INSCYCLEle��EX stall,�קK�A������Branch,�ҥH�M��Control Signal
                    pipeline.flag = 1;
                    cout << "branch" << endl;
                    pipeline.IS_beq = true;//��X�һ�
                    // Stall_Count = 2;
                }
            }

            //��signed extend(�n���h��)
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
* �bID���q:
* �ڭ̥D�n���ާ@�|��decode instruction�A�ҥH�ڭ̷|�N������control�Ƚ��
* ���۷|�w��stall�Bforwarding�i��P�_
* ���O�����M�D�D�n�ؼЬOforwarding�A���w��lw���»ݭnstall
* �]���ڭ̷|��o�̧P�_lw��stall�A�P��L���p(ex:stall�@����i��nforwarding)
* ��data hazrd���p�G����stall�A�N������forwarding�A�o��|²�檺�����@�U
* ��L�ԲӨB�J�ЬݥH�U
*/
void ID() {
    // stall�P�w,�åB�Ostall�������{�����|����
    if (pipeline.Stall_Count > 0) {//stall ��������X(��stall���ܤW�@�����O�|�b��X�@��)
        pipeline.STOP_ID = false;
        if (pipeline.INSCYCLE[1][0] != '0')//�Ŧr�šA��ܲ׵��e
        {
            fstream out;
            //�n�b�����~��gout.open("result.txt", ios::out | ios::app);����kios::out | ios::app
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
    //�H�U�O�p��Ů�(ins())�P�_�O������O
    for (int i = 0; i < pipeline.instruction.length(); i++) {
        if (pipeline.instruction[i] == ' ') {
            space[format] = i;
            format++;
        }
    }
    //�P�_���O
    ins();
    //�w�q��������(�i�H�ﴶ�q���ܼ�)
    int lw = 1, sw = 2, r_format = 3, beq = 4;

    if (job == lw) {//lw stall
        //*************************
        //16($0)
        // sign_extend->16/4=4
        // rs�s�쥻��m
        pipeline.rs = hoc_rs;
        //hoc_rs->���ثe��rs
        hoc_rs = hoc_rs + pipeline.sign_extend;//Memory�n�p��

        //���{�b���Ȧs�����S���M�e�@�����Ȧs���۵�
        //MEM_Dest�W�@�Ӫ�rd�A�M�ثe��rs�O�_�۵�(�۵��hstall->2���A���k�s�A�̫��٬O�|�Q���)
        if (pipeline.MEM_Dest == hoc_rs && pipeline.MEM_Reg_Write == 1) {
            pipeline.Stall_Count = 2;
            setEX(0, 0, 0, 0, 0, 0, 0);
        }
    }
    else if (job == sw) {//sw �J��lw�nstall
        // rs�s�쥻��m
        pipeline.rs = hoc_rs;
        //rt���������Ȧs�����έp��
        //MEM_Dest�W�@�Ӫ�rd�A�M�ثe��rt�O�_�۵�(�۵��hstall->2���A���k�s�A�̫��٬O�|�Q���)
        if (pipeline.MEM_Dest == hoc_rt && pipeline.MEM_Reg_Write == 1) {
            pipeline.sw_forwarding = 1;//�nforwarding(�����bex����)
            if (pipeline.MEM_Mem_Read == 1) {//�W�@�ӬO���Olw(lw��MemRead == 1)�O���ܭnstall
                if (pipeline.MEM_Dest == hoc_rs) {//�n�bhoc_rs(�ثe��rs�Ȧs������)
                    //����ڤW�bsw��Memory���������|�Q��ӵ����
                    pipeline.MEM_Forward_rs = 1;//Memory �� forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {//��ڧ쵹ex���ȥu��rt����
                    pipeline.MEM_Forward_rt = 1;
                }
                pipeline.Stall_Count = 1;//�n��stall
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {//�S��stall (�Dlw)
                if (pipeline.MEM_Dest == hoc_rs) {//��forwarding
                    //�n�bhoc_rs(�ثe��rs�Ȧs������)
                    //����ڤW�bsw��Memory���������|�Q��ӵ����
                    pipeline.EX_Forward_rs = 1;//EX �� forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {//��ڧ쵹ex���ȥu��rt����
                    pipeline.EX_Forward_rt = 1;
                }
            }
        }
    }
    else if (job == r_format) {//r-format stall
        cout << "MEM_Dest=" << pipeline.MEM_Dest << ", hoc_rs=" << hoc_rs << ", WB_rd=" << pipeline.WB_rd
            << ", hoc_rt" << hoc_rt << "\n";
        //�P�_�ثe��rs�Brt�|���O�e�@�Ӫ�rd&RegWrite == 1
        if ((pipeline.MEM_Dest == hoc_rs || pipeline.MEM_Dest == hoc_rt) && pipeline.MEM_Reg_Write == 1) {
            //forwarding = 1;
            if (pipeline.MEM_Mem_Read == 1) {//�P�_�O���Olw(lw�n��stall)
                if (pipeline.MEM_Dest == hoc_rs) {//Memory �� forwarding
                    pipeline.MEM_Forward_rs = 1;
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.MEM_Forward_rt = 1;
                }
                pipeline.Stall_Count = 1;//��stall
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {
                pipeline.forwarding = 1;//����stall��forwarding
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.EX_Forward_rs = 1;//EX �� forwarding
                    cout << "�ڬOrs\n";
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.EX_Forward_rt = 1;
                }
            }
        }
        //�P�_�e�e�����O(�@�˪�����)
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
        //�e�@����rd=rs��rt�BMEM_Reg_Write == 1
        if ((pipeline.MEM_Dest == hoc_rs || pipeline.MEM_Dest == hoc_rt) && pipeline.MEM_Reg_Write == 1) {
            if (pipeline.MEM_Mem_Read == 1) {//lw
                pipeline.Stall_Count = 2;//stall 2��
                pipeline.bforwarding = 1;//��branch forwarding
                pipeline.count_beq = 1;//�{�b�o�Ӧ^�Xbeq�n���X��
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.MEM_Forward_rs = 1;//Memory �� forwarding
                }
                if (pipeline.MEM_Dest == hoc_rt) {
                    pipeline.MEM_Forward_rt = 1;
                }
                setEX(0, 0, 0, 0, 0, 0, 0);
            }
            else {//stall 1��
                pipeline.Stall_Count = 1;//r-format
                pipeline.bforwarding = 1;//forwarding
                if (pipeline.MEM_Dest == hoc_rs) {
                    pipeline.bMEM_Forward_rs = 1;//Memory �� forwarding
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

    
    if (pipeline.IS_beq) {//�p�Gbeq�P�w�n����(���ӬO�bex�~�|���P�_���G�A���ӺC�F�A
        //�ҥH�ڭ̧��id���P�_�O�_��(�P�ҥ�����)�A��ex�~�����) �h���eIF���O�������ǤJID
        pipeline.INSCYCLE[1][0] = '0';
        pipeline.IS_beq = false;
        pipeline.Stall_Count = 1;
    }
    //�LID
    if (pipeline.INSCYCLE[1][1] == 'e' && pipeline.Stall_Count == 1)//�L�{���|�򥢭ȡA�j���^(beq)
    {
        pipeline.INSCYCLE[1][0] = 'b';
        fstream out;
        out.open("result.txt", ios::out | ios::app);
        out << "    " << pipeline.INSCYCLE[1] << ":ID" << endl;
        out.close();
    }
    else if (pipeline.INSCYCLE[1][0] != '0') {//���`�S�ܭ�
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

//ps.�H�U��Ȥ�2�N��X
//�]�w ex control table����
void setEX(int rd, int alusrc, int b, int mr, int mw, int rw, int mtr) {
    pipeline.EX_RegDst = rd;
    pipeline.EX_ALUSrc = alusrc;
    pipeline.EX_Branch = b;
    pipeline.EX_Mem_Read = mr,
        pipeline.EX_Mem_Write = mw;
    pipeline.EX_Reg_Write = rw;
    pipeline.EX_MemtoReg = mtr;
}
//c++�r����Ʀr���ѦҺ���
// https://shengyu7697.github.io/cpp-string-to-integer/
//�t�d����reg��reg�� I-format��Ȧs��
int GetRegValue_I(string instruction, int spaceIndex) {

    //3�M2�O�P�_�Ȧs���O�Q����٬O�Ӧ��(ex:16($2))
    // �P�_�Ȧs���O�Q����٬O�Ӧ��(ex:16($2))
    if (instruction[spaceIndex + 3] >= '0' && instruction[spaceIndex + 3] <= '9') {
        // rd��Ĥ@�ӭ�
        return ((int)instruction[spaceIndex + 3] - '0') + ((int)instruction[spaceIndex + 2] - '0') * 10;
    }
    else if (instruction[spaceIndex + 2] >= '0' && instruction[spaceIndex + 2] <= '9') {
        return ((int)instruction[spaceIndex + 2] - '0');
    }
    return -1;
}

//�t�d����reg��reg�� R-format��Ȧs��
int GetRegValue_R(const std::string& instruction, int digit1, int digit2) {
    if (instruction[digit2] >= '0' && instruction[digit2] <= '9') {
        return ((int)instruction[digit2] - '0') + ((int)instruction[digit1] - '0') * 10;
    }
    else if (instruction[digit1] >= '0' && instruction[digit1] <= '9') {
        return ((int)instruction[digit1] - '0');
    }
    return -1;
}
//�t�d����I-format��������첾
void GetSE_I() {
    //�]���ݭn��memory�����Ȧs�����P�_�A���ڭ̥D�n�O�ɤO�ѡA���L�̦h����ơA�n�O�o/4(word)
    if (space[1] + 1 == '-') {//���t����
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
    else {//�S�t����
        if (pipeline.instruction[space[1] + 2] >= '0' && pipeline.instruction[space[1] + 2] <= '9') {
            //�n���檺�첾/4
            pipeline.sign_extend = (((int)pipeline.instruction[space[1] + 2] - '0') + ((int)pipeline.instruction[space[1] + 1] - '0') * 10) / 4;
            if (pipeline.instruction[space[1] + 6] >= 48 && pipeline.instruction[space[1] + 6] <= '9') {
                //Memory �����Ȧs�����W�r
                hoc_rs = ((int)pipeline.instruction[space[1] + 6] - '0') + ((int)pipeline.instruction[space[1] + 5] - '0') * 10;
                //Memory �����Ȧs������ڪ���
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
//�t�d����R-format��rd�ð������t�P�_
void GetSE_R() {
    // find rt �������t add ����  sub ���t
    if (pipeline.instruction[0] == 'a') {
        if (pipeline.instruction[space[2] + 1] == '$') {
            if (pipeline.instruction[space[2] + 3] >= '0' && pipeline.instruction[space[2] + 3] <= '9') {
                //��ĤT��(�Ȧs�����W�r)
                hoc_rt = ((int)pipeline.instruction[space[2] + 3] - '0') + ((int)pipeline.instruction[space[2] + 2] - '0') * 10;
                //��ĤT��(�Ȧs������������)
                pipeline.rt = pipeline.reg[hoc_rt];
            }
            else if (pipeline.instruction[space[2] + 2] >= 48 && pipeline.instruction[space[2] + 2] <= '9') {
                hoc_rt = ((int)pipeline.instruction[space[2] + 2] - '0');
                pipeline.rt = pipeline.reg[hoc_rt];
            }
        }
        else {// find rt=�̫᪺�`��
            //�P�_add�̫�@�ӫ��O�O���O�`��
            int temp = 1;
            pipeline.rt = 0;
            if (space[2] + 1 == '-') {
                for (int i = pipeline.instruction.length() - 1; i > space[2] + 1; i--) {
                    //�s�������Ʀr hoc_rt�|�s��e�@�ӫ��O �ҥH�o�̳]��-1
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
    //�P�W��add(�o�̬O)sub
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
        else {// find rt=�̫᪺�`��
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