#pragma once
#include<queue>
#include<cstring>
#include<fstream>
#include<iostream>
#include<cstdlib>
#include<sys/stat.h>
#include<string>
#include<vector>
using namespace std;
struct File_Node {
	int CODE;
	int CODE_LEN;
	unsigned char DAta;
	File_Node() {
		CODE_LEN = 0;
	}
};
class HuffMan_Node {
public:
	HuffMan_Node* Left_Child;//哈夫曼树的左节点
	HuffMan_Node* Right_Child;//哈夫曼树的右节点
	unsigned char DATA;//哈夫曼树节点的数据
	int Weight;
	bool Is_Leave;
	HuffMan_Node(unsigned char D, int W) {//叶子节点初始化
		Left_Child = nullptr;
		Right_Child = nullptr;
		DATA = D;
		Is_Leave = true;
		Weight = W;
	}
	HuffMan_Node() {//中间节点初始化
		Left_Child = nullptr;
		Right_Child = nullptr;
		DATA = 0;
		Is_Leave = false;
		Weight = 0;
	}
};
struct rule {//queue规则重置
	bool operator()(HuffMan_Node* a, HuffMan_Node* b) {
		return a->Weight > b->Weight;
	}
};
class HuffMan_Tree {
	HuffMan_Node* Head;//哈夫曼树的头节点。
	File_Node wfile[260];//文件节点。
	File_Node unwfile[260];//解压文件节点。
	unsigned char* Buffer;//缓存区，为压缩提供便利。
	unsigned char* UnBuffer;//解压缓存区。
	int buf;
	int buffer_max;
protected:
	void Make_HuffMan_Tree(priority_queue<HuffMan_Node*, vector<HuffMan_Node*>, rule> Huffman_Queue) {
		//两个及以上。
		HuffMan_Node* n1;//先取出两个构造初始节点
		HuffMan_Node* n2;
		HuffMan_Node* nptr;
		while (Huffman_Queue.size() > 1) {
			n1 = Huffman_Queue.top();
			Huffman_Queue.pop();
			n2 = Huffman_Queue.top();
			Huffman_Queue.pop();
			nptr = new HuffMan_Node;//将队前两个进行组织排序
			nptr->Left_Child = n1;
			nptr->Right_Child = n2;
			nptr->Weight = n1->Weight + n2->Weight;//计算新节点权重。
			Huffman_Queue.push(nptr);//将新节点push进队列。
		}
		Head = Huffman_Queue.top();
		Huffman_Queue.pop();
	}

	void Huffman_Coding(HuffMan_Node* p, int code, int code_len) {
		if (p->Is_Leave) {
			wfile[p->DATA].CODE = code;
			wfile[p->DATA].CODE_LEN = code_len;
			wfile[p->DATA].DAta = p->DATA;
		}
		if (p->Left_Child)Huffman_Coding(p->Left_Child, code << 1, code_len + 1);
		if (p->Right_Child)Huffman_Coding(p->Right_Child, ((code << 1) | 1), code_len + 1);
	}

	bool Read_File(string fn) {//读文件，统计字符及其对应频率，创建叶子节点，创建优先队列。
		priority_queue<HuffMan_Node*, vector<HuffMan_Node*>, rule > Huffman_Queue;
		fstream f;
		f.open(fn);
		if (!f.is_open()) {
			cerr << "File Not Found!" << endl;
			return false;
		}
		int element_type[260];//存放元素类型，字节长度为八个比特位，一共就只有256种情况。
		memset(element_type, 0, sizeof(element_type));
		while (!f.eof()) {
			unsigned char letter;
			f >> noskipws >> letter;
			element_type[letter]++;
		}
		for (int i = 0; i < 256; i++) {//对所有字节进行遍历，存入队列中，等待下一步处理。
			if (element_type[i] != 0) {
				HuffMan_Node* p = new HuffMan_Node(unsigned char(i), element_type[i]);
				Huffman_Queue.push(p);
			}
		}
		if (Huffman_Queue.size() == 1) {//如果只有一个节点。
			Head = Huffman_Queue.top();
			Head->Left_Child = nullptr;
			Head->Right_Child = nullptr;
			wfile[Head->DATA].CODE = 0;
			wfile[Head->DATA].CODE_LEN = 1;
			Huffman_Queue.pop();
		}
		else {
			Make_HuffMan_Tree(Huffman_Queue);//相关队列可以清空了
			Huffman_Coding(Head, 0, 1);
		}//顺便编码。
		f.close();
		return true;
	}
	//要变成线性结构，不能进行线索化，会造成哈夫曼树整体复杂性上升。
	//相信计算机内存，即充分利用缓存区内存进行操作，可极大降低编程难度。但会使程序普适化程度降低。
	//树只是形式，code在本树中以int型存储，不用担心发生近似错误。
	void zip(string str) {//只读不输。//将字符对应的哈夫曼编码按位传入缓存区。
		ifstream read;
		read.open(str, ios::binary);//以二进制方式读入。
		while (!read.eof()) {
			unsigned char filedata;
			read >> noskipws >> filedata;
			//将字符对应的编码读出，并将位压入缓存。
			for (int i = wfile[filedata].CODE_LEN - 1; i >= 0; i--) {
				int code = wfile[filedata].CODE;
				wbit(1 & (code >> i));
			}
		}
		read.close();
	}

