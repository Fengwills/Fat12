#include <iostream>
#include <string>
#include "fat12.h"
using namespace std;

int main() {
	DWORD res;
	DWORD res2;
	string cmd;//�����ַ���
	char command[16];//������Ŀ¼���ļ���
	char currentdir[80] = {"C:\\"};//��ǰĿ¼��
	cout << "****************************************" << endl;
	cout << "*                                      *" << endl;
	cout << "*        ��ӭ����will���ļ�ϵͳ        *" << endl;
	cout << "*                                      *" << endl;
	cout << "****************************************" << endl;
	cout << "����help�鿴֧�ֵ�����" << endl;

	while(1){
		
		cout << currentdir;
		cin >> cmd;
		if (cmd == "mkdir"){
			cin >> command;
			// ����Ŀ¼
			
			res = MyCreateDirectory(currentdir, command);
			cout << "MyCreateDirectory => return " << res << endl;
			//*/
		}
		else if (cmd=="rmdir"){
			cin >> command;
			// ɾ��Ŀ¼
			
			res = MyDeleteDirectory(currentdir, "command");
			cout << "MyDeleteDirectory => return " << res << endl;
			//*/
		}
		else if (cmd == "create"){
			cin >> command;
			// �����ļ�
			//

			res = MyCreateFile(currentdir, command);
			cout << "MyCreateFile => return " << res << endl;
			//*/
		}
			else if (cmd == "write"){
			cin >> command;
			// ���ļ�

			res = MyOpenFile(currentdir, command);
			cout << "MyOpenFile => return " << res << endl;
			//*/
			// д�ļ�

			char pBuffer[6] = { "hello" };
			/*
			for (int i = 0; i < 768; i++) {
			pBuffer[i] = 'a';
			}
			*/
			res2 = MyWriteFile(res, &pBuffer, 6);
			cout << "MyWriteFile => return " << res << endl;
			//*/
			// �ر��ļ�
			
			MyCloseFile(res);
			cout << "MyCloseFile => void"<< endl;
			//*/
		}
		else if (cmd == "read"){
			cin >> command;
			// ���ļ�

			res = MyOpenFile(currentdir, command);
			cout << "MyOpenFile => return " << res << endl;
			//*/
			// ���ļ�

			
			char rBuffer[1025] = { 0 };
			res2 = MyReadFile(res, &rBuffer, 1024);
			cout << "MyReadFile => return " << res << endl;
			cout << "rBuffer => " << rBuffer << endl;
			cout << "rBuffer length => " << strlen(rBuffer) << endl;
			//*/
			// �ر��ļ�
			
			MyCloseFile(res);
			cout << "MyCloseFile => void"<< endl;
			//*/
		}
		else if (cmd == "rm"){
			cin >> command;
			// ɾ���ļ�
			
			res = MyDeleteFile(currentdir, command);
			cout << "MyDeleteFile => return " << res << endl;
			//*/
		}
		else if (cmd == "quit")
		{
			break;
		}
		else if (cmd == "help"){
			cout << "mkdir dirname   ����Ŀ¼" << endl;
			cout << "rmdir dirname   ɾ��Ŀ¼" << endl;
			cout << "create filename   �����ļ�" << endl;
			cout << "read filename   ���ļ�" << endl;
			cout << "write filename   д�ļ�" << endl;
			cout << "rm filename   ɾ���ļ�" << endl;
			cout << "quit   �˳�" << endl;
		}
		else cout << "��Чָ��,����������:" << endl;

	}

	

	
	//*/

	// �ر��ļ�
	/*
	MyCloseFile(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// �����ļ�
	//
	/*
	res = MyCreateFile("c:", "test1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/

	// �ƶ��ļ�ָ��
	/*
	MySetFilePointer(res, 1024, MY_FILE_BEGIN);
	res = MySetFilePointer(res, -512, MY_FILE_CURRENT);
	cout << "MySetFilePointer => return " << res << endl;
	//*/
	// ���ļ�
	/*
	res = MyOpenFile("c:\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/
	// д�ļ�
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

	// ���ļ�

	/*
	char rBuffer[1025] = { 0 };
	res2 = MyReadFile(res, &rBuffer, 1024);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// ����Ŀ¼
	/*
	res = MyCreateDirectory("c:\\huangjun", "qin");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// ɾ��Ŀ¼
	/*
	res = MyDeleteDirectory("c:\\", "drafts");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	return 0;
}