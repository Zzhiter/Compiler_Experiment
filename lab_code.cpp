#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <malloc.h>
using namespace std;
// 输入的字符序列临时变量
char temp_input[200];
// 输入的字符序列
char input_code[100];

// 全局指示参数
int symbol_num = 0;

// 存放读入的数字常数，临时变量
int temp_digit[100] = {0};
// 存放读入的数字常数
int digit_table[100] = {0};

// 临时变量，存放读入的token
char temp_token[100][10] = {'\0'};
// 存放读入的token
char token_table[100][10];

// 最后要输出的三地址代码
char inter_code[100][20]; 
// 记录要输出的三地址码的行数
int current_line = 0;

// 指向Value_table 和 token_table
int parse_pos = 0;

char w_keyword[][8] = {"if", "else", "while", "do", "int", "float"};

int row = 0;
// 记录分析中在第几行出错
void Show_Error()
{
    cout << "Error at row " << row << endl;
}

// 缓存要被分析的代码
void Get_Code()
{
    FILE *fp;
    if ((fp = fopen("read_buffer.txt", "wb")) == NULL)
    {
        cout << endl
             << "open file Fail,close!";
        getchar();
        exit(1);
    }
    char str[50] = "\0";
    cout << "Please input your codes each line with an enter\nInput '$' to exit" << endl;
    // 每次读入一行
    while (gets(str))
    {
        if (strcmp(str, "$") == 0)
            break;
        if (strcmp(str, "else") == 0)
        {
            fputs(str, fp);
            fputs(";", fp);
            continue;
        }
        fputs(str, fp);
    }
    fclose(fp);
}

// 数字到符号的映射
char Int2Sym(int a)
{
    switch (a)
    {
    case 0:
        return 'p'; // p = P'
    case 1:
        return 'P';
    case 2:
        return 'D';
    case 3:
        return 'S';
    case 4:
        return 'L';
    case 5:
        return 'C';
    case 6:
        return 'E';
    case 7:
        return 'F';
    case 8:
        return 'T';
    case 9:
        return 'a'; // a = id
    case 10:
        return ';';
    case 11:
        return 'b'; // b = int
    case 12:
        return 'c'; // c = float
    case 13:
        return '=';
    case 14:
        return '(';
    case 15:
        return ')';
    case 16:
        return '>';
    case 17:
        return '<';
    case 18:
        return 'h'; // h = '=='
    case 19:
        return '+';
    case 20:
        return '-';
    case 21:
        return '*';
    case 22:
        return '/';
    case 23:
        return 'd'; // d = digits
    case 24:
        return 'e'; // e = if
    case 25:
        return 'f'; // f = else
    case 26:
        return 'g'; // g = while
    case 27:
        return '$';
    case 28:
        return 'n'; // n = null
    }
    return ' ';
}

// 符号到数字的映射
int Sym2Int(char a)
{
    switch (a)
    {
    case 'p':
        return 0; // p = P'
    case 'P':
        return 1;
    case 'D':
        return 2;
    case 'S':
        return 3;
    case 'L':
        return 4;
    case 'C':
        return 5;
    case 'E':
        return 6;
    case 'F':
        return 7;
    case 'T':
        return 8;
    case 'a':
        return 9; // a = id
    case ';':
        return 10;
    case 'b':
        return 11; // b = int
    case 'c':
        return 12; // c = float
    case '=':
        return 13;
    case '(':
        return 14;
    case ')':
        return 15;
    case '>':
        return 16;
    case '<':
        return 17;
    case 'h':
        return 18; // h = '=='
    case '+':
        return 19;
    case '-':
        return 20;
    case '*':
        return 21;
    case '/':
        return 22;
    case 'd':
        return 23; // d = digits
    case 'e':
        return 24; // e = if
    case 'f':
        return 25; // f = else
    case 'g':
        return 26; // g = while
    case '$':
        return 27;
    }
}

// 把输入从临时变量转存到全局变量
void Init_Input()
{
    strcpy(input_code, temp_input);
    for (int i = 0; i < 100; i++)
    {
        digit_table[i] = temp_digit[i];
        strcpy(token_table[i], temp_token[i]);
        // cout << token_table[i] << endl;
        // cout << digit_table[i] << endl;
    }
}

// 把字符串in添加到全局三地址代码中
void Add_Intercode(char *in)
{
    strcpy(inter_code[current_line], in);
    current_line++;
}

// 在全局三地址码中删除n行
void Remove_inter_code(int n)
{
    current_line = current_line - n;
}

int tempVariable[1000]; // temp variable used to hold the
int used_temp_num = 0;  // 生成三地址码过程中使用的临时变量个数

// 各个符号对应的语义属性，包含终结符和非终结符
// 不同的符号对应的语义属性不同，不同的符号在这些
// 属性中进行选取，这里只是单纯全部列出来了
struct Sym_attr
{
    int sym_num;  // 符号对应的数字 
    int sym_type; // 类型，
    char name[10];
    // 一个语义动作可能生成很多代码
    char code[10][100]; // code
    char final_code[100];  // 如果是id或者dig的话，生成的就直接是最终代码了
    int codenum; // the number of code lines
    int value; 
    int value_pos; // 临时的value存放的位置
    int is_digit;  // 是否为digit
};
// 语义栈
struct Sym_attr SymStack[100]; 
// 语义栈栈顶指针
int sym_ptr = 0;

// 向语义栈中压入元素in
void Push_Sym_Stack(struct Sym_attr *in)
{
    SymStack[sym_ptr].sym_num = in->sym_num;
    SymStack[sym_ptr].sym_type = in->sym_type;
    SymStack[sym_ptr].value = in->value;
    SymStack[sym_ptr].is_digit = in->is_digit;
    // 依次复制
    for (int i = 0; i < in->codenum; i++)
    {
        strcpy(SymStack[sym_ptr].code[i], in->code[i]);
    }
    strcpy(SymStack[sym_ptr].final_code, in->final_code);
    strcpy(SymStack[sym_ptr].name, in->name);
    SymStack[sym_ptr].codenum = in->codenum;
    SymStack[sym_ptr].value_pos = in->value_pos;
    sym_ptr++;
}

// 从语义栈中弹出num个元素
void Pop_Sym_Stack(int num)
{
    sym_ptr = sym_ptr - num;
}

// 符号表中的项
struct Symbol
{   
    // 变量的类型，11为int，12为float
    int sym_type;
    // 变量的名字
    char name[10];
    // 变量的值
    int value;
    int has_init; // 1为已被初始化，0表示还未被初始化
};

// 符号表
struct Symbol Symtable[100]; // symbol table
int sym_number = 0;

// 在符号表给in，添加信息
void CreateSym(struct Symbol *in, int type, char *name, int value, int has_init)
{
    in->sym_type = type;
    in->value = value;
    strcpy(in->name, name);
    in->has_init = has_init;
}

// 在符号表中查找符号name，并修改其value值为value，成功则返回0，否则返回1
int ChangeSym(char *name, int value)
{
    // 顺序遍历符号表，找到目标name之后
    for (int i = 0; i <= sym_number; i++)
    {
        if (strcmp(name, Symtable[i].name) == 0)
        {
            Symtable[i].value = value;
            Symtable[i].has_init = 1;
            return 0;
        }
    }
    return 1;
}

// 在符号表中添加一个符号项in，如果重复定义，返回1，否则添加，返回0
int AddSym(struct Symbol in)
{
    for (int i = 0; i <= sym_number; i++)
    {
        if (strcmp(in.name, Symtable[i].name) == 0)
        {
            return 1; // error
        }
    }
    Symtable[sym_number].sym_type = in.sym_type;
    strcpy(Symtable[sym_number].name, in.name);
    Symtable[sym_number].value = in.value;
    Symtable[sym_number].has_init = in.has_init;
    sym_number++;
    return 0;
}

// 使用第num个产生式进行规约，执行对应的语义动作
void Reduce_Symbol(int num)
{
    struct Symbol temp; // Symbol be initialized
    int err = AddSym(temp);
    int L1;

    // 每次reduce规约的时候，先把规约的产生式左部先生成出来备用

    // D：定义语句
    struct Sym_attr D;
    D.codenum = 0;
    D.sym_num = 2;
    // S：statement 声明语句
    struct Sym_attr S;
    S.codenum = 0;
    S.sym_num = 3;
    // Expression：表达式
    struct Sym_attr E;
    E.codenum = 0;
    E.sym_num = 6;
    // Terminal：
    struct Sym_attr T;
    T.codenum = 0;
    T.sym_num = 8;
    // Factor：因子
    struct Sym_attr F;
    F.codenum = 0;
    F.sym_num = 7;
    // L：类型，int or float
    struct Sym_attr L;
    L.codenum = 0;
    L.sym_num = 4;
    // 判断式：E > E 或者其他
    struct Sym_attr C;
    C.codenum = 0;
    C.sym_num = 5;
    // Program
    struct Sym_attr P;
    P.codenum = 0;  
    P.sym_num = 1;

    int err_id;
    char code[100];
    char final_code[100];

    char Dcode[100]; // 存放生成的临时的中间代码 

    // 对应不同的产生式规约
    switch (num)
    {
    // D L id;  往符号表中添加一个标识符
    case 23:
        // 表示未被初始化，目前仅仅只是定义了
        temp.has_init = 0;
        // temp.name = id
        strcpy(temp.name, SymStack[sym_ptr - 2].name);
        // sym_ptr - 3是对应的类型L，到底是int还是float
        // temp.sym_type = L.sym_type
        temp.sym_type = SymStack[sym_ptr - 3].sym_type;
        // 添加到符号表
        err_id = AddSym(temp);
        // 如果添加失败，表示重复定义了
        if (err_id == 1)
        {
            cout << "error:repeated definition" << endl;
        }

        // 判断是L的类型，是int还是float
        switch (SymStack[sym_ptr - 3].sym_type)
        {
        case 11:
            strcpy(code, "int");
            break;
        case 12:
            strcpy(code, "float");
            break;
        }

        // 把格式化的数据写入Dcode字符串
        sprintf(Dcode, "%s %s;", code, SymStack[sym_ptr - 2].name);
        strcpy(D.code[0], Dcode);
        // D L id; 所以弹出三个
        Pop_Sym_Stack(3);
        // 对应的语义信息入栈
        Push_Sym_Stack(&D);
        break;
    //  D L id;D  连续定义语句
    case 2:
        temp.has_init = 0;
        strcpy(temp.name, SymStack[sym_ptr - 3].name);
        temp.sym_type = SymStack[sym_ptr - 4].sym_type;
        err = AddSym(temp);
        if (err == 1)
        {
            cout << "error:repeated definition" << endl;
        }

        switch (SymStack[sym_ptr - 4].sym_type)
        {
        case 11:
            strcpy(code, "int");
            break;
        case 12:
            strcpy(code, "float");
            break;
        }
        sprintf(Dcode, "%s %s;", code, SymStack[sym_ptr - 3].name);
        strcpy(D.code[0], Dcode);
        Pop_Sym_Stack(4);
        Push_Sym_Stack(&D);
        break;
    // P DS
    case 1:
        // DS出栈，所以是2
        Pop_Sym_Stack(2);
        P.sym_num = 1;
        // P入栈
        Push_Sym_Stack(&P);
        break;
    // P S
    case 3:
        // S出栈，所以是1
        Pop_Sym_Stack(1);
        P.sym_num = 1;
        Push_Sym_Stack(&P);
        break;
    // L int
    case 4:
        // L.sym_type = int.sym_num
        L.sym_type = SymStack[sym_ptr - 1].sym_num;
        L.sym_num = 4;
        // int出栈
        Pop_Sym_Stack(1);
        // L进栈
        Push_Sym_Stack(&L);
        break;
    // L float
    case 5:
        // L.sym_type = float.sym_num
        L.sym_type = SymStack[sym_ptr - 1].sym_num;
        L.sym_num = 4;
        // float出栈
        Pop_Sym_Stack(1);
        // L进栈
        Push_Sym_Stack(&L);
        break;
    // S id=E;  赋值语句  b=E;  所以出栈四个
    case 6:
        // 维护S的sym_num 因为是S，所以是3
        S.sym_num = 3;
        // 先把E的代码全部给S
        for (int i = 0; i < SymStack[sym_ptr - 2].codenum; i++)
        {
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 2].code[i]);
            S.codenum++;
        }

        // 如果E的类型是23：digits
        if (SymStack[sym_ptr - 2].sym_type == 23)
        {
            // 生成代码id = digits
            sprintf(Dcode, "%s=%d;", SymStack[sym_ptr - 4].name, SymStack[sym_ptr - 2].value);
        }
        // 如果E的类型是id
        else if (SymStack[sym_ptr - 2].sym_type == 9)
        {
            // 生成代码id = id
            sprintf(Dcode, "%s=%s;", SymStack[sym_ptr - 4].name, SymStack[sym_ptr - 2].name);
        }
        // 否则，把E的地址给id，SymStack[sym_ptr - 2].value_pos是存放E的数值的地址
        else
        {
            sprintf(Dcode, "%s=Reg%d;", SymStack[sym_ptr - 4].name, SymStack[sym_ptr - 2].value_pos);
        }

        // 维护S的代码，再添加一行
        strcpy(S.code[S.codenum], Dcode);
        S.codenum++;

        // 维护全局的三地址代码
        Add_Intercode(Dcode);
        // 在符号表中修改S的值为E.value_pos
        err = ChangeSym(SymStack[sym_ptr - 4].name, SymStack[sym_ptr - 2].value_pos);
        if (err)
        {
            cout << "error:identifier used before identified" << endl;
        }
        // b=E; 这四个出栈
        Pop_Sym_Stack(4);
        // S进栈
        Push_Sym_Stack(&S);
        break;
    // S if(C) S    S e(C)S
    case 7:
        // 先把C的代码给S
        S.codenum = SymStack[sym_ptr - 3].codenum;
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            strcpy(S.code[i], SymStack[sym_ptr - 3].code[i]);
        }
        // 先把产生式右部S的代码从全局删除
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        // if C goto current_line + 2，  +2 是因为跳过了当前的if语句
        sprintf(Dcode, "if %s goto %d;", SymStack[sym_ptr - 3].final_code, current_line + 2);
        Add_Intercode(Dcode);
        strcpy(S.code[S.codenum], Dcode);
        S.codenum++;
        S.sym_num = 3;
        // 对应if失败的情况，直接跳过if后边的S
        sprintf(Dcode, "goto %d;", current_line + SymStack[sym_ptr - 1].codenum + 1);
        Add_Intercode(Dcode);
        strcpy(S.code[S.codenum], Dcode);
        S.codenum++;

        // 把上边所有代码加到产生式左部的S
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 1].code[i]);
            S.codenum++;
        }
        // e(C)S出栈
        Pop_Sym_Stack(5);
        // 产生式左部S进栈
        Push_Sym_Stack(&S);
        break;

    // S if(C) S else S    S0  e(C)S1fS2
    case 8:
        // 把C的代码复制给产生式左部的S
        strcpy(S.code[0], Dcode);
        S.codenum = SymStack[sym_ptr - 5].codenum;
        for (int i = 0; i < SymStack[sym_ptr - 5].codenum + 1; i++)
        {
            strcpy(S.code[i], SymStack[sym_ptr - 5].code[i]);
        }

        // 删除S2的代码
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);

        // 在全局删除S1的代码
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);

        // 在全局删除C的代码
        Remove_inter_code(SymStack[sym_ptr - 5].codenum);
    
        // if C goto S1
        sprintf(Dcode, "if %s goto %d;", SymStack[sym_ptr - 5].final_code, current_line + SymStack[sym_ptr - 1].codenum + 2);
        Add_Intercode(Dcode);
        strcpy(S.code[S.codenum], Dcode);
        S.codenum++;
        S.sym_num = 3;

        // 再把S2代码添加进来
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 1].code[i]);
            S.codenum++;
        }
        // goto current_line+S1.codenum+1
        sprintf(Dcode, "goto %d;", current_line + SymStack[sym_ptr - 3].codenum + 1);
        Add_Intercode(Dcode);
        strcpy(S.code[S.codenum], Dcode);
        S.codenum++;

        // 再把S1代码添加进来
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 3].code[i]);
            S.codenum++;
        }
        // 这7个出栈 e(C)SfS
        Pop_Sym_Stack(7);
        Push_Sym_Stack(&S);
        break;
    // S SS   S S1S2
    case 10:
        S.codenum = 0;
        // 先把S2代码复制过来
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 3].code[i]);
            S.codenum++;
        }
        // 再把S1代码复制过来
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            strcpy(S.code[S.codenum], SymStack[sym_ptr - 1].code[i]);
            S.codenum++;
        }
        // S1 和 S2
        Pop_Sym_Stack(2);

        // 这是原来的，出栈3个，可是我觉得是2个
        // Pop_Sym_Stack(3);
        Push_Sym_Stack(&S);
        break;
    // C E>E  C E1 > E2
    case 11:
        // 在全局中先删除E2,再删除E1
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        C.codenum = 0;

        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 3].code[i]);
            C.codenum++;
        }
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 1].code[i]);
            C.codenum++;
        }
        // 如果E1是id
        if (SymStack[sym_ptr - 3].sym_type == 9)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%s>%s", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%s>%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%s>Reg%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        // 如果E1是digits
        else if (SymStack[sym_ptr - 3].sym_type == 23)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%d>%s", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%d>%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%d>Reg%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        else
        {   
            // 如果E2是id
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d>%s", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            // 如果E2是digits
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d>%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            // 这俩都不是id或dig
            else
            {
                sprintf(Dcode, "Reg%d>Reg%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }

        // E>E 出栈
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;
    // C E<E  和上面类似，只需要改变比较符号就行
    case 12:
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        C.codenum = 0;
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 3].code[i]);
            C.codenum++;
        }
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 1].code[i]);
            C.codenum++;
        }
        if (SymStack[sym_ptr - 3].sym_type == 9)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%s<%s", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%s<%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%s<Reg%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        else if (SymStack[sym_ptr - 3].sym_type == 23)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%d<%s", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%d<%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%d<Reg%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d<%s", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d<%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "Reg%d<Reg%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;
    // C E==E
    case 13:
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        C.codenum = 0;
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 3].code[i]);
            C.codenum++;
        }
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(C.code[C.codenum], SymStack[sym_ptr - 1].code[i]);
            C.codenum++;
        }
        if (SymStack[sym_ptr - 3].sym_type == 9)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%s==%s", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%s==%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%s==Reg%d", SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        else if (SymStack[sym_ptr - 3].sym_type == 23)
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "%d==%s", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "%d==%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "%d==Reg%d", SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 1].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d==%s", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
                strcpy(C.final_code, Dcode);
            }
            else if (SymStack[sym_ptr - 1].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d==%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
                strcpy(C.final_code, Dcode);
            }
            else
            {
                sprintf(Dcode, "Reg%d==Reg%d", SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
                strcpy(C.final_code, Dcode);
            }
        }
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;

    //  
    case 14:
        // 先删除E和T的代码，在全局
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        E.codenum = 0;

        // 为左部的E添加右部的E的代码
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(E.code[E.codenum], SymStack[sym_ptr - 3].code[i]);
            E.codenum++;
        }

        // 为E添加T的代码
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(E.code[E.codenum], SymStack[sym_ptr - 1].code[i]);
            E.codenum++;
        }

        // 如果T是dig
        if (SymStack[sym_ptr - 1].sym_type == 23)
        {
            // E也是dig
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d+%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
            }
            // E是id
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s+%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
            }
            // E是变量
            else
            {
                sprintf(Dcode, "Reg%d=Reg%d+%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
            }
        }
        // 如果T是id
        else if (SymStack[sym_ptr - 1].sym_type == 9)
        {
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d+%s", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s+%s", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
            }
            else
            {
                sprintf(Dcode, "Reg%d=Reg%d+%s", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d+Reg%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s+Reg%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
            }
            else
            {
                sprintf(Dcode, "Reg%d=Reg%d+Reg%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
            }
        }
        // 都是要使用临时变量的
        E.value_pos = used_temp_num;
        used_temp_num++;

        Add_Intercode(Dcode);
        strcpy(E.code[E.codenum], Dcode);
        E.codenum++;
        E.sym_type = 6;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&E);
        break;
    // E E-T   和上面类似
    case 15:
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        E.codenum = 0;
    
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(E.code[E.codenum], SymStack[sym_ptr - 3].code[i]);
            E.codenum++;
        }
        
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(E.code[E.codenum], SymStack[sym_ptr - 1].code[i]);
            E.codenum++;
        }

        E.value_pos = used_temp_num;
        used_temp_num++;
        if (SymStack[sym_ptr - 1].sym_type == 23)
        {
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d-%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s-%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d-%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
            }
        }
        else if (SymStack[sym_ptr - 1].sym_type == 9)
        {
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d-%s", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s-%s", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
            }
            else
            {
                sprintf(Dcode, "Reg%d=Reg%d-%s", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 3].sym_type == 23)
            {
                sprintf(Dcode, "Reg%d=%d-Reg%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s-Reg%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
            }
            else
            {
                sprintf(Dcode, "Reg%d=Reg%d-Reg%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
            }
        }
        E.value_pos = used_temp_num;
        used_temp_num++;
        Add_Intercode(Dcode);
        strcpy(E.code[E.codenum], Dcode);
        E.codenum++;
        E.sym_type = 6;

        Pop_Sym_Stack(3);
        Push_Sym_Stack(&E);
        break;
    // E T
    case 16:
        // 直接把T的属性复制给E
        E.codenum = 0;
        E.is_digit = SymStack[sym_ptr - 1].is_digit;
        E.value = SymStack[sym_ptr - 1].value;
        E.sym_type = SymStack[sym_ptr - 1].sym_type;
        strcpy(E.name, SymStack[sym_ptr - 1].name);
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            strcpy(E.code[E.codenum], SymStack[sym_ptr - 1].code[i]);
            E.codenum++;
        }
        E.value_pos = SymStack[sym_ptr - 1].value_pos;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&E);
        break;
    // T F
    case 17:
        // 直接把F的属性复制给T
        T.codenum = 0;
        T.is_digit = SymStack[sym_ptr - 1].is_digit;
        T.value = SymStack[sym_ptr - 1].value;
        T.sym_type = SymStack[sym_ptr - 1].sym_type;
        strcpy(T.name, SymStack[sym_ptr - 1].name);
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            strcpy(T.code[T.codenum], SymStack[sym_ptr - 1].code[i]);
            T.codenum++;
        }
        T.value_pos = SymStack[sym_ptr - 1].value_pos;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&T);
        break;
    // T T*F
    case 18:
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        T.codenum = 0;
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(T.code[T.codenum], SymStack[sym_ptr - 3].code[i]);
            T.codenum++;
        }
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(T.code[T.codenum], SymStack[sym_ptr - 1].code[i]);
            T.codenum++;
        }
        T.value_pos = used_temp_num;
        used_temp_num++;
        if (SymStack[sym_ptr - 1].sym_type == 7)
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d*Reg%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s*Reg%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d*Reg%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
            }
        }
        else if (SymStack[sym_ptr - 1].sym_type == 9)
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d*%s", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s*%s", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d*%s", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d*%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s*%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d*%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
            }
        }
        Add_Intercode(Dcode);
        strcpy(T.code[T.codenum], Dcode);
        T.codenum++;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&T);
        break;
    // T T/F 
    case 19:
        Remove_inter_code(SymStack[sym_ptr - 1].codenum);
        Remove_inter_code(SymStack[sym_ptr - 3].codenum);
        T.codenum = 0;
        for (int i = 0; i < SymStack[sym_ptr - 3].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 3].code[i]);
            strcpy(T.code[T.codenum], SymStack[sym_ptr - 3].code[i]);
            T.codenum++;
        }
        for (int i = 0; i < SymStack[sym_ptr - 1].codenum; i++)
        {
            Add_Intercode(SymStack[sym_ptr - 1].code[i]);
            strcpy(T.code[T.codenum], SymStack[sym_ptr - 1].code[i]);
            T.codenum++;
        }
        T.value_pos = used_temp_num;
        used_temp_num++;
        if (SymStack[sym_ptr - 1].sym_type == 7)
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d+Reg%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value_pos);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s+Reg%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value_pos);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d+Reg%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value_pos);
            }
        }
        else if (SymStack[sym_ptr - 1].sym_type == 9)
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d/%s", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].name);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s/%s", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].name);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d/%s", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].name);
            }
        }
        else
        {
            if (SymStack[sym_ptr - 3].sym_type == 7)
            {
                sprintf(Dcode, "Reg%d=Reg%d/%d", used_temp_num, SymStack[sym_ptr - 3].value_pos, SymStack[sym_ptr - 1].value);
            }
            else if (SymStack[sym_ptr - 3].sym_type == 9)
            {
                sprintf(Dcode, "Reg%d=%s/%d", used_temp_num, SymStack[sym_ptr - 3].name, SymStack[sym_ptr - 1].value);
            }
            else
            {
                sprintf(Dcode, "Reg%d=%d/%d", used_temp_num, SymStack[sym_ptr - 3].value, SymStack[sym_ptr - 1].value);
            }
        }
        Add_Intercode(Dcode);
        strcpy(T.code[T.codenum], Dcode);
        T.codenum++;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&T);
        break;

    // F (E)
    case 20:
        // 直接复制E的属性给F
        F.codenum = 0;
        F.sym_type = 7;
        F.value = SymStack[sym_ptr - 2].value;
        F.is_digit = SymStack[sym_ptr - 2].is_digit;
        for (int i = 0; i < SymStack[sym_ptr - 2].codenum; i++)
        {
            strcpy(F.code[F.codenum], SymStack[sym_ptr - 2].code[i]);
            F.codenum++;
        }
        F.value_pos = SymStack[sym_ptr - 2].value_pos;
        // 出栈3个元素
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&F);
        break;
    // F id
    case 21:
        // 直接复制给F
        F.is_digit = 0;
        F.sym_type = 9;
        F.value = SymStack[sym_ptr - 1].value;
        strcpy(F.name, SymStack[sym_ptr - 1].name);
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&F);
        break;
    // F digits
    case 22:
        // 直接复制给F
        F.is_digit = 1;
        F.sym_type = 23;
        F.value = SymStack[sym_ptr - 1].value;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&F);
        break;
    }
}

