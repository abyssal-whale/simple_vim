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

node get_coordinate(HANDLE hStdout)//��������̨���棬���ع������
{
	node coordinate;
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	coordinate.x = pBuffer.dwCursorPosition.X;
	coordinate.y = pBuffer.dwCursorPosition.Y;
	return coordinate;
}

void vim_operator::renew()  //����curָ�����ڹ��λ��
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO INFO;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hOut, &INFO); //�õ�������Ļ�������Ĺ��λ��
}

bool vim_operator::judge()//�����ж�������һ��ģʽ
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

bool vim_operator::normal_function()//normalģʽ�µĹ��ܱ���
{
	while (1)
	{
		char ch;
		ch = _getch(); //���������е��������ַ�����ʽ����
		if (ch == ':')
		{
			if (!lastline()) return false;
		}
		else if (ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l')  //�ƶ����
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
			state_judge = 'x'; //ȷ���ò���Ϊ����
			Delete();
		}
		else if (ch == 'i')
		{
			state_judge = 'i';  //ȷ���ò���Ϊ����
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

bool vim_operator::lastline()//��ָ����������һ��
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	COORD lowest;	//��¼��׶˿�ͷ�Ĺ��λ��
	COORD record = pBuffer.dwCursorPosition;
	lowest.X = 0;
	lowest.Y = pBuffer.srWindow.Bottom;
	int page = pBuffer.srWindow.Bottom / height;
	int row = pBuffer.srWindow.Bottom % height;
	if (page > page_num)
		num[page][row] = 0;
	char* temp = new char[num[page][row] + 2];  //��¼ԭ�����һ��
	temp[num[page][row]] = 0;  //��֤0��β��ʹ��temp��Ϊ�ַ���
	SetConsoleCursorPosition(hStdout, lowest);
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hStdout, temp, num[page][row], lowest, num1);	//��ȡ����̨��Ļ�����������һ�е�Ԫ��
	for (int i = 0; i < num[page][row]; i++)
		cout << " ";  //��ո���ԭ������
	SetConsoleCursorPosition(hStdout, lowest);
	int count = 0;
	cout << ':';
	char str[5] = { 0 };
	cin >> str;
	if (str[0] == 'q' && str[1] == 0) //�˳�
	{
		DeleteFileA(TempFileName);
		DeleteFileA(LastFileName);
		return false;
	}
	else if (str[0] == 'o' && str[1] == 'p' && str[2] == 'e' && str[3] == 'n' && str[4] == 0)  // �ļ���ȡ
	{
		char file_name[100];
		cin >> file_name;
		for (count = 0; file_name[count] != 0; count++);
		count += 6;
		//��ԭ
		SetConsoleCursorPosition(hStdout, lowest);
		for (int i = 0; i < count; i++)
			cout << " ";  //��ո���ԭ������
		SetConsoleCursorPosition(hStdout, lowest);
		cout << temp;  //��ԭ���һ�е�Ԫ��
		SetConsoleCursorPosition(hStdout, record);  //��ԭԭ��������ڹ��λ��
		if (!OpenFile(file_name))
			return false;
	}
	else if (str[0] == 'w' && str[1] == 0)  //�ļ�д��
	{
		char file_name[100];
		cin >> file_name;
		for (count = 0; file_name[count] != 0; count++);
		count += 3;
		//��ԭ
		SetConsoleCursorPosition(hStdout, lowest);
		for (int i = 0; i < count; i++)
			cout << " ";  //��ո���ԭ������
		SetConsoleCursorPosition(hStdout, lowest);
		cout << temp;  //��ԭ���һ�е�Ԫ��
		SetConsoleCursorPosition(hStdout, { 0,0 });  //��ԭԭ��������ڹ��λ��
		SetConsoleCursorPosition(hStdout, record);  //��ԭԭ��������ڹ��λ��
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
		num[page_num][i] = 0;  //��ʼ��ÿһ�е�Ԫ�ظ�����Ϊ0
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
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
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
		//���ļ���Ϊ��\n��ʱ����Ҫ�Կ���̨��row_num ��������
		if (p == '\n')
		{
			cout << '\n';
			if (page * height + row == row_num)
				row_num++;
			if (row_num == page_num * height) //����ܵ����һ��Ϊ���ҳ�����һ�У�����Ҫ�¼�ҳ
				if (!grow()) return false;
			page += (row + 1) / height;
			row = (row + 1) % height;
			count_char = 0;
		}
		else
		{
			HANDLE hStdout;
			CONSOLE_SCREEN_BUFFER_INFO scr;
			hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
			GetConsoleScreenBufferInfo(hStdout, &scr);
			if (count_char == scr.srWindow.Right + 1)
			{
				if (page * height + row == row_num)
					row_num++;
				if (row_num == page_num * height) //����ܵ����һ��Ϊ���ҳ�����һ�У�����Ҫ�¼�ҳ
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
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
	GetConsoleScreenBufferInfo(hStdout, &scr);
	COORD before = scr.dwCursorPosition;
	COORD pos = { 0,0 };
	while (pos.Y <= row_num)
	{
		int page = pos.Y / height;
		int row = pos.Y % height;
		char* temp = new char[num[page][row] + 1]; //�洢ԭ�����һ�е�Ԫ��
		temp[num[page][row]] = 0;  //��֤0��β��ʹ��temp��Ϊ�ַ���
		unsigned long num1 = 0;
		ReadConsoleOutputCharacterA(hStdout, temp, num[page][row], pos, &num1);//��ȡ����̨��Ļ�������е�һ�е�Ԫ��
		outfile << temp;
		delete[]temp;
		if (pos.Y != row_num)  //��ֹ���һ�ζ����һ��
			outfile << endl;
		pos.Y++;
	}
	SetConsoleCursorPosition(hStdout, before);  //��ԭ���λ��
	outfile.close();
	return true;
}

bool vim_operator::NormalMove(const char& ch)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;
	GetConsoleScreenBufferInfo(hStdout, &pBuffer);
	COORD present;	//��¼��ǰ�Ĺ��λ��
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
	GetConsoleScreenBufferInfo(hStdout, &INFO); //�õ�������Ļ�������Ĺ��λ��
	COORD present = INFO.dwCursorPosition;
	int page = present.Y / height;
	int row = present.Y % height;
	char a = _getch();
	if (a == 75) //����
	{
		if (present.X > 0)
			present.X -= 1;
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 80) // ����
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
	else if (a == 72)  // ����
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
			if (present.X < 0)  // ��ֹ�������ݳ����������
				present.X = 0;
		}
		SetConsoleCursorPosition(hStdout, present);
	}
	else if (a == 77)  // ����
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
	else if (a == 73) // page_upʱ��������ı䣬��Ļ��������
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
		if (a == 27) //��ⰴ��ESC
		{
			//��ԭ��ǰһ�����λ��
			HANDLE hOut;
			CONSOLE_SCREEN_BUFFER_INFO scr;
			hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
			GetConsoleScreenBufferInfo(hOut, &scr);
			COORD before = scr.dwCursorPosition;
			if (before.X != 0)
				before.X -= 1;
			SetConsoleCursorPosition(hOut, before);  // ��ԭԭ�й��λ��
			mode = 'n';
			renew();
			return true;
		}
		else if (a == -32)  //���·����
		{
			if (!InsertMove())
				return false;
		}
		else if (a == 8)
		{
			if (!Delete())
				return false;
		}
		else  //ֱ�����룬����Ϊ����ԭ�����ݵ�����
		{
			if (!Insert_cin(a))
				return false;
		}
	}
}

