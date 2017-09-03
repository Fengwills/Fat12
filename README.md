# Fat12
## 实验目的

设计和实现基于 FAT12 的模拟磁盘卷及其 I/O 系统的文件存取操作基本功能函数，深入 领会和理解文件系统的体系结构、工作原理和设计要领。具体如下：
• 根据 FAT12 设计模拟磁盘（1.44MB 软盘映像文件）的磁盘组织结构及文 件或空闲盘块管理方法与描述用数据结构，并实现模拟磁盘的创建和格 式化操作。
• 构建和提供用户与文件系统之间的基本接口，包括目录和文件的创建、 重命名、删除和显示操作，目录的进入操作以及文件的定位及读、写操 作。
• 所做系统要能够和 Linux1系统兼容，能够交叉使用虚拟机上的 Linux 系统和系统原型接口对所创建的软盘映像文件进行查看和执行文件或目录的各种操作。

## 实验设计

1. 模拟文件系统的操作对象为模拟磁盘，模拟磁盘为软盘映像文件（img 格式）， 在虚拟机（本实验使用VMware）中可以加载，并由 Ubuntu 操作系统进行管 理。
2. Ubuntu系统对 FAT12、FAT16、FAT32 文件系统都支持，要做的模拟文件系统只 是实现了 DOS 功能的一个子集，该子集严格遵循 FAT12 规范，且与 DOS 兼容。
3. 该模拟文件系统基于 C++的文件 I/O 实现，分为两层，下层为核心文件系统， 实现各种命令和对模拟磁盘的操作，其功能封装在 FileSystem 类中；上层为 用户命令接口（终端），实现和用户的交互。
4. 已实现的模拟文件系统命令和 Linux 系统的命令的对应关系如下表。
<table>
<tr>
<td>
  Linux
</td>
<td>
  模拟文件系统
</td>
</tr>
<tr>
<td>
  mkdir path
</td>
<td>
  Mkdir path
</td>
</tr>
<tr>
<td>
  rmdir path
</td>
<td>
  Rmdir path
</td>
</tr>
<tr>
<td>
  touch filename
</td>
<td>
  Crate filename
</td>
</tr>
<tr>
<td>
  Cat 内容 > filename
</td>
<td>
  Write filename 内容
</td>
</tr>
<tr>
<td>
  more filename
</td>
<td>
  Read filename
</td>
</tr>
<tr>
<td>
  rm fileName
</td>
<td>
  Rm filename
</td>
</tr>
</table>
 

另外，模拟文件系统还有创建虚拟磁盘，打开虚拟磁盘，关闭虚拟磁盘和退 出终端的命令。<br>
5. 模拟文件系统支持”.”目录和”..”目录，”.”表示当前目录，”..”表 示上一级目录。<br>
6. 模拟文件系统的路径表示有绝对路径和相对路径两种，用法与 linux系统相同， 与 linux 系统不同的是，模拟文件系统的目录分隔符为’/’。
## 开发平台
操作系统：win10、ubuntu16.04、MSDos7.1<br>
开发工具：viusal  studio 2013 professional、UltraEdit
## 算法及关键数据结构设计

 1.	文件系统底层功能在myDisk.h中，其保存了一些磁盘的参数和对磁盘的操作函数声明，其实现在myDisk.cpp中。功能简介如下表<br>
函数	功能<br>
BOOL StartDisk(LPCTSTR lpszFile);	启动硬盘。该API必须先于所有其他API调用，传入已存在路径下的文件名。<br>
void ShutDisk();	关闭硬盘。该API在完成对文件系统的操作后调用<br>
int ReadDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);	从文件系统中读取指定长度的数据。<br>
int WriteDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);	向虚拟磁盘中写入指定长度的数据<br>
int SetOffset(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);	移动磁头，读写不同位置<br>

2.	文件系统的底层实现在disk.h中，包括引导区结构体、根目录条目结构体、文件句柄结构体以及底层函数声明，功能简介如下表<br>
函数	功能<br>
BOOL initBPB();	读取BPB块<br>
BOOL isFileExist(char *pszFileName, u16 FstClus);	判断文件是否存在<br>
u16 isDirectoryExist(char *FolderName, u16 FstClus);	判断目录是否存在<br>
u16 isPathExist(char *pszFolderPath);	判断路径是否存在 若存在，返回路径最后一个目录的首簇号，若不存在则返回0<br>
u16 getFATValue(u16 FstClus);	创建目录或者文件时初始化信息<br>
void initFileInfo(RootEntry* FileInfo_ptr, char* FileName, u8 FileAttr, u32 FileSize);	创建目录或者文件时初始化信息<br>
BOOL writeEmptyClus(u16 FstClus, RootEntry* FileInfo);	查询可用的簇<br>
u16 setFATValue(int clusNum);	查询可用簇，链接簇链，并初始化目录项<br>
DWORD createHandle(RootEntry* FileInfo, u16 parentClus);	分配句柄<br>
BOOL recoverClus(u16 fileClus);	回收簇<br>
void recursiveDeleteDirectory(u16 fClus);	递归删除文件夹下的东西<br>
fClus保存待删除文件夹首簇<br>
void syncFat12();	同步FAT12<br>


3.	文件系统的操作函数声明在fat12.h中，包括创建文件、打开文件、删除文件、读写文件等操作，功能简介如下表。<br>

函数	功能<br>
DWORD MyCreateFile(char *pszFolderPath, char *pszFileName);	在指定目录下创建指定文件<br>
DWORD MyOpenFile(char *pszFolderPath, char *pszFileName);	打开指定目录下的指定文件<br>
void MyCloseFile(DWORD dwHandle);	关闭该文件<br>
BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName);	删除指定目录下的指定文件<br>
DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);	将pBuffer中dwBytesToWrite长度的数据写入指定文件的文件指针位置<br>
DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);	读取指定文件中、指定长度的数据到传入的缓冲区<br>
BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName);	在指定路径下，创建指定名称的文件夹<br>
BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName);	在指定路径下，删除指定名称的文件夹<br>
BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);	移动指定文件的文件头，读写不同位置<br>

4.  dostime.h里是获得dos日期时间的声明，具体实现在dostime.cpp。<br>

## 源程序清单和说明

由于源代码较长，故此处略去。源程序分为 8 个文件。myDisk.cpp和myDisk.h中是磁盘参数声明和基本操作实现。disk.h中是文件系统底层实现的结构体和公共操作函数。Dostime.h和dostime.cpp是获取dos时间日期实现。Fat12.h和fat12.cpp是创建文件、删除文件等操作的函数实现。Fat12_test.cpp中是main函数

