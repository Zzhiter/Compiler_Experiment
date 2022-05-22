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
        // cout << "Token generated:" << endl;

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
                        // cout << "token :  [ const digits ," << word_token << "]" << endl;

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
                                // cout << "token :  [ keyword ," << word_token << "]" << endl;
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
                            // cout << "token :  [ id ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
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
                        // cout << "token :  [ op ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
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
                    // cout << "token :  [ op ," << word_token << "]" << endl;
                    w_state = 0;
                    w_next = w_forward;
                    break;
                case 100:
                    break;
                }

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

    // if(choose == 1) return 0;
    cout << "Lexical analysis has completed, press any key to start grammar analysis" << endl;
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
                cout << "Grammar analysis :";
                cout << "Accept!" << endl;
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
            // 如果      手动处理了移入规约冲突      25是else 
            // 进行规约
            if ((Follow[headnum][cnum - 9] == 1) && (cnum != 25))
            { // in Follow Set  对'else'特殊规定
                PopStack(ACT[status][cnum].reducelen);
                int pre_status = GetStackTop();
                PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
            }
            // 这项为空，出错
            else if (ACT[status][cnum].status_code == 0)
            {
                cout << "Grammar analysis error" << endl;
                exit(1);
                break;
            }
            else // 正常的移入
            {
                int nextstatus = GOTO[status][cnum];
                PushStack(nextstatus);
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
                PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                continue;
            }

            // 出错
            if (ACT[status][cnum].status_code == 0)
            {
                cout << "Grammar analysis error" << endl;
                break;
            }

            // 移入
            int nextstatus = GOTO[status][cnum];
            PushStack(nextstatus);
            parse_pos++;
        }
    }

    // if(choose == 2) return 0;

    return 0;
}