int Find_Distance(char* t, const char ch, const int len) //BM�㷨���ƶ�����
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

bool vim_operator::BM(char* t, const int m)  // ��BM�㷨ֻ���������̨����Ȼԭ����һ����
{
	int i = m - 1;
	int j = m - 1;
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	char* info = new char[m + 1];
	CONSOLE_SCREEN_BUFFER_INFO INFO;
	GetConsoleScreenBufferInfo(hOut, &INFO); //�õ�������Ļ�������Ĺ��λ��
	COORD now = INFO.dwCursorPosition;
	COORD position = now;
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hOut, info, m, position, num1);//��ȡ����̨��Ļ�������е�m��Ԫ��
	while (true)
	{
		if (info[i] == t[j])
		{
			i--;
			j--;
		}
		else if (j < 0)
		{
			//�ҵ�ƥ�䣬���ù��ָ��ƥ��ĵ�һ��Ԫ������ĸ
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
			//������һ�μ�����λ��
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
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
	GetConsoleScreenBufferInfo(hOut, &scr);
	COORD before = scr.dwCursorPosition;
	COORD pos = { 0,scr.srWindow.Bottom };  //������Ƶ���ǰ���ڻ��������·�
	int page = scr.srWindow.Bottom / height;
	int row = scr.srWindow.Bottom % height;
	char* temp = new char(num[page][row] + 1);  //�洢ԭ�����һ�е�Ԫ��
	temp[num[page][row]] = 0;  //��֤0��β��ʹ��temp��Ϊ�ַ���
	SetConsoleCursorPosition(hOut, pos);
	unsigned long* num1 = new unsigned long;
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row], pos, num1);//��ȡ����̨��Ļ�����������һ�е�Ԫ��
	for (int i = 0; i < num[page][row]; i++)
		cout << " ";
	SetConsoleCursorPosition(hOut, pos);
	cout << '/';
	//�洢pattern
	int count = 0;
	char pattern[100] = { 0 };
	cin >> pattern;
	while (pattern[count++] != 0);
	count--;
	SetConsoleCursorPosition(hOut, pos);
	for (int i = 0; i < count + 2; i++)
		cout << " ";  //��ո���ԭ������
	SetConsoleCursorPosition(hOut, pos);
	cout << temp;  //��ԭ���һ�е�Ԫ��
	SetConsoleCursorPosition(hOut, { 0,0 });  //��ԭԭ��������ڹ��λ��
	SetConsoleCursorPosition(hOut, before);  // ��ԭԭ�й��λ��
	if (!BM(pattern, count))
		return false;
	else
		return true;
}