	void wbit(int i) {//将编码按位压缩。
		Buffer[buffer_max] = Buffer[buffer_max] | (i << buf);
		buf++;
		if (buf >= 8) {
			buf = 0;
			++buffer_max;
			Buffer[buffer_max] = 0;
		}
	}

	void write(string str) {//只输不读
		ofstream out;
		out.open(str);
		out << 'z' << 'i' << 'p' << endl;
		out << buffer_max << ' ' << buf << ' ';
		for (int i = 0; i < 256; i++) {
			if (wfile[i].CODE_LEN > 0) {
				out << int(wfile[i].DAta) << ' ';
				out << wfile[i].CODE << ' ';
				out << wfile[i].CODE_LEN << ' ';
			}
		}
		out << endl;
		out.close();
	}
	void write2(string str) {
		ofstream out;
		out.open(str, ios::binary);
		for (int i = 0; i <= buffer_max; i++) {
			out << Buffer[i];
		}
		out.close();
	}

	void READ_FILE(string str) {
		ifstream in;
		in.open(str);
		unsigned char a;
		in >> a;
		if (a != 'z') {
			cout << "文件格式错误！" << endl;
			return;
		}
		in >> a;
		if (a != 'i') {
			cout << "文件格式错误！" << endl;
			return;
		}
		in >> a;
		if (a != 'p') {
			cout << "文件格式错误！" << endl;
			return;
		}
		//以上，判断文件格式。
		in >> buffer_max;
		in >> buf;
		//cout << buf << endl;
		int data = 0;
		int uncode = 0;
		int uncode_len = 0;
		while (!in.eof()) {
			in >> data >> uncode >> uncode_len;
			unwfile[data].DAta = unsigned char(data);
			unwfile[data].CODE = uncode;
			unwfile[data].CODE_LEN = uncode_len;
		}
		in.close();
	}
	void READ2(string str) {
		ifstream in;
		in.open(str, ios::binary);
		int i = 0;
		while (!in.eof()) {
			in >> noskipws >> UnBuffer[i++];
		}
		in.close();
	}
	void unzip(string str) {
		int cscs = 0;
		ofstream out;
		out.open(str, ios::binary);
		int temp = 0;
		int i = 0;
		int c = 0;
		int len = 0;
		int linshi;
		while (temp < buffer_max || (temp == buffer_max && i < buf)) {
			cscs++;
			c = c << 1;
			c |= 1 & (UnBuffer[temp] >> i);
			linshi = 1 & (UnBuffer[temp] >> i);
			++i;
			if (i >= 8) {
				i = 0;
				++temp;
			}
			len++;
			for (int j = 0; j < 256; j++) {
				if (len == unwfile[j].CODE_LEN) {
					if (c == unwfile[j].CODE) {
						len = 0;
						c = 0;
						out << unwfile[j].DAta;
						break;
					}
				}
			}
		}
		out.close();
	}
public:
	
	HuffMan_Tree() {
		Head = nullptr;
		Buffer = new unsigned char[1024 * 1024 * 10];
		UnBuffer = new unsigned char[1024 * 1024 * 10];
		memset(UnBuffer, 0, sizeof(UnBuffer));
		memset(Buffer, 0, sizeof(Buffer));
		buf = 0;
		buffer_max = 0;
	}

	~HuffMan_Tree() {
		delete[] Buffer;
		delete[] UnBuffer;
	}
	size_t GFS(string& str) {
		struct _stat info;
		_stat(str.c_str(), &info);
		size_t size = info.st_size;
		return size;
	}
	void get_ratio() {
		size_t size1, size2;
		string str1, str2;
		str1 = "F:\\programe\\C++_Programes\\data_structuer\\data\\Eng.txt";
		str2 = "F:\\programe\\C++_Programes\\data_structuer\\data\\in3.txt";
		size1 = GFS(str1);
		size2 = GFS(str2);
		double a = size1;
		double b = size2;
		cout << "压缩率约为" <<  (b / a)*100 << "%" << endl;
	}

	void compare() {
		ifstream write;
		write.open("F:\\programe\\C++_Programes\\data_structuer\\data\\3new.txt");
		ifstream out2;
		out2.open("F:\\programe\\C++_Programes\\data_structuer\\data\\Eng.txt");
		char a1;
		char a2;
		bool flag = true;
		int i = 0;
		int j = 0;
		while (1) {
			if (!write.eof()) {
				write >> a1; i++;
			}
			else {
				break;
			}
			if (!out2.eof()) {
				out2 >> a2; j++;
			}
			else {
				break;
			}
			if (a1 != a2) {
				cout << "wrong!" << "  i:" << i << "  j:" << j << "  压前:" << a1 << "  压后:" << a2 << endl;
				flag = false;
			}
			write.close();
			out2.close();
		}
		if (flag) {
			cout << "完全一致！" << endl;
		}
		return;
	}
	void ZIP() {
		Read_File("F:\\programe\\C++_Programes\\data_structuer\\data\\Eng.txt");
		zip("F:\\programe\\C++_Programes\\data_structuer\\data\\Eng.txt");
		write("F:\\programe\\C++_Programes\\data_structuer\\data\\config.txt");
		write2("F:\\programe\\C++_Programes\\data_structuer\\data\\in3.txt");
	}
	void UNZIP() {
		READ_FILE("F:\\programe\\C++_Programes\\data_structuer\\data\\config.txt");
		READ2("F:\\programe\\C++_Programes\\data_structuer\\data\\in3.txt");
		unzip("F:\\programe\\C++_Programes\\data_structuer\\data\\3new.txt");
	}
};
