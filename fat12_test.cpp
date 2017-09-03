#include <iostream>
#include <string>
#include "fat12.h"
using namespace std;

int main() {
	DWORD res;
	DWORD res2;
	string cmd;//命令字符串
	char command[16];//操作的目录或文件名
	char currentdir[80] = {"C:\\"};//当前目录名
	cout << "****************************************" << endl;
	cout << "*                                      *" << endl;
	cout << "*        欢迎进入will的文件系统        *" << endl;
	cout << "*                                      *" << endl;
	cout << "****************************************" << endl;
	cout << "输入help查看支持的命令" << endl;

	while(1){
		
		cout << currentdir;
		cin >> cmd;
		if (cmd == "mkdir"){
			cin >> command;
			// 创建目录
			
			res = MyCreateDirectory(currentdir, command);
			cout << "MyCreateDirectory => return " << res << endl;
			//*/
		}
		else if (cmd=="rmdir"){
			cin >> command;
			// 删除目录
			
			res = MyDeleteDirectory(currentdir, "command");
			cout << "MyDeleteDirectory => return " << res << endl;
			//*/
		}
		else if (cmd == "create"){
			cin >> command;
			// 创建文件
			//

			res = MyCreateFile(currentdir, command);
			cout << "MyCreateFile => return " << res << endl;
			//*/
		}
			else if (cmd == "write"){
			cin >> command;
			// 打开文件

			res = MyOpenFile(currentdir, command);
			cout << "MyOpenFile => return " << res << endl;
			//*/
			// 写文件

			char pBuffer[6] = { "hello" };
			/*
			for (int i = 0; i < 768; i++) {
			pBuffer[i] = 'a';
			}
			*/
			res2 = MyWriteFile(res, &pBuffer, 6);
			cout << "MyWriteFile => return " << res << endl;
			//*/
			// 关闭文件
			
			MyCloseFile(res);
			cout << "MyCloseFile => void"<< endl;
			//*/
		}
		else if (cmd == "read"){
			cin >> command;
			// 打开文件

			res = MyOpenFile(currentdir, command);
			cout << "MyOpenFile => return " << res << endl;
			//*/
			// 读文件

			
			char rBuffer[1025] = { 0 };
			res2 = MyReadFile(res, &rBuffer, 1024);
			cout << "MyReadFile => return " << res << endl;
			cout << "rBuffer => " << rBuffer << endl;
			cout << "rBuffer length => " << strlen(rBuffer) << endl;
			//*/
			// 关闭文件
			
			MyCloseFile(res);
			cout << "MyCloseFile => void"<< endl;
			//*/
		}
		else if (cmd == "rm"){
			cin >> command;
			// 删除文件
			
			res = MyDeleteFile(currentdir, command);
			cout << "MyDeleteFile => return " << res << endl;
			//*/
		}
		else if (cmd == "quit")
		{
			break;
		}
		else if (cmd == "help"){
			cout << "mkdir dirname   创建目录" << endl;
			cout << "rmdir dirname   删除目录" << endl;
			cout << "create filename   创建文件" << endl;
			cout << "read filename   读文件" << endl;
			cout << "write filename   写文件" << endl;
			cout << "rm filename   删除文件" << endl;
			cout << "quit   退出" << endl;
		}
		else cout << "无效指令,请重新输入:" << endl;

	}

	

	
	//*/

	// 关闭文件
	/*
	MyCloseFile(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// 创建文件
	//
	/*
	res = MyCreateFile("c:", "test1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/

	// 移动文件指针
	/*
	MySetFilePointer(res, 1024, MY_FILE_BEGIN);
	res = MySetFilePointer(res, -512, MY_FILE_CURRENT);
	cout << "MySetFilePointer => return " << res << endl;
	//*/
	// 打开文件
	/*
	res = MyOpenFile("c:\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/
	// 写文件
	/*
	char pBuffer[6] = { "hello" };
	/*
	for (int i = 0; i < 768; i++) {
	pBuffer[i] = 'a';
	}
	*/
	/*
	res2 = MyWriteFile(res, &pBuffer, 6);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// 读文件

	/*
	char rBuffer[1025] = { 0 };
	res2 = MyReadFile(res, &rBuffer, 1024);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// 创建目录
	/*
	res = MyCreateDirectory("c:\\huangjun", "qin");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// 删除目录
	/*
	res = MyDeleteDirectory("c:\\", "drafts");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	return 0;
}