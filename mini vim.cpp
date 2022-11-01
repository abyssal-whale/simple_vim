#include<iostream>
#include"mini vim.h"
#include<Windows.h>
#include<string.h>
#include<fstream>
#include<stdlib.h>
#include <conio.h>
using namespace std;

struct node
{
	int x, y;
};

node get_coordinate(HANDLE hStdout)//给定控制台界面，返回光标坐标
{
	node coordinate;
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	coordinate.x = pBuffer.dwCursorPosition.X;
	coordinate.y = pBuffer.dwCursorPosition.Y;
	return coordinate;
}

void vim_operator::renew()  //更新cur指向现在光标位置
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO INFO;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hOut, &INFO); //得到现在屏幕缓冲区的光标位置
}

bool vim_operator::judge()//用来判断属于哪一种模式
{
	while (true)
	{
		if (mode == 'n')
		{
			if (!normal_function())
				return false;
		}
		else if (mode == 'i')
		{
			if (!insert_function())
				return false;
		}
	}
	return false;
}

bool vim_operator::normal_function()//normal模式下的功能表函数
{
	while (1)
	{
		char ch;
		ch = _getch(); //将缓冲区中的数据以字符的形式读出
		if (ch == ':')
		{
			if (!lastline()) return false;
		}
		else if (ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l')  //移动光标
		{
			if (!NormalMove(ch)) return false;
		}
		else if (ch == -32)
		{
			if (!InsertMove()) return false;
		}
		else if (ch == '/')
		{
			search();
		}
		else if (ch == 'x')
		{
			Feedback.ch = 0;
			Feedback.mod_re = 1;
			Feedback.pre = { 0,0 };
			state_judge = 'x'; //确定该操作为撤销
			Delete();
		}
		else if (ch == 'i')
		{
			state_judge = 'i';  //确定该操作为插入
			mode = 'i';
			return true;
		}
		else if (ch == 'u')
		{
			if (!withdraw())
				return false;
		}
	}
}

bool vim_operator::lastline()//将指令输出在最后一行
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	COORD lowest;	//记录最底端开头的光标位置
	COORD record = pBuffer.dwCursorPosition;
	lowest.X = 0;
	lowest.Y = pBuffer.srWindow.Bottom;
	int page = pBuffer.srWindow.Bottom / height;
	int row = pBuffer.srWindow.Bottom % height;
	if (page > page_num)
		num[page][row] = 0;
	char* temp = new char[num[page][row] + 2];  //记录原来最后一行
	temp[num[page][row]] = 0;  //保证0结尾，使得temp成为字符串
	SetConsoleCursorPosition(hStdout, lowest);
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hStdout, temp, num[page][row], lowest, num1);	//读取控制台屏幕缓冲区中最后一行的元素
	for (int i = 0; i < num[page][row]; i++)
		cout << " ";  //清空该行原有数据
	SetConsoleCursorPosition(hStdout, lowest);
	int count = 0;
	cout << ':';
	char str[5] = { 0 };
	cin >> str;
	if (str[0] == 'q' && str[1] == 0) //退出
	{
		DeleteFileA(TempFileName);
		DeleteFileA(LastFileName);
		return false;
	}
	else if (str[0] == 'o' && str[1] == 'p' && str[2] == 'e' && str[3] == 'n' && str[4] == 0)  // 文件读取
	{
		char file_name[100];
		cin >> file_name;
		for (count = 0; file_name[count] != 0; count++);
		count += 6;
		//还原
		SetConsoleCursorPosition(hStdout, lowest);
		for (int i = 0; i < count; i++)
			cout << " ";  //清空该行原有数据
		SetConsoleCursorPosition(hStdout, lowest);
		cout << temp;  //还原最后一行的元素
		SetConsoleCursorPosition(hStdout, record);  //还原原有输出窗口光标位置
		if (!OpenFile(file_name))
			return false;
	}
	else if (str[0] == 'w' && str[1] == 0)  //文件写入
	{
		char file_name[100];
		cin >> file_name;
		for (count = 0; file_name[count] != 0; count++);
		count += 3;
		//还原
		SetConsoleCursorPosition(hStdout, lowest);
		for (int i = 0; i < count; i++)
			cout << " ";  //清空该行原有数据
		SetConsoleCursorPosition(hStdout, lowest);
		cout << temp;  //还原最后一行的元素
		SetConsoleCursorPosition(hStdout, { 0,0 });  //还原原有输出窗口光标位置
		SetConsoleCursorPosition(hStdout, record);  //还原原有输出窗口光标位置
		if (!WriteFile(file_name))
			return false;
	}
}

bool vim_operator::grow()
{
	num[page_num] = new int[height];
	find_enter[page_num] = new int[height];
	for (int i = 0; i < height; i++)
	{
		num[page_num][i] = 0;  //初始化每一行的元素个数都为0
		find_enter[page_num][i] = 0;
	}
	page_num++;
	return true;
}

bool vim_operator::OpenFile(const char* filename)
{
	char p;
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO scr;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hStdout, &scr);
	COORD pos = scr.dwCursorPosition;
	int page = pos.Y / height;
	int row = pos.Y % height;
	ifstream infile;
	infile.open(filename);
	int count_char = 0;
	while (!infile.fail())
	{
		p = infile.get();
		//当文件中为‘\n’时，需要对控制台的row_num 进行增加
		if (p == '\n')
		{
			cout << '\n';
			if (page * height + row == row_num)
				row_num++;
			if (row_num == page_num * height) //如果总的最后一行为最大页的最后一行，则需要新加页
				if (!grow()) return false;
			page += (row + 1) / height;
			row = (row + 1) % height;
			count_char = 0;
		}
		else
		{
			HANDLE hStdout;
			CONSOLE_SCREEN_BUFFER_INFO scr;
			hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
			GetConsoleScreenBufferInfo(hStdout, &scr);
			if (count_char == scr.srWindow.Right + 1)
			{
				if (page * height + row == row_num)
					row_num++;
				if (row_num == page_num * height) //如果总的最后一行为最大页的最后一行，则需要新加页
					if (!grow()) return false;
				page += (row + 1) / height;
				row = (row + 1) % height;
				count_char = 0;
			}
			else
				count_char++;
			cout << p;
			num[page][row]++;
		}
	}
	GetConsoleScreenBufferInfo(hStdout, &scr);
	pos = scr.dwCursorPosition;
	if (pos.X < 3)
		pos.X = 0;
	else
		pos.X -= 3;
	SetConsoleCursorPosition(hStdout, pos);
	num[page][row]--;
	infile.close();
	return true;
}

bool vim_operator::WriteFile(const char* filename)
{
	ofstream outfile;
	outfile.open(filename);
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO scr;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hStdout, &scr);
	COORD before = scr.dwCursorPosition;
	COORD pos = { 0,0 };
	while (pos.Y <= row_num)
	{
		int page = pos.Y / height;
		int row = pos.Y % height;
		char* temp = new char[num[page][row] + 1]; //存储原有最后一行的元素
		temp[num[page][row]] = 0;  //保证0结尾，使得temp成为字符串
		unsigned long num1 = 0;
		ReadConsoleOutputCharacterA(hStdout, temp, num[page][row], pos, &num1);//读取控制台屏幕缓冲区中第一行的元素
		outfile << temp;
		delete[]temp;
		if (pos.Y != row_num)  //防止最后一次多输出一行
			outfile << endl;
		pos.Y++;
	}
	SetConsoleCursorPosition(hStdout, before);  //还原光标位置
	outfile.close();
	return true;
}

bool vim_operator::NormalMove(const char& ch)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	COORD present;	//记录当前的光标位置
	present.X = get_coordinate(hStdout).x;
	present.Y = get_coordinate(hStdout).y;
	if (ch == 'h')
	{
		present.X--;
		SetConsoleCursorPosition(hStdout, present);
		return true;
	}
	else if (ch == 'j')
	{
		present.Y++;
		SetConsoleCursorPosition(hStdout, present);
		return true;
	}
	else if (ch == 'k')
	{
		present.Y--;
		SetConsoleCursorPosition(hStdout, present);
		return true;
	}
	else if (ch == 'l')
	{
		present.X++;
		SetConsoleCursorPosition(hStdout, present);
		return true;
	}
	else
		return false;
}

bool vim_operator::InsertMove()
{
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO INFO;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdout, &INFO); //得到现在屏幕缓冲区的光标位置
	COORD present = INFO.dwCursorPosition;
	int page = present.Y / height;
	int row = present.Y % height;
	char a = _getch();
	if (a == 75) //左移
	{
		if (present.X > 0)
			present.X -= 1;
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 80) // 下移
	{
		if (present.Y < row_num)
		{
			present.Y += 1;
			if (row == height - 1)
			{
				page += 1;
				row = 0;
			}
			else
				row += 1;
			if (present.X > num[page][row])
				present.X = num[page][row];
		}
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 72)  // 上移
	{
		if (present.Y > 0)
		{
			present.Y -= 1;
			if (row == 0)
			{
				page -= 1;
				row = height - 1;
			}
			else
				row -= 1;
			if (present.X > num[page][row])
				present.X = num[page][row];
			if (present.X < 0)  // 防止数组数据出现其他情况
				present.X = 0;
		}
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 77)  // 右移
	{
		if (present.X < num[page][row])
			present.X += 1;
		else if (num[page][row] == 1)
		{
			if (present.Y < row_num)
			{
				present.X = 0;
				present.Y += 1;
			}
		}
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 73) // page_up时不尽光标会改变，屏幕缓冲区所
	{
		pageup();
	}
	else if (a == 81)
	{
		pagedown();
	}
	else
		return false;
	return true;
}

bool vim_operator::insert_function()
{
	while (1)
	{
		char a = _getch();
		if (a == 27) //检测按下ESC
		{
			//还原到前一个光标位置
			HANDLE hOut;
			CONSOLE_SCREEN_BUFFER_INFO scr;
			hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
			GetConsoleScreenBufferInfo(hOut, &scr);
			COORD before = scr.dwCursorPosition;
			if (before.X != 0)
				before.X -= 1;
			SetConsoleCursorPosition(hOut, before);  // 还原原有光标位置
			mode = 'n';
			renew();
			return true;
		}
		else if (a == -32)  //按下方向键
		{
			if (!InsertMove())
				return false;
		}
		else if (a == 8)
		{
			if (!Delete())
				return false;
		}
		else  //直接输入，输入为覆盖原有数据的输入
		{
			if (!Insert_cin(a))
				return false;
		}
	}
}

int Find_Distance(char* t, const char ch, const int len) //BM算法求移动距离
{
	int i = len - 1;
	if (ch == t[i])
		return len;
	i--;
	while (i >= 0)
	{
		if (ch == t[i])
			return len - 1 - i;
		else
			i--;
	}
	return len;
}

bool vim_operator::BM(char* t, const int m)  // 此BM算法只适用与控制台，虽然原理是一样的
{
	int i = m - 1;
	int j = m - 1;
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	char* info = new char[m + 1];
	CONSOLE_SCREEN_BUFFER_INFO INFO;
	GetConsoleScreenBufferInfo(hOut, &INFO); //得到现在屏幕缓冲区的光标位置
	COORD now = INFO.dwCursorPosition;
	COORD position = now;
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hOut, info, m, position, num1);//读取控制台屏幕缓冲区中的m个元素
	while (true)
	{
		if (info[i] == t[j])
		{
			i--;
			j--;
		}
		else if (j < 0)
		{
			//找到匹配，设置光标指向匹配的第一个元素首字母
			SetConsoleCursorPosition(hOut, position);
			return true;
		}
		else
		{
			if (position.Y > row_num)
			{
				int page2 = row_num / height;
				int row = row_num % height;
				COORD posi = { num[page2][row], row_num };
				SetConsoleCursorPosition(hOut, posi);
				return true;
			}
			i = Find_Distance(t, info[i], m);
			j = m - 1;
			//设置下一次检索的位置
			int page = position.Y / height;
			int row = position.Y % height;
			position.Y += (position.X + i) / num[page][row];
			position.X = (position.X + i) % num[page][row];
			i = j;
			ReadConsoleOutputCharacterA(hOut, info, m, position, num1);
		}
	}
	return false;
}

bool vim_operator::search()
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO scr;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hOut, &scr);
	COORD before = scr.dwCursorPosition;
	COORD pos = { 0,scr.srWindow.Bottom };  //将光标移到当前窗口缓冲区最下方
	int page = scr.srWindow.Bottom / height;
	int row = scr.srWindow.Bottom % height;
	char* temp = new char(num[page][row] + 1);  //存储原有最后一行的元素
	temp[num[page][row]] = 0;  //保证0结尾，使得temp成为字符串
	SetConsoleCursorPosition(hOut, pos);
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row], pos, num1);//读取控制台屏幕缓冲区中最后一行的元素
	for (int i = 0; i < num[page][row]; i++)
		cout << " ";
	SetConsoleCursorPosition(hOut, pos);
	cout << '/';
	//存储pattern
	int count = 0;
	char pattern[100] = { 0 };
	cin >> pattern;
	while (pattern[count++] != 0);
	count--;
	SetConsoleCursorPosition(hOut, pos);
	for (int i = 0; i < count + 2; i++)
		cout << " ";  //清空该行原有数据
	SetConsoleCursorPosition(hOut, pos);
	cout << temp;  //还原最后一行的元素
	SetConsoleCursorPosition(hOut, { 0,0 });  //还原原有输出窗口光标位置
	SetConsoleCursorPosition(hOut, before);  // 还原原有光标位置
	if (!BM(pattern, count))
		return false;
	else
		return true;
}

bool vim_operator::Delete()
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO hBuffer;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hOut, &hBuffer);
	COORD position = hBuffer.dwCursorPosition;
	unsigned long num1 = 0;
	int page = position.Y / height;
	int row = position.Y % height;
	if (position.X == 0 && num[page][row] == 0)
		return true;
	char* temp = new char[num[page][row] - hBuffer.dwCursorPosition.X]; //存储光标后的元素，在后续进行输出
	temp[num[page][row] - position.X - 1] = 0;  //保证0结尾，使得temp成为字符串
	position.X += 1;
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row] - position.X, position, &num1);//读取控制台屏幕缓冲区中最后一行的元素
	position.X -= 1;
	if (Feedback.mod_re == 1)  //为撤销操作做准备
	{
		Feedback.pre = position;
		ReadConsoleOutputCharacterA(hOut, &Feedback.ch, 1, Feedback.pre, &num1);//读取光标前的第一个元素，为撤回操作做准备
	}
	SetConsoleCursorPosition(hOut, position);
	cout << temp << ' ';
	delete[]temp;
	num[page][row]--;
	position = hBuffer.dwCursorPosition;
	if (position.X != 0)
		position.X -= 1;
	SetConsoleCursorPosition(hOut, position);
	return true;
}

bool vim_operator::clean()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { 0,0 };
	SetConsoleCursorPosition(hOut, position);
	while (position.Y <= row_num)
	{
		int page = position.Y / height;
		int row = position.Y % height;
		for (int i = 0; i < num[page][row]; i++)
			cout << ' ';
		cout << endl;
		position.Y += 1;
	}
	for (int i = 0; i < page_num; i++)
		delete[] num[i];
	num[0] = new int[height];
	for (int i = 0; i < height; i++)
		num[0][i] = 0;  //初始化每一行的元素个数都为0
	page_num = 1;
	row_num = 0; //代表现在最后一行为第一行
	SetConsoleCursorPosition(hOut, { 0,0 });
	return true;
}

bool vim_operator::Insert_cin(char ch)
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO hBuffer;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hOut, &hBuffer);
	unsigned long num1 = 0;
	COORD position = hBuffer.dwCursorPosition;
	int page = position.Y / height;
	int row = position.Y % height;
	//这个是更改find_enter数组，为了自动转行而设置
	if (position.X == 0 && !(page == 0 && row == 0)) //防止第一行出现问题
	{
		if (row != 0)
			row -= 1;
		else
		{
			page -= 1;
			row = height;
		}
	}
	page = position.Y / height;
	row = position.Y % height;
	char* temp = new char[num[page][row] + 1 - hBuffer.dwCursorPosition.X]; //存储光标后面的元素
	temp[num[page][row] - position.X] = 0;  //保证0结尾，使得temp成为字符串
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row] - position.X, position, &num1);//读取光标后面的元素
	char temp_one = 0;
	if (num[page][row] - position.X != 0)
	{
		temp_one = temp[num[page][row] - position.X - 1];
		temp[num[page][row] - position.X - 1] = 0;
	}
	cout << ch << temp;
	if (num[page][row] == hBuffer.srWindow.Right + 1) // 判断输入该字符后，该行最后一个字符会不会转行
	{
		cout << endl;
		if (position.Y == row_num)
		{
			row_num++;
			if (row_num == page_num * height) //如果总的最后一行为最大页的最后一行，则需要新加页
				if (!grow()) return false;
		}
		if (row != height)
			row += 1;
		else
		{
			row = 0;
			page += 1;
		}
		Insert_cin(temp_one);
	}
	else
	{
		num[page][row]++;
		cout << temp_one;
	}
	position = hBuffer.dwCursorPosition; //插入时候的光标位置
	if (position.X == hBuffer.srWindow.Right)
	{
		position.X = 0;
		position.Y += 1;
	}
	else
		position.X += 1;
	SetConsoleCursorPosition(hOut, position);
	delete[]temp;
	return true;
}

bool vim_operator::withdraw()
{
	if (state_judge == 'i')  //撤销的是insert模式输入
	{
		if (mod_re == 1)  //第一次撤销（奇数次撤销），真正的撤销
		{
			WriteFile(LastFileName);
			clean();
			OpenFile(TempFileName);
			mod_re = 2;
		}
		else  //第二次（偶数次）撤销，即还原操作
		{
			WriteFile(TempFileName);
			clean();
			OpenFile(LastFileName);
			mod_re = 1;
		}
	}
	else if (state_judge == 'x')  //撤销的是x命令删除当前光标数据
	{
		HANDLE hOut;
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(hOut, Feedback.pre);
		if (Feedback.mod_re == 1)  //第一次撤销（奇数次撤销），真正的撤销
		{
			Insert_cin(Feedback.ch);
			Feedback.mod_re = 2;
			SetConsoleCursorPosition(hOut, Feedback.pre);
		}
		else  //第二次（偶数次）撤销，即还原操作
		{
			Delete();
			Feedback.mod_re = 1;
		}
	}
	return true;
}

bool vim_operator::pageup()
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO scr;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hOut, &scr);
	COORD position = scr.dwCursorPosition;
	int page = position.Y / height;
	int row = position.Y % height;
	if (page != 0)
	{
		page -= 1;
		position.Y -= height;
		if (position.X > num[page][row])
			position.X = num[page][row];
		SetConsoleCursorPosition(hOut, position);
	}
	SMALL_RECT rect{ 0,page * height, scr.srWindow.Right, (page + 1) * height };
	SetConsoleWindowInfo(hOut, true, &rect);
	return true;
}

bool vim_operator::pagedown()
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO scr;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // 获取标准输出句柄 
	GetConsoleScreenBufferInfo(hOut, &scr);
	COORD position = scr.dwCursorPosition;
	int page = position.Y / height;
	int row = position.Y % height;
	if (page != page_num)
	{
		page += 1;
		position.Y += height;
		if (position.Y > row_num)
			position.Y = row_num;
		if (position.X > num[page][row])
			position.X = num[page][row];
		SetConsoleCursorPosition(hOut, position);
	}
	SMALL_RECT rect{ 0,page * height, scr.srWindow.Right, (page + 1) * height };
	SetConsoleWindowInfo(hOut, true, &rect);
	return true;
}

int main()
{
	vim_operator vim;
	vim.judge();
	return 0;
}