bool vim_operator::Delete()
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO hBuffer;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
	GetConsoleScreenBufferInfo(hOut, &hBuffer);
	COORD position = hBuffer.dwCursorPosition;
	unsigned long num1 = 0;
	int page = position.Y / height;
	int row = position.Y % height;
	if (position.X == 0 && num[page][row] == 0)
		return true;
	char* temp = new char[num[page][row] - hBuffer.dwCursorPosition.X]; //�洢�����Ԫ�أ��ں����������
	temp[num[page][row] - position.X - 1] = 0;  //��֤0��β��ʹ��temp��Ϊ�ַ���
	position.X += 1;
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row] - position.X, position, &num1);//��ȡ����̨��Ļ�����������һ�е�Ԫ��
	position.X -= 1;
	if (Feedback.mod_re == 1)  //Ϊ����������׼��
	{
		Feedback.pre = position;
		ReadConsoleOutputCharacterA(hOut, &Feedback.ch, 1, Feedback.pre, &num1);//��ȡ���ǰ�ĵ�һ��Ԫ�أ�Ϊ���ز�����׼��
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
		num[0][i] = 0;  //��ʼ��ÿһ�е�Ԫ�ظ�����Ϊ0
	page_num = 1;
	row_num = 0; //�����������һ��Ϊ��һ��
	SetConsoleCursorPosition(hOut, { 0,0 });
	return true;
}

bool vim_operator::Insert_cin(char ch)
{
	HANDLE hOut;
	CONSOLE_SCREEN_BUFFER_INFO hBuffer;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
	GetConsoleScreenBufferInfo(hOut, &hBuffer);
	unsigned long num1 = 0;
	COORD position = hBuffer.dwCursorPosition;
	int page = position.Y / height;
	int row = position.Y % height;
	//����Ǹ���find_enter���飬Ϊ���Զ�ת�ж�����
	if (position.X == 0 && !(page == 0 && row == 0)) //��ֹ��һ�г�������
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
	char* temp = new char[num[page][row] + 1 - hBuffer.dwCursorPosition.X]; //�洢�������Ԫ��
	temp[num[page][row] - position.X] = 0;  //��֤0��β��ʹ��temp��Ϊ�ַ���
	ReadConsoleOutputCharacterA(hOut, temp, num[page][row] - position.X, position, &num1);//��ȡ�������Ԫ��
	char temp_one = 0;
	if (num[page][row] - position.X != 0)
	{
		temp_one = temp[num[page][row] - position.X - 1];
		temp[num[page][row] - position.X - 1] = 0;
	}
	cout << ch << temp;
	if (num[page][row] == hBuffer.srWindow.Right + 1) // �ж�������ַ��󣬸������һ���ַ��᲻��ת��
	{
		cout << endl;
		if (position.Y == row_num)
		{
			row_num++;
			if (row_num == page_num * height) //����ܵ����һ��Ϊ���ҳ�����һ�У�����Ҫ�¼�ҳ
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
	position = hBuffer.dwCursorPosition; //����ʱ��Ĺ��λ��
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
	if (state_judge == 'i')  //��������insertģʽ����
	{
		if (mod_re == 1)  //��һ�γ����������γ������������ĳ���
		{
			WriteFile(LastFileName);
			clean();
			OpenFile(TempFileName);
			mod_re = 2;
		}
		else  //�ڶ��Σ�ż���Σ�����������ԭ����
		{
			WriteFile(TempFileName);
			clean();
			OpenFile(LastFileName);
			mod_re = 1;
		}
	}
	else if (state_judge == 'x')  //��������x����ɾ����ǰ�������
	{
		HANDLE hOut;
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(hOut, Feedback.pre);
		if (Feedback.mod_re == 1)  //��һ�γ����������γ������������ĳ���
		{
			Insert_cin(Feedback.ch);
			Feedback.mod_re = 2;
			SetConsoleCursorPosition(hOut, Feedback.pre);
		}
		else  //�ڶ��Σ�ż���Σ�����������ԭ����
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
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
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
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);    // ��ȡ��׼������ 
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