// 向语义栈添加id并初始化其属性
void Insert_Symbol_id(char *name)
{
    struct Sym_attr A;
    A.sym_num = 9; // 9是id
    A.codenum = 0; 
    strcpy(A.name, name);
    // A入语义栈
    Push_Sym_Stack(&A);
}

// 向符号栈添加常数digis并初始化其属性
void Insert_Symbol_digits(int digits)
{
    struct Sym_attr A;
    A.sym_num = 23; // 23是digits
    A.codenum = 0;
    A.value = digits;
    Push_Sym_Stack(&A);
}

// 向语义栈单纯添加一个非 id 非 常数的东西
void Insert_Symbol(int n)
{
    struct Sym_attr A;
    A.codenum = 0;
    Push_Sym_Stack(&A);
}

// 产生式的右部
char start[10] = "P";
char str1[10] = "DS";
char str2[10] = "La;D";
char str3[10] = "S";
char str4[10] = "b";
char str5[10] = "c";
char str6[10] = "a=E;";
char str7[10] = "e(C)S"; //e 是 if
char str8[10] = "e(C)SfS"; // f 是 else
char str9[10] = "g(C)S";
char str10[10] = "SS";
char str11[10] = "E>E";
char str12[10] = "E<E";
char str13[10] = "EhE"; // h是==
char str14[10] = "E+T";
char str15[10] = "E-T";
char str16[10] = "T";
char str17[10] = "F";
char str18[10] = "T*F";
char str19[10] = "T/F";
char str20[10] = "(E)";
char str21[10] = "a";   // a是标识符id
char str22[10] = "d";   // d是digits 数字常数
char str23[10] = "La;";

