#pragma once
#include"Huffman.h"
using namespace std;
class menu {
	HuffMan_Tree hft;
public:
	menu() {
		while (1) {
			start_menu();
		}
	}
	void start_menu() {
		cout << "压缩系统" << endl;
		cout << "************" << endl;
		cout << "1.压缩预存文件" << endl;
		cout << "2.解压缩文件" << endl;
		cout << "3.文件压缩率" << endl;
		cout << "4.对比相同性" << endl;
		cout << "************" << endl;
		int a;
		cin >> a;
		func(a);
	}
	void func(int a) {
		switch (a) {
		case 1:hft.ZIP(); break;
		case 2:hft.UNZIP(); break;
		case 3:hft.get_ratio(); system("pause"); break;
		case 4:hft.compare(); system("pause"); break;
		default:exit(1); break;
		}
		system("cls");
	}
	~menu() {

	}
};