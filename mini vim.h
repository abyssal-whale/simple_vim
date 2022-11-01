#pragma once
#include<iostream>
#include<Windows.h>
#include<string.h>
#include<stdlib.h>
#include <conio.h>

class vim_operator {
private:
	//normal模式下的函数们
	bool normal_function();	//normal模式功能表函数
	bool lastline();	//将指令输出在最后一行
	bool withdraw();	//执行命令u
	bool Delete();	//执行命令x
	bool NormalMove(const char& a);	//normal光标位置移动
	bool OpenFile(const char* filename);	//打开文件
	bool WriteFile(const char* filename);	//写入文件
	bool search();	//执行搜索命令
	bool BM(char* t, const int m);	//利用BM算法匹配字符串
	bool clean();	//清除控制台页面中的所有内容

	//insert模式下的函数们
	bool insert_function();	//insert模式功能表函数
	bool InsertMove();	//insert光标位置移动
	bool grow();	//行数超过时的增长
	bool Insert_cin(char ch);	//insert模式下单字符的插入

	//bonus
	bool pageup();	//向上翻页
	bool pagedown();	//向下翻页

	//有用的数据和结构们
	char mode;	//记录模式
	int height;  //记录窗口高度
	int row_num;  //记录最后的一行的行号
	int page_num;  //页数的最大值
	const char* TempFileName = { "tempfile.txt" };  //临时文件的名称，撤销操作用
	const char* LastFileName = { "lastfile.txt" };
	int** num;  //存储每一行的元素个数,两个维度分别是页数和行数
	int** find_enter;  //记录每一行的的回车符是不是在本行，因为在下一行时，再在本行输入，除了正常输入，还需要回车
	int mod_re; // 判断是撤销操作还是反撤销操作
	char state_judge;  //判断撤销是撤销insert还是x
	struct feedback//用来记录撤销前的光标位置、字符、是撤销还是反撤销
	{
		COORD pre = { 0,0 };
		char ch = 0;
		int mod_re = 1;
	} Feedback;

public:
	bool judge();	//用来判断属于哪一种模式
	void renew();	//更新目前光标位置
	vim_operator() //构造函数
	{
		mode = 'n';//默认模式为normal模式
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO pBuffer;
		GetConsoleScreenBufferInfo(hStdout, &pBuffer);
		height = pBuffer.srWindow.Bottom - pBuffer.srWindow.Top + 1;
		num = new int* [100];
		find_enter = new int* [100];
		num[0] = new int[height];  //一页
		find_enter[0] = new int[height];
		page_num = 1;
		row_num = 0; //代表现在最后一行为第一行
		mod_re = 1;
		//which_back = 0; //0表示没有撤回，1表示之前操作为insert，2表示之前操作为/x
		for (int i = 0; i < height; i++)
		{
			num[0][i] = 0;  //初始化每一行的元素个数都为0
			find_enter[0][i] = 0; //0表示该行为开头，即上行转下来时需要回车
		}
	}
};