int First[9][20];  // First Set 18 is '$'    19 is 'null'
int Follow[9][20]; // 18 is '$'

// 生成式项，为了方便，使用映射后的数字表示产生式左部
struct Item
{
    // 产生式左部
    int head; //头编号
    // 产生式右部
    char body[10]; //体
    // 产生式右部长度
    int len; //长度
};

struct Item I[24]; // 23个生成式项

//一个状态的项集
struct ISet
{
    // 23个产生式，每个产生式最长为9
    int SetItem[24][10];
    int validItem[24];
    int conf; //存在冲突
};

struct ISet Status[200]; //所有状态
int status_num = 0;      //状态数目

struct Action
{   
    // 出错则为0， 正常则为1， 接受acc则为2
    int status_code; 
    int movein;      //为1表示移入
    int movenum;     //表示移入的状态
    // 表示需要用0-24中哪个产生式进行规约
    int reduce;    //为-1表示不规约
    int reducelen; //回退长度
};
// 最多200个状态，28个符号
int GOTO[200][28]; // GOTO表，描述状态转移
// Action表
struct Action ACT[200][28];

// 状态栈，包含栈本身和栈顶指针
struct StatusStack
{
    int StatStack[200];
    int topptr;
};

// 状态栈
struct StatusStack SStack;
// 返回状态栈的栈顶状态
int GetStackTop()
{
    return SStack.StatStack[SStack.topptr - 1];
}

// 从状态栈的栈顶弹出n个元素
int PopStack(int n)
{
    int a = SStack.topptr;
    SStack.topptr = SStack.topptr - n;
}

// 状态n入栈
void PushStack(int n)
{
    int a = SStack.topptr;
    SStack.StatStack[SStack.topptr] = n;
    SStack.topptr = SStack.topptr + 1;
}



int SetEqual(struct ISet *in1, struct ISet *in2)
{
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (((in1->SetItem[i][j] != in2->SetItem[i][j]) &&
                 (in1->validItem[i] == 1)) ||
                (in1->validItem[i] != in2->validItem[i]))
            {
                return 0; // 0表示不相等
            }
        }
    }
    return 1; // 1表示相等
}

void SetCopy(struct ISet *in1, struct ISet *in2)
{
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            in1->SetItem[i][j] = in2->SetItem[i][j];
            in1->validItem[i] = in2->validItem[i];
        }
    }
    in1->conf = in2->conf;
}

// 计算
void CLOSURE(struct ISet *in)
{
    // num记录in下有多少个产生式
    int num = 0;
    for (int i = 0; i < 24; i++)
    {
        if (in->validItem[i] == 1)
            num++;
    }
    int newnum;
    do
    {
        newnum = num;
        for (int i = 0; i < 24; i++)
        {
            // validItem为1表示，存在第i个产生式
            if (in->validItem[i] == 1)
            { //存在该项
                for (int k = 0; k < 10; k++)
                {
                    //
                    if ((k < I[i].len) && (in->SetItem[i][k] == 1))
                    {
                        char c1 = I[i].body[k];
                        for (int j = 0; j < 24; j++)
                        {
                            //存在产生式头部为非终极符号
                            int headnum = I[j].head;
                            char c2 = Int2Sym(headnum);
                            if (c2 == c1)
                            {
                                if ((in->validItem[j] == 0) || (in->SetItem[j][0] == 0))
                                {
                                    newnum++;
                                    in->validItem[j] = 1;
                                    in->SetItem[j][0] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // 如果没有新的产生式加进来，newnum就会等于num，循环就会退出
    while (num != newnum);
}

//
int SetValid(struct ISet *in)
{
    int a = 0;
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < 10; j++)
            if ((in->validItem[i] == 1) && (in->SetItem[i][j] == 1))
            {
                a = 1;
            }
    }
    return a;
}

//若in在j项有效，返回1
int ItemValid(struct ISet *in, int j)
{
    int a = 0;
    for (int i = 0; i <= I[j].len; i++)
    {
        a = a + in->SetItem[j][i];
    }
    if (a == 0)
    {
        return 0;
    }
    return 1;
}

void Init_LR()
{ //初始化
    memset(GOTO, -1, sizeof(GOTO));
    memset(First, 0, sizeof(First));
    memset(Follow, 0, sizeof(Follow));

    memset(SStack.StatStack, 0, sizeof(SStack.StatStack));
    SStack.topptr = 0;
    PushStack(0);

    // init Action table
    for (int i = 0; i < 200; i++)
    {
        for (int j = 0; j < 28; j++)
        {
            ACT[i][j].status_code = 0;
            ACT[i][j].movein = 0;
            ACT[i][j].movenum = 0;
            ACT[i][j].reduce = -1;
            ACT[i][j].reducelen = 0;
        }
    }

    First[2][19] = 1;
    Follow[0][18] = 1;

    // 添加产生式
    // P
    I[0].head = 0;
    strcpy(I[0].body, start);
    I[0].len = strlen(start);

    // str1为DS
    I[1].head = 1;
    strcpy(I[1].body, str1);
    I[1].len = strlen(str1);
    //
    I[2].head = 2;
    strcpy(I[2].body, str2);
    I[2].len = strlen(str2);

    I[3].head = 1;
    strcpy(I[3].body, str3);
    I[3].len = strlen(str3);

    I[4].head = 4;
    strcpy(I[4].body, str4);
    I[4].len = strlen(str4);

    I[5].head = 4;
    strcpy(I[5].body, str5);
    I[5].len = strlen(str5);

    I[6].head = 3;
    strcpy(I[6].body, str6);
    I[6].len = strlen(str6);

    I[7].head = 3;
    strcpy(I[7].body, str7);
    I[7].len = strlen(str7);

    I[8].head = 3;
    strcpy(I[8].body, str8);
    I[8].len = strlen(str8);

    I[9].head = 3;
    strcpy(I[9].body, str9);
    I[9].len = strlen(str9);

    I[10].head = 3;
    strcpy(I[10].body, str10);
    I[10].len = strlen(str10);

    I[11].head = 5;
    strcpy(I[11].body, str11);
    I[11].len = strlen(str11);

    I[12].head = 5;
    strcpy(I[12].body, str12);
    I[12].len = strlen(str12);

    I[13].head = 5;
    strcpy(I[13].body, str13);
    I[13].len = strlen(str13);

    I[14].head = 6;
    strcpy(I[14].body, str14);
    I[14].len = strlen(str14);

    I[15].head = 6;
    strcpy(I[15].body, str15);
    I[15].len = strlen(str15);

    I[16].head = 6;
    strcpy(I[16].body, str16);
    I[16].len = strlen(str16);

    I[17].head = 8;
    strcpy(I[17].body, str17);
    I[17].len = strlen(str17);

    I[18].head = 8;
    strcpy(I[18].body, str18);
    I[18].len = strlen(str18);

    I[19].head = 8;
    strcpy(I[19].body, str19);
    I[19].len = strlen(str19);

    I[20].head = 7;
    strcpy(I[20].body, str20);
    I[20].len = strlen(str20);

    I[21].head = 7;
    strcpy(I[21].body, str21);
    I[21].len = strlen(str21);

    I[22].head = 7;
    strcpy(I[22].body, str22);
    I[22].len = strlen(str22);
    // D -> La;
    I[23].head = 2;
    strcpy(I[23].body, str23);
    I[23].len = strlen(str23);

    // 初始化所有状态
    for (int i = 0; i < 200; i++)
    {
        for (int j = 0; j < 24; j++)
        {
            Status[i].validItem[j] = 0;
            for (int k = 0; k < 10; k++)
            {
                Status[i].SetItem[j][k] = 0;
            }
        }
    }

    Status[0].validItem[0] = 1;
    Status[0].SetItem[0][0] = 1;
}

void MergeFirst(int i, int j)
{ // merge j to i
    for (int k = 0; k < 19; k++)
    {
        if (First[j][k] == 1)
        {
            First[i][k] = 1;
        }
    }
}

void MergeFollow(int i, int j)
{ // merge j to i
    for (int k = 0; k < 19; k++)
    {
        if (Follow[j][k] == 1)
        {
            Follow[i][k] = 1;
        }
    }
}

void MergeFollowFirst(int i, int j)
{ // merge j to i
    for (int k = 0; k < 19; k++)
    {
        if (First[j][k] == 1)
        {
            Follow[i][k] = 1;
        }
    }
}

int CalFirst()
{
    int num = 0;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (First[i][j] == 1)
            {
                num++;
            }
        }
    }
    return num;
}

int CalFollow()
{
    int num = 0;
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (Follow[i][j] == 1)
            {
                num++;
            }
        }
    }
    return num;
}

void GetFirst()
{
    for (int i = 0; i < 24; i++)
    {
        char c = I[i].body[0];
        int number = Sym2Int(c);
        if (number > 8)
        {
            First[I[i].head][number - 9] = 1;
        }
    }
    int oldnum = CalFirst();
    int truenum = oldnum;

    do
    {
        oldnum = truenum;
        for (int i = 0; i < 24; i++)
        {
            for (int j = 0; j < I[i].len; j++)
            {
                char c = I[i].body[j];
                int number = Sym2Int(c);
                if (number > 8)
                {
                    First[I[i].head][number - 9] = 1;
                    break;
                }
                else
                {
                    MergeFirst(I[i].head, number);
                    if (First[number][19] != 1)
                    {
                        break;
                    }
                }
                if (j == I[i].len - 1)
                {
                    if (First[j][19] == 1)
                    {
                        First[I[i].head][19] = 1;
                    }
                }
            }
        }
        truenum = CalFirst();
    } while (truenum != oldnum);
}

void GetFollow()
{
    for (int i = 0; i < 24; i++)
    {
        for (int j = 0; j < I[i].len - 1; j++)
        {
            char c1 = I[i].body[j];
            char c2 = I[i].body[j + 1];
            int number1 = Sym2Int(c1);
            int number2 = Sym2Int(c2);
            if ((number1 < 9) && (number2 > 8))
            {
                Follow[number1][number2 - 9] = 1;
            }
        }
    }
    int oldnum = CalFollow();
    int truenum = oldnum;

    do
    {
        oldnum = truenum;
        for (int i = 0; i < 24; i++)
        {
            for (int j = 0; j < I[i].len - 1; j++)
            {
                char c1 = I[i].body[j];
                char c2 = I[i].body[j + 1];
                int number1 = Sym2Int(c1);
                int number2 = Sym2Int(c2);
                if ((number1 < 9) && (number2 < 9))
                {
                    MergeFollowFirst(number1, number2);
                    for (int k = number2; k < I[i].len - 1; k++)
                    {
                        if (First[k][19] == 1)
                        {
                            MergeFollowFirst(number1, k + 1);
                            if (k == I[i].len - 2)
                            {
                                if (First[k + 1][19] == 1)
                                {
                                    MergeFollow(number1, I[i].head);
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            char c3 = I[i].body[I[i].len - 1];
            int num = Sym2Int(c3);
            if (num < 9)
            {
                MergeFollow(num, I[i].head);
            }
        }
        truenum = CalFollow();
    } while (truenum != oldnum);
}

int main()
{
    cout << "Please input 1, 2 or 3\n1 for Lexical analysis\n2 for Syntax analysis\n3 for Semantic Analysis\n ";
    int choose;
    cin >> choose;

    if(choose != 1 && choose != 2 && choose != 3)
    {
        cout << "input error" <<endl;
        return 0;
    }
     
    int after_else = 0;
    Get_Code();

    char sentance_input[200];
    char word_token[20];
    // 下一个字符
    char next_c;
    int w_forward = 0, w_next = 0;
    int w_state = 0;
    int x_num = 0, id_x = 0;

    FILE *fp_soure;

    fp_soure = fopen("read_buffer.txt", "r+");

    if (fp_soure != NULL)
    {
        memset(sentance_input, 0x00, sizeof(char) * 200);
        cout << "Token generated:" << endl;

        // reading all lines
        while (EOF != fscanf(fp_soure, "%[^\n]\n", sentance_input))
        { //%*c

            row++;
            w_next = 0;
            w_forward = 0;
            w_state = 0;
            x_num = 0;

            while (1)
            {
                switch (w_state)
                {
                // 初始状态是0，从这开始处理
                case 0:
                    // 删除每行前边的空格
                    while ((next_c = sentance_input[w_next]) == ' ' || next_c == '\t')
                    { // || next_c=='\t'
                        w_next++;
                        w_forward++;
                    }
                    // 临时变量
                    memset(word_token, 0x00, sizeof(char) * 20);
                    x_num = 0;

                    // processing the digits
                    if (isdigit(next_c))
                    {
                        while ((next_c != ' ') && (isdigit(next_c) || next_c == '.'))
                        { 
                            w_next++;
                            next_c = sentance_input[w_next];
                        }

                        if (next_c == ' ' || !(isalpha(next_c)))
                            w_next--;
                        if (isalpha(next_c))
                        {
                            Show_Error();
                        }

                        x_num = 0;
                        while (w_forward <= w_next)
                        {
                            word_token[x_num] = sentance_input[w_forward];
                            w_forward++;
                            x_num++;
                        }
                        word_token[w_forward + 1] = '\0';
                        cout << "token is [ const digits ," << word_token << "]" << endl;

                        temp_input[symbol_num] = 'd';
                        // 转化成数字
                        int num = atoi(word_token);
                        temp_digit[symbol_num] = num;
                        symbol_num++;

                        w_state = 0;
                        w_next = w_forward;
                        break;
                    }
                    // processing the id  字母打头的有可能是标识符或者关键字
                    if (isalpha(next_c))
                    {

                        while ((next_c != ' ') && (isalpha(next_c) || isalnum(next_c) || next_c == '_'))
                        {
                            w_next++;
                            next_c = sentance_input[w_next];
                        }

                        if (next_c == ' ' || !(isalpha(next_c)))
                            w_next--;

                        x_num = 0;
                        while (w_forward <= w_next)
                        {
                            word_token[x_num] = sentance_input[w_forward];
                            w_forward++;
                            x_num++;
                        }
                        word_token[w_forward + 1] = '\0';

                        // char w_keyword[][8] = {"if", "else", "while", "do", "int", "float"};
                        for (int i = 0; i < 6; i++)
                        {
                            // ("word_token:%s  w_keyword[i]:%s\n", word_token, w_keyword[i]);
                            if (strcmp(word_token, w_keyword[i]) == 0)
                            {
                                cout << "token is [ keyword ," << word_token << "]" << endl;
                                // 表示是关键字
                                id_x = 1; // keyword not in table
                                switch (i)
                                {
                                case 0:
                                    temp_input[symbol_num] = 'e';
                                    symbol_num++;
                                    break;
                                case 1:
                                    temp_input[symbol_num] = 'f';
                                    symbol_num++;
                                    after_else = 1;
                                    break;
                                case 2:

                                    temp_input[symbol_num] = 'g';
                                    symbol_num++;
                                    break;
                                case 3:
                                    break;
                                case 4:

                                    temp_input[symbol_num] = 'b';
                                    symbol_num++;
                                    break;
                                case 5:

                                    temp_input[symbol_num] = 'c';
                                    symbol_num++;
                                    break;
                                }
                                break;
                            }
                        }
                        // 不是关键字，那就是标识符
                        if (id_x == 0)
                        {
                            cout << "token is [ id ," << word_token << "]" << endl;
                            strcpy(temp_token[symbol_num], word_token);
                            temp_input[symbol_num] = 'a';
                            symbol_num++;
                        }

                        w_state = 0;
                        id_x = 0;
                        w_next = w_forward;
                        break;
                    }
                    // processing the relation operators,
                    // the calculating operators and the other operators.
                    switch (next_c)
                    {
                    case '<':
                        w_state = 1;
                        break;
                    case '=':
                        w_state = 5;
                        break;
                    case '>':
                        w_state = 6;
                        break;
                    case '+':
                        w_state = 9;
                        break;
                    case '-':
                        w_state = 10;
                        break;
                    case '*':
                        w_state = 11;
                        break;
                    case '/':
                        w_state = 12;
                        break;
                    case '(':
                        w_state = 13;
                        break;
                    case ')':
                        w_state = 14;
                        break;
                    case ';':
                        w_state = 15;
                        break;
                    case '!':
                        w_state = 16;
                        break;
                    case '\'':
                        w_state = 17;
                        break;
                    // 读到末尾，while要退出了
                    case '\0':
                        w_state = 100;
                        // sentance_input="";
                        memset(sentance_input, 0x00, sizeof(char) * 200);
                        break;
                    default:
                        cout << "lexical analysis error:An unrecognized keyword" << next_c << " was found" << endl;
                        w_state = 0;
                        w_next++;
                        w_forward++;
                        break;
                    }

                    break;
                case 1:
                    w_next++;
                    next_c = sentance_input[w_next];
                    switch (next_c)
                    {
                    case '=':
                        w_state = 2;
                        break;

                    default:
                        w_state = 4;
                        temp_input[symbol_num] = '<';
                        symbol_num++;
                        break;
                    }

                    break;
                case 2:
                    x_num = 0;
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward + 1] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                
                case 4:
                    w_next--;

                    x_num = 0;
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 5:
                    x_num = 0;
                    if (sentance_input[w_next + 1] == '=')
                    {
                        w_next++;
                        next_c = sentance_input[w_next];
                    }
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    temp_input[symbol_num] = '=';
                    symbol_num++;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 17:
                    switch (w_state)
                    {
                    case 9:
                        temp_input[symbol_num] = '+';
                        symbol_num++;
                        break;
                    case 10:
                        temp_input[symbol_num] = '-';
                        symbol_num++;
                        break;
                    case 11:
                        temp_input[symbol_num] = '*';
                        symbol_num++;
                        break;
                    case 12:
                        temp_input[symbol_num] = '/';
                        symbol_num++;
                        break;
                    case 13:
                        temp_input[symbol_num] = '(';
                        symbol_num++;
                        break;
                    case 14:
                        temp_input[symbol_num] = ')';
                        symbol_num++;
                        break;
                    case 15:
                        ("after_else:%d\n", after_else);
                        if (after_else == 0)
                        {
                            temp_input[symbol_num] = ';';
                            symbol_num++;
                        }

                        break;
                    }
                    x_num = 0;
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    if (!((strcmp(word_token, ";") == 0) && (after_else == 1)))
                    {
                        cout << "token is [ op ," << word_token << "]" << endl;
                    }
                    if (after_else == 1)
                    {
                        after_else = 0;
                    }

                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 16:
                    x_num = 0;
                    if (sentance_input[w_next + 1] == '=')
                    {
                        w_next++;
                        next_c = sentance_input[w_next];
                    }
                    else
                        Show_Error();
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 6:
                    w_next++;
                    next_c = sentance_input[w_next];
                    switch (next_c)
                    {
                    case '=':
                        w_state = 7;
                        break;
                    default:
                        w_state = 8;
                        temp_input[symbol_num] = '>';
                        symbol_num++;
                        break;
                    }

                    break;
                case 7:
                    x_num = 0;
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 8:
                    w_next--;
                    x_num = 0;
                    while (w_forward <= w_next)
                    {
                        word_token[x_num] = sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                    }
                    word_token[w_forward] = '\0';
                    cout << "token is [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 100:
                    break;
                }

                //
                if (w_state == 100)
                    break;
            }
        }
        fclose(fp_soure);
    }
    else
    {
        cout << "open file error!" << endl;
    }

    temp_input[symbol_num] = '$';
    symbol_num++;
    temp_input[symbol_num] = '\0';

    if(choose == 1) return 0;
    cout << "lexical analysis has completed, press any key to start grammar analysis" << endl;
    // cout << "grammar analysis can only use '=' '>' '<' '==' '+' '-' '*' '/' '(' ')' 'if' 'else' 'while'" << endl;
    getchar();

    // for grammar analysis
    Init_LR();
    Init_Input();
    GetFirst();
    GetFollow();

    //初始化
    CLOSURE(&Status[0]);

    int status_num_old;

    // 构造Action表和GOTO， 构造LR分析表
    do
    {
        status_num_old = status_num;
        //遍历所有状态
        for (int sta_num = 0; sta_num <= status_num; sta_num++)
        {
            for (int i = 0; i < 28; i++) //遍历每个符号，共有28个符号
            {
                // 状态的项目集闭包，临时变量
                struct ISet TempSet;
                SetCopy(&TempSet, &Status[sta_num]);

                //去除移动到末尾的式子并标记规约
                // 对于每个产生式
                for (int j = 0; j < 24; j++)
                {
                    if (TempSet.SetItem[j][I[j].len] == 1)
                    {
                        TempSet.SetItem[j][I[j].len] = 0;
                        if (ItemValid(&TempSet, j) == 0)
                        {
                            TempSet.validItem[j] = 0;
                        }
                    }
                }
                // 数字映射到符号
                char c = Int2Sym(i);
                for (int j = 0; j < 24; j++)
                { //该符号读入后的情况
                    for (int ind = 0; ind < 10; ind++)
                    {
                        if (Status[sta_num].SetItem[j][ind] == 1)
                        {
                            int pos = ind;
                            if ((Status[sta_num].validItem[j]) &&
                                (pos < I[j].len) && (I[j].body[pos] == c))
                            {
                                TempSet.SetItem[j][pos + 1] = 1;
                                TempSet.SetItem[j][pos] = 0;
                            }
                            else
                            {
                                // TempSet.validItem[j] =0;
                                TempSet.SetItem[j][pos] = 0;
                            }
                        }
                        if (ItemValid(&TempSet, j) == 0)
                        {
                            TempSet.validItem[j] = 0;
                        }
                    }
                }

                CLOSURE(&TempSet);
                int exist = 0;    //该状态不存在
                int matchSet = 0; // Set turned to
                for (int sta_num2 = 0; sta_num2 <= status_num; sta_num2++)
                {
                    if (SetEqual(&Status[sta_num2], &TempSet))
                    {
                        exist = 1;
                        matchSet = sta_num2;
                        break;
                    }
                }
                if (exist == 0)
                {
                    if (SetValid(&TempSet) == 1)
                    {
                        status_num++;
                        GOTO[sta_num][i] = status_num;
                        ACT[sta_num][i].status_code = 1;
                        SetCopy(&Status[status_num], &TempSet);
                    }
                }
                else
                {
                    GOTO[sta_num][i] = matchSet;
                    ACT[sta_num][i].status_code = 1;
                }
            }
        }
    }
    // 直到不再变化为止
    while (status_num != status_num_old);

    // get the info of ISETS
    // 遍历自动机的所有状态， 填充表项，规约项
    for (int i = 0; i <= status_num; i++)
    {
        int has_reduce = 0;
        int reduce_num;
        int has_move = 0; // exist Item that has not moved to its' end
        // 遍历所有的产生式
        for (int j = 0; j < 24; j++)
        {
            int length = I[j].len;
            // 处理规约项目集
            if (Status[i].SetItem[j][length] == 1)
            {
                has_reduce = 1;
                reduce_num = j;
                break;
            }
        }
        // 修改需要规约的Action表
        if (has_reduce == 1)
        {
            for (int j = 0; j < 28; j++)
            {
                ACT[i][j].reduce = reduce_num;
                ACT[i][j].reducelen = I[reduce_num].len;
            }
        }

        // 处理移入项目
        for (int j = 0; j < 24; j++)
        {
            for (int k = 0; k < I[j].len; k++)
            {
                if (Status[i].SetItem[j][k] == 1)
                {
                    has_move = 1;
                    break;
                }
            }
        }

        // 如果既要移入，又要规约，产生移入规约冲突
        if ((has_move == 1) && (has_reduce == 1))
        {
            Status[i].conf = 1;
        }
    }

    // 主控程序，语法制导翻译，使用手动解决冲突的SLR(1)分析法
    // 在规约的过程中，使用语义栈进行中间代码生成
    while (1)
    {
        char c = input_code[parse_pos];
        // 把当前字符映射成数字
        int cnum = Sym2Int(c);
        // 获取常数
        int value = digit_table[parse_pos];
        char name[10];
        // 获取标识符
        strcpy(name, token_table[parse_pos]);
        // 循环退出的条件
        if (c == '$')
        {
            if (GetStackTop() == 1)
            {
                cout << endl;
                cout << "grammar analysis answer:";
                cout << "accept!" << endl;
                cout << endl;
                break;
            }
        }
        
        // 获取栈顶状态
        int status = GetStackTop();
        // 如果当前状态，存在移入规约冲突
        if (Status[status].conf == 1)
        {
            // 要规约到的状态
            int Itemnum = ACT[status][cnum].reduce;
            // 获取要规约到的产生式的左部
            int headnum = I[Itemnum].head;
            // 如果     25是else  手动处理了移入规约冲突
            // 进行规约
            if ((Follow[headnum][cnum - 9] == 1) && (cnum != 25))
            { // in Follow Set  对'else'特殊规定
                PopStack(ACT[status][cnum].reducelen);
                int pre_status = GetStackTop();
                Reduce_Symbol(ACT[status][cnum].reduce);
                PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
            }
            // 这项为空，出错
            else if (ACT[status][cnum].status_code == 0)
            {
                cout << "grammar analysis error" << endl;
                exit(1);
                break;
            }
            else // 正常的移入
            {
                int nextstatus = GOTO[status][cnum];
                PushStack(nextstatus);
                switch (cnum)
                {
                    // 遇到了id
                case 9:
                // 向语义栈添加元素id
                    Insert_Symbol_id(name);
                    break;
                    // 遇到了digits
                case 23:
                // 向语义栈添加元素常数value
                    Insert_Symbol_digits(value);
                    break;
                // 如果不是id或者value，就向语义栈添加
                default:
                    Insert_Symbol(cnum);
                }
                // 只有移入parse_pos才会++
                parse_pos++;
            }
        }
        // 不存在冲突，可以规约或者移入
        else
        {
            // 规约
            if (ACT[status][cnum].reduce != -1)
            {

                PopStack(ACT[status][cnum].reducelen);
                int pre_status = GetStackTop();
                Reduce_Symbol(ACT[status][cnum].reduce);
                PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                continue;
            }

            // 出错
            if (ACT[status][cnum].status_code == 0)
            {
                cout << "error" << endl;
                break;
            }

            // 移入
            int nextstatus = GOTO[status][cnum];
            switch (cnum)
            {
                // 如果是标识符id
            case 9:
                Insert_Symbol_id(name);
                break;
                // 如果是常数digits
            case 23:
                Insert_Symbol_digits(value);
                break;
            default:
                Insert_Symbol(cnum);
            }

            PushStack(nextstatus);
            parse_pos++;
        }
    }

    if(choose == 2) return 0;

    cout << "final code generated" << endl;
    for (int i = 0; i < current_line; i++)
    {
        cout << i << "," << inter_code[i] << endl;
    }


    cout << endl;
    cout << "symbol table:" << endl;

    for (int i = 0; i < sym_number; i++)
    {

        switch (Symtable[i].sym_type)
        {
        case 11:
            cout << "symbol type:int  " << endl;
            break;
        case 12:
            cout << "symbol type:int  " << endl;
            break;
        }
        cout << "symbol name:" << Symtable[i].name << endl;
    }
    return 0;
}
