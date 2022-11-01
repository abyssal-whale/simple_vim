#pragma once
#include<iostream>
#include<Windows.h>
#include<string.h>
#include<stdlib.h>
#include <conio.h>

class vim_operator {
private:
	//normalģʽ�µĺ�����
	bool normal_function();	//normalģʽ���ܱ���
	bool lastline();	//��ָ����������һ��
	bool withdraw();	//ִ������u
	bool Delete();	//ִ������x
	bool NormalMove(const char& a);	//normal���λ���ƶ�
	bool OpenFile(const char* filename);	//���ļ�
	bool WriteFile(const char* filename);	//д���ļ�
	bool search();	//ִ����������
	bool BM(char* t, const int m);	//����BM�㷨ƥ���ַ���
	bool clean();	//�������̨ҳ���е���������

	//insertģʽ�µĺ�����
	bool insert_function();	//insertģʽ���ܱ���
	bool InsertMove();	//insert���λ���ƶ�
	bool grow();	//��������ʱ������
	bool Insert_cin(char ch);	//insertģʽ�µ��ַ��Ĳ���

	//bonus
	bool pageup();	//���Ϸ�ҳ
	bool pagedown();	//���·�ҳ

	//���õ����ݺͽṹ��
	char mode;	//��¼ģʽ
	int height;  //��¼���ڸ߶�
	int row_num;  //��¼����һ�е��к�
	int page_num;  //ҳ�������ֵ
	const char* TempFileName = { "tempfile.txt" };  //��ʱ�ļ������ƣ�����������
	const char* LastFileName = { "lastfile.txt" };
	int** num;  //�洢ÿһ�е�Ԫ�ظ���,����ά�ȷֱ���ҳ��������
	int** find_enter;  //��¼ÿһ�еĵĻس����ǲ����ڱ��У���Ϊ����һ��ʱ�����ڱ������룬�����������룬����Ҫ�س�
	int mod_re; // �ж��ǳ����������Ƿ���������
	char state_judge;  //�жϳ����ǳ���insert����x
	struct feedback//������¼����ǰ�Ĺ��λ�á��ַ����ǳ������Ƿ�����
	{
		COORD pre = { 0,0 };
		char ch = 0;
		int mod_re = 1;
	} Feedback;

public:
	bool judge();	//�����ж�������һ��ģʽ
	void renew();	//����Ŀǰ���λ��
	vim_operator() //���캯��
	{
		mode = 'n';//Ĭ��ģʽΪnormalģʽ
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO pBuffer;
		GetConsoleScreenBufferInfo(hStdout, &pBuffer);
		height = pBuffer.srWindow.Bottom - pBuffer.srWindow.Top + 1;
		num = new int* [100];
		find_enter = new int* [100];
		num[0] = new int[height];  //һҳ
		find_enter[0] = new int[height];
		page_num = 1;
		row_num = 0; //�����������һ��Ϊ��һ��
		mod_re = 1;
		//which_back = 0; //0��ʾû�г��أ�1��ʾ֮ǰ����Ϊinsert��2��ʾ֮ǰ����Ϊ/x
		for (int i = 0; i < height; i++)
		{
			num[0][i] = 0;  //��ʼ��ÿһ�е�Ԫ�ظ�����Ϊ0
			find_enter[0][i] = 0; //0��ʾ����Ϊ��ͷ��������ת����ʱ��Ҫ�س�
		}
	}
};


