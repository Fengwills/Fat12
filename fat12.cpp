#include "fat12.h"
#include "myDisk.h"
#include <string.h>
#pragma comment(lib, "DiskLib.lib")
using namespace std;

int  BytsPerSec;    //ÿ�����ֽ���
int  SecPerClus;    //ÿ��������  
int  RsvdSecCnt;    //Boot��¼ռ�õ�������
int  NumFATs;   //FAT������  
int  RootEntCnt;    //��Ŀ¼����ļ���  
int  TotSec; //��������
int  FATSz; // FAT������

const char* fs = "me.img";

FileHandle* dwHandles[MAX_NUM] = { NULL };

u8* setzero = (u8*)calloc(512, sizeof(u8)); // ���ڴ���Ŀ¼ʱ��0

BPB bpb;
BPB* bpb_ptr = &bpb;



RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;

DWORD MyCreateFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	if (strlen(pszFileName) > 12 || strlen(pszFileName) < 3) return 0; // � 8+3+'.' ;��� 1+'.'+1
	u16 FstClus;
	u32 FileSize = 0; // ��ʼֵΪ0
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	memset(FileInfo_ptr, 0, sizeof(RootEntry));
	if (initBPB()) {
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) <= 3) {
			if (isFileExist(pszFileName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFileName << " has existed!" << endl;
			}
			else {
				initFileInfo(FileInfo_ptr, pszFileName, 0x20, FileSize);
				if (FileInfo_ptr->DIR_FstClus == 0) return 0; // �����ʧ��
				if (writeEmptyClus(FstClus, FileInfo_ptr) == TRUE) {
					// �������
					FileHandle = createHandle(FileInfo_ptr, FstClus);
				}
			}
		}
	}
	if (FileHandle != 0) syncFat12();
	ShutDisk();
	return FileHandle;
}

DWORD MyOpenFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	if (strlen(pszFileName) > 12 || strlen(pszFileName) < 3) return 0; // 8+3+'.'
	u16 FstClus = 0;
	BOOL isExist = FALSE;
	char filename[13];
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		// fix bug:���ȼ�����ֵ�ǵü�����
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) <= 3) {
			u16 parentClus = FstClus;
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					int loop;
					if (FstClus == 0) {
						// ��Ŀ¼��ƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// �������ļ���ַƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						//SetOffset(dataBase, NULL, FILE_BEGIN);
						SetOffset(dataBase, NULL, FILE_BEGIN);
						//if (ReadDisk(FileInfo_ptr, 32, NULL) != 0) {
						if (ReadDisk(FileInfo_ptr, 32, NULL) != 0) {
							// Ŀ¼0x10���ļ�0x20������0x28
							if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E) {
								int len_of_filename = 0;
								if (FileInfo_ptr->DIR_Attr == 0x20) {
									for (int j = 0; j < 11; j++) {
										if (FileInfo_ptr->DIR_Name[j] != ' ') {
											filename[len_of_filename++] = FileInfo_ptr->DIR_Name[j];
										}
										else {
											filename[len_of_filename++] = '.';
											while (FileInfo_ptr->DIR_Name[j] == ' ') j++;
											j--;
										}
									}
									filename[len_of_filename] = '\0';
									// ���Դ�Сд�Ƚ�
									if (_stricmp(filename, pszFileName) == 0) {
										isExist = TRUE;
										break;
									}
								}
							}
						}
						dataBase += 32;
					}
					if (isExist) {
						FileHandle = createHandle(FileInfo_ptr, parentClus);
						break;
					}
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	//ShutDisk();
	ShutDisk();
	return FileHandle;
}

void MyCloseFile(DWORD dwHandle) {
	if (dwHandles[dwHandle] != NULL) {
		free(dwHandles[dwHandle]);
		dwHandles[dwHandle] = NULL;
	}
}

BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName) {
	BOOL result = FALSE;
	if (strlen(pszFileName) > 12 || strlen(pszFileName) < 3) return FALSE; // 8+3+'.'
	u16 FstClus;
	char filename[13];
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) <= 3) {
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					int loop;
					if (FstClus == 0) {
						// ��Ŀ¼��ƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// �������ļ���ַƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						//SetOffset(dataBase, NULL, FILE_BEGIN);
						SetOffset(dataBase, NULL, FILE_BEGIN);
						//if (ReadDisk(FileInfo_ptr, 32, NULL) != 0) {
						if (ReadDisk(FileInfo_ptr, 32, NULL) != 0) {
							// Ŀ¼0x10���ļ�0x20������0x28
							if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E) {
								int len_of_filename = 0;
								if (FileInfo_ptr->DIR_Attr == 0x20) {
									for (int j = 0; j < 11; j++) {
										if (FileInfo_ptr->DIR_Name[j] != ' ') {
											filename[len_of_filename++] = FileInfo_ptr->DIR_Name[j];
										}
										else {
											filename[len_of_filename++] = '.';
											while (FileInfo_ptr->DIR_Name[j] == ' ') j++;
											j--;
										}
									}
									filename[len_of_filename] = '\0';
									// ���Դ�Сд�Ƚ�
									if (_stricmp(filename, pszFileName) == 0) {
										// �����ȡ��32�ֽڣ���λһ�£���һ�ֽ�д��0xe5
										//SetOffset(dataBase, NULL, FILE_BEGIN);
										SetOffset(dataBase, NULL, FILE_BEGIN);
										u8 del = 0xE5;
										//if (WriteDisk(&del, 1, NULL) != 0) {
										if (WriteDisk(&del, 1, NULL) != 0) {
											// ���մ�
											u16 fileClus = FileInfo_ptr->DIR_FstClus; // �״�
											u16 bytes;
											u16* bytes_ptr = &bytes;
											// ��һ��Ϊĩβ���˳�ѭ��
											while (fileClus != 0xFFF) {
												int clusBase = RsvdSecCnt * BytsPerSec + fileClus * 3 / 2;
												u16 tempClus = getFATValue(fileClus); // �ݴ���һ�أ���ǰ������ˢ�³�0
												//SetOffset(clusBase, NULL, FILE_BEGIN);
												SetOffset(clusBase, NULL, FILE_BEGIN);
												//if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
												if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
													if (fileClus % 2 == 0) {
														bytes = bytes >> 12;
														bytes = bytes << 12; // ��12λ��0
													}
													else {
														bytes = bytes << 12;
														bytes = bytes >> 12; // ��12λ��0
													}
													//SetOffset(clusBase, NULL, FILE_BEGIN);
													SetOffset(clusBase, NULL, FILE_BEGIN);
													WriteDisk(bytes_ptr, 2, NULL); // д�أ����ոô�
												}
												fileClus = tempClus; // ����ƫ����
											}
											result = TRUE;
											break;
										}
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	if (result) syncFat12();
	ShutDisk();
	return result;
}

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // �ļ�ָ�뵱ǰƫ��
	int curClusNum = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int curClusOffset = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (curClusNum) {
		if (getFATValue(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// ��ȡ��ǰָ����ָ����
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // �õ��ļ�ָ����ָλ��
	int lenOfBuffer = dwBytesToWrite; // ��������д�볤��
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // ���ƹ���
	//SetOffset(dataOffset, NULL, FILE_BEGIN);
	SetOffset(dataOffset, NULL, FILE_BEGIN);
	if ((BytsPerSec - curClusOffset >= lenOfBuffer) && curClusNum == 0) {
		//if (WriteDisk(pBuffer, lenOfBuffer, &result) == 0) {
		if (WriteDisk(pBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		u16 tempClus;
		u16 bytes; // ÿ�ζ�ȡ�Ĵغ�
		u16* bytes_ptr = &bytes;
		int fatBase = RsvdSecCnt * BytsPerSec;
		int leftLen = lenOfBuffer;
		int hasWritten = 0;
		if (curClusNum == 0) {
			//if (WriteDisk(pBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
			if (WriteDisk(pBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
				return -1;
			}
			result += temp; // ��¼д�볤��
			leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // ʣ�೤��
			hasWritten = BytsPerSec - curClusOffset;
		}
		do {
			tempClus = getFATValue(FstClus); // ��������һ��FAT
			if (tempClus == 0xFFF) {
				tempClus = setFATValue(1);
				if (tempClus == 0) return -1; //�����ʧ��
				//SetOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
				SetOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
				//if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
				if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
					if (FstClus % 2 == 0) {
						bytes = bytes >> 12;
						bytes = bytes << 12; // ��������λ����12λΪ0
						bytes = bytes | tempClus;
					}
					else {
						bytes = bytes << 12;
						bytes = bytes >> 12; // ��������λ����12λΪ0
						bytes = bytes | (tempClus << 4);
					}
					//SetOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					SetOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					//if (WriteDisk(bytes_ptr, 2, NULL) == 0) {
					if (WriteDisk(bytes_ptr, 2, NULL) == 0) {
						return -1;
					}
				}
			}
			FstClus = tempClus; // �����õ���һ��FAT
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // ˢ������ƫ��
			//SetOffset(dataBase, NULL, FILE_BEGIN); // һ���Ǵ�����ͷ��ʼд
			SetOffset(dataBase, NULL, FILE_BEGIN); // һ���Ǵ�����ͷ��ʼд
			if (leftLen > BytsPerSec) {
				//if (WriteDisk(&cBuffer[hasWritten], BytsPerSec, &temp) == 0) {
				if (WriteDisk(&cBuffer[hasWritten], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasWritten += BytsPerSec;
			}
			else {
				//if (WriteDisk(&cBuffer[hasWritten], leftLen, &temp) == 0) {
				if (WriteDisk(&cBuffer[hasWritten], leftLen, &temp) == 0) {
					return -1;
				}
				hasWritten += leftLen;
			}
			leftLen -= BytsPerSec;
			result += temp;
		} while (leftLen > 0);
	}
	// ˢ���ļ���С
	if ((offset + result) > hd->fileInfo.DIR_FileSize) {
		int dBase;
		BOOL isExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + result) - hd->fileInfo.DIR_FileSize;
		// ������ǰĿ¼������Ŀ
		u16 parentClus = hd->parentClus;
		do {
			int loop;
			if (parentClus == 0) {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
				loop = RootEntCnt;
			}
			else {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (parentClus - 2) * BytsPerSec;
				loop = BytsPerSec / 32;
			}
			for (int i = 0; i < loop; i++) {
				//SetOffset(dBase, NULL, FILE_BEGIN);
				SetOffset(dBase, NULL, FILE_BEGIN);
				//if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
				if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
					if (rootEntry_ptr->DIR_Attr == 0x20) {
						if (_stricmp(rootEntry_ptr->DIR_Name, hd->fileInfo.DIR_Name) == 0) {
							//SetOffset(dBase, NULL, FILE_BEGIN);
							SetOffset(dBase, NULL, FILE_BEGIN);
							//WriteDisk(&hd->fileInfo, 32, NULL);
							WriteDisk(&hd->fileInfo, 32, NULL);
							isExist = TRUE;
							break;
						}
					}
				}
				dBase += 32;
			}
			if (isExist) break;
		} while ((parentClus = getFATValue(parentClus)) != 0xFFF && parentClus != 0);
		if (isExist) syncFat12();
	}
	//ShutDisk();
	ShutDisk();
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //ƫ����ˢ��
	return result;
}

DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // �ļ�ָ�뵱ǰƫ��
	int curClusNum = offset / BytsPerSec; // ��ǰָ���ڵڼ�������
	int curClusOffset = offset % BytsPerSec; // ��ǰ��������ƫ��
	while (curClusNum) {
		if (getFATValue(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// ��ȡ��ǰָ����ָ����
	if (curClusNum > 0 || offset > (int)hd->fileInfo.DIR_FileSize) return -1; // �����ļ�ƫ�Ʒ�Χ��
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // �õ��ļ�ָ����ָλ��
	int lenOfBuffer = dwBytesToRead; // �����������볤��
	if ((int)hd->fileInfo.DIR_FileSize - offset < lenOfBuffer) {
		lenOfBuffer = hd->fileInfo.DIR_FileSize - offset;
	}
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer); // ����һ��������
	memset(cBuffer, 0, lenOfBuffer);
	//SetOffset(dataOffset, NULL, FILE_BEGIN);
	SetOffset(dataOffset, NULL, FILE_BEGIN);
	// ��ȡ
	if (BytsPerSec - curClusOffset >= lenOfBuffer) {
		//if (ReadDisk(cBuffer, lenOfBuffer, &result) == 0) {
		if (ReadDisk(cBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		//if (ReadDisk(cBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
		if (ReadDisk(cBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
			return -1;
		}
		result += temp; // ��¼��ȡ���ĳ���
		int leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // ʣ�೤��
		int hasRead = BytsPerSec - curClusOffset;
		do {
			FstClus = getFATValue(FstClus); // �õ���һ��FAT
			if (FstClus == 0xFFF) {
				break;
			}
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // ˢ������ƫ��
			//SetOffset(dataBase, NULL, FILE_BEGIN);
			SetOffset(dataBase, NULL, FILE_BEGIN);
			if (leftLen > BytsPerSec) {
				//if (ReadDisk(&cBuffer[hasRead], BytsPerSec, &temp) == 0) {
				if (ReadDisk(&cBuffer[hasRead], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasRead += BytsPerSec;
			}
			else {
				//if (ReadDisk(&cBuffer[hasRead], leftLen, &temp) == 0) {
				if (ReadDisk(&cBuffer[hasRead], leftLen, &temp) == 0) {
					return -1;
				}
				hasRead += leftLen;
			}
			leftLen -= BytsPerSec; // ֱ�Ӽ���һ��������ֻҪ��<=0���˳�ѭ��
			result += temp;
		} while (leftLen > 0);
	}
	memcpy(pBuffer, cBuffer, lenOfBuffer); // д�뻺����
	//ShutDisk();
	ShutDisk();
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //ƫ����ˢ��
	return result;
}

BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	u16 originClus;
	BOOL result = FALSE;
	if (strlen(pszFolderName) > 11 || strlen(pszFolderName) <= 0) return FALSE;
	int dataBase;
	if (initBPB()) {
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) <= 3) {
			originClus = FstClus;
			if (isDirectoryExist(pszFolderName, FstClus)) {
				//cout << pszFolderPath << '\\' << pszFolderName << " has existed!" << endl;
			}
			else {
				do {
					int loop;
					if (FstClus == 0) {
						// ��Ŀ¼��ƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// �������ļ���ַƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						//SetOffset(dataBase, NULL, FILE_BEGIN);
						SetOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
							// Ŀ¼�����
							if (rootEntry_ptr->DIR_Name[0] == 0x00 || rootEntry_ptr->DIR_Name[0] == 0xE5) {
								initFileInfo(rootEntry_ptr, pszFolderName, 0x10, 0); // �ļ��д�СΪ0
								if (rootEntry_ptr->DIR_FstClus == 0) return FALSE;
								SetOffset(dataBase, NULL, FILE_BEGIN); // ��ͷ��λ
								if (WriteDisk(rootEntry_ptr, 32, NULL) != 0) {
									// ���� . �� ..Ŀ¼
									int dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (rootEntry_ptr->DIR_FstClus - 2) * BytsPerSec;
									SetOffset(dBase, NULL, FILE_BEGIN);
									WriteDisk(setzero, BytsPerSec, NULL); // Ŀ¼������ʼ��0
									// .
									SetOffset(dBase, NULL, FILE_BEGIN);
									rootEntry_ptr->DIR_FileSize = 0;
									rootEntry_ptr->DIR_Name[0] = 0x2E;
									for (int i = 1; i < 11; i++) {
										rootEntry_ptr->DIR_Name[i] = 0x20;
									}
									WriteDisk(rootEntry_ptr, 32, NULL);
									// ..
									SetOffset(dBase + 32, NULL, FILE_BEGIN);
									rootEntry_ptr->DIR_Name[1] = 0x2E;
									rootEntry_ptr->DIR_FstClus = originClus;
									WriteDisk(rootEntry_ptr, 32, NULL);
									result = TRUE;
									break;
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	if (result) syncFat12();
	ShutDisk();
	return result;
}

BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	BOOL result = FALSE;
	if (strlen(pszFolderName) > 11 || strlen(pszFolderName) <= 0) return FALSE;
	if (initBPB()) {
		// ·�����ڻ���Ϊ��Ŀ¼
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) <= 3) {
			// ��ɾ��Ŀ¼����
			if (isDirectoryExist(pszFolderName, FstClus)) {
				int dataBase;
				int loop;
				char directory[12];
				u8 del = 0xE5;
				RootEntry fd;
				RootEntry* fd_ptr = &fd;
				do {
					if (FstClus == 0) {
						// ��Ŀ¼��ƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// �������ļ���ַƫ��
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadDisk(fd_ptr, 32, NULL) != 0) {
							if (fd_ptr->DIR_Name[0] != 0xE5 && fd_ptr->DIR_Name[0] != 0 && fd_ptr->DIR_Name[0] != 0x2E) {
								// Ŀ¼0x10���ļ�0x20������0x28
								if (fd_ptr->DIR_Attr == 0x10) {
									for (int j = 0; j < 11; j++) {
										if (fd_ptr->DIR_Name[j] != ' ') {
											directory[j] = fd_ptr->DIR_Name[j];
											if (j == 10) {
												directory[11] = '\0';
												break;
											}
										}
										else {
											directory[j] = '\0';
											break;
										}
									}
									// ���Դ�Сд�Ƚ�
									if (_stricmp(directory, pszFolderName) == 0) {
										recursiveDeleteDirectory(fd_ptr->DIR_FstClus);
										// ɾ�����ļ���
										SetOffset(dataBase, NULL, FILE_BEGIN);
										if (WriteDisk(&del, 1, NULL) != 0) {
											result = recoverClus(fd_ptr->DIR_FstClus); // �����״أ�����
											break;
										}
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	if (result) syncFat12();
	ShutDisk();
	return result;
}

BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle* hd = dwHandles[dwFileHandle];
	if (hd == NULL || initBPB() == FALSE) return FALSE; // ���������
	LONG curOffset = nOffset + hd->offset; // currentģʽ��ƫ�ƺ��λ��
	u16 currentClus = hd->fileInfo.DIR_FstClus; // �״�
	int fileSize = hd->fileInfo.DIR_FileSize; // �ļ���С
	int fileBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (currentClus - 2) * BytsPerSec;
	switch (dwMoveMethod) {
	case MY_FILE_BEGIN:
		if (nOffset < 0) {
			hd->offset = 0; // С��0����Ϊ0
		}
		else if (nOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = nOffset;
		}
		break;
	case MY_FILE_CURRENT:
		if (curOffset < 0) {
			hd->offset = 0;
		}
		else if (curOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = curOffset;
		}
		break;
	case MY_FILE_END:
		if (nOffset > 0) {
			hd->offset = fileSize;
		}
		else if (nOffset < -fileSize) {
			hd->offset = 0;
		}
		else {
			hd->offset = fileSize + nOffset;
		}
		break;
	}
	ShutDisk();
	return TRUE;
}

BOOL initBPB() {
	/*
		const char [] תWCHAR����
	*/
	WCHAR wszClassName[256];
	memset(wszClassName, 0, sizeof(wszClassName));
	MultiByteToWideChar(CP_ACP, 0, fs, strlen(fs) + 1, wszClassName,
		sizeof(wszClassName) / sizeof(wszClassName[0]));
	/*
		��ʼ��BPB
	
	bpb_ptr->BPB_BytsPerSec = 512;
	bpb_ptr->BPB_SecPerClus = 1;
	bpb_ptr->BPB_RsvdSecCnt = 1;
	bpb_ptr->BPB_NumFATs = 2;
	bpb_ptr->BPB_RootEntCnt = 224;
	bpb_ptr->BPB_TotSec16 = 2880;
	bpb_ptr->BPB_Media = 0xF0;
	bpb_ptr->BPB_FATSz16 = 9;
	bpb_ptr->BPB_NumHeads = 2;
	bpb_ptr->BPB_SecPerTrk = 18;
	bpb_ptr->BPB_HiddSec = 0;
	*/

	//if (StartupDisk(wszClassName)) {
	if (StartDisk(wszClassName)) {
		//cout << "start up disk..." << endl;
		//����BPB,ƫ��11���ֽڶ�ȡ
		//SetOffset(11, NULL, FILE_BEGIN);
		SetOffset(11, NULL, FILE_BEGIN);
		//if (ReadDisk(bpb_ptr, 25, NULL) != 0) {
		if (ReadDisk(bpb_ptr, 25, NULL) != 0) {
			//��ʼ������ȫ�ֱ���  
			BytsPerSec = bpb_ptr->BPB_BytsPerSec;
			SecPerClus = bpb_ptr->BPB_SecPerClus;
			RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
			NumFATs = bpb_ptr->BPB_NumFATs;
			RootEntCnt = bpb_ptr->BPB_RootEntCnt;
			if (bpb_ptr->BPB_TotSec16 != 0) {
				TotSec = bpb_ptr->BPB_TotSec16;
			}
			else {
				TotSec = bpb_ptr->BPB_TotSec32;
			}
			FATSz = bpb_ptr->BPB_FATSz16;
			/*
			cout << "ÿ�����ֽ�����" << BytsPerSec << endl; // 512
			cout << "ÿ����������" << SecPerClus << endl; // 1
			cout << "Boot��¼ռ�õ���������" << RsvdSecCnt << endl; // 1
			cout << "FAT��������" << NumFATs << endl; // 2
			cout << "��Ŀ¼����ļ�����" << RootEntCnt << endl; // 224
			cout << "����������" << TotSec << endl; // 2880
			cout << "ÿFAT��������" << FATSz << endl; // 9
			*/
			return TRUE;
		}
		else {
			//cout << "read BPB fail..." << endl;
		}
	}
	else {
		//cout << "cannot start up disk..." << endl;
	}
	return FALSE;
}

BOOL isFileExist(char *pszFileName, u16 FstClus) {
	char filename[13];
	int dataBase;
	BOOL isExist = FALSE;
	// ������ǰĿ¼������Ŀ
	do {
		int loop;
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			//SetOffset(dataBase, NULL, FILE_BEGIN);
			SetOffset(dataBase, NULL, FILE_BEGIN);
			//if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
			if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20������0x28
				if (rootEntry_ptr->DIR_Name[0] != 0xE5 && rootEntry_ptr->DIR_Name[0] != 0 && rootEntry_ptr->DIR_Name[0] != 0x2E) {
					int len_of_filename = 0;
					if (rootEntry_ptr->DIR_Attr == 0x20) {
						for (int j = 0; j < 11; j++) {
							if (rootEntry_ptr->DIR_Name[j] != ' ') {
								filename[len_of_filename++] = rootEntry_ptr->DIR_Name[j];
							}
							else {
								filename[len_of_filename++] = '.';
								while (rootEntry_ptr->DIR_Name[j] == ' ') j++;
								j--;
							}
						}
						filename[len_of_filename] = '\0';
						// ���Դ�Сд�Ƚ�
						if (_stricmp(filename, pszFileName) == 0) {
							isExist = TRUE;
							break;
						}
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
	return isExist;
}

u16 isDirectoryExist(char *FolderName, u16 FstClus) {
	char directory[12];
	int dataBase;
	u16 isExist = 0;
	// ������ǰĿ¼������Ŀ
	do {
		int loop;
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20������0x28
				if (rootEntry_ptr->DIR_Name[0] != 0xE5 && rootEntry_ptr->DIR_Name[0] != 0 && rootEntry_ptr->DIR_Name[0] != 0x2E) {
					if (rootEntry_ptr->DIR_Attr == 0x10) {
						for (int j = 0; j < 11; j++) {
							if (rootEntry_ptr->DIR_Name[j] != ' ') {
								directory[j] = rootEntry_ptr->DIR_Name[j];
								if (j == 10) {
									directory[11] = '\0';
									break;
								}
							}
							else {
								directory[j] = '\0';
								break;
							}
						}
						// ���Դ�Сд�Ƚ�
						if (_stricmp(directory, FolderName) == 0) {
							isExist = rootEntry_ptr->DIR_FstClus;
							break;
						}
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
	return isExist;
}

u16 isPathExist(char *pszFolderPath) {
	char directory[12]; // ���Ŀ¼��
	u16 FstClus = 0;
	if (strlen(pszFolderPath) <= 3) return 0;
	/* ��3��ʼ�������̷�C:\\ */
	int i = 3, len = 0;
	// ���ñ߽�ֵi<11
	while (pszFolderPath[i] != '\0' && len <= 11) {
		if (pszFolderPath[i] == '\\') {
			directory[len] = '\0';
			//cout << directory << endl;
			if (FstClus = isDirectoryExist(directory, FstClus)) {
				len = 0;
			}
			else {
				len = 0;
				break;
			}
			i++;
		}
		else {
			if (len == 11) break; // ����Ѿ�����11�ֽڻ��������Ŀ¼��̫����
			directory[len++] = pszFolderPath[i++];
		}
	}
	if (pszFolderPath[i] != '\0' && len == 11) return 0; //˵���м�ĳ��Ŀ¼��̫�����˳�
	if (len > 0) {
		directory[len] = '\0';
		//cout << directory << endl;
		FstClus = isDirectoryExist(directory, FstClus);
	}
	return FstClus;
}

u16 getFATValue(u16 FstClus) {
	//FAT1��ƫ���ֽ�  
	int fatBase = RsvdSecCnt * BytsPerSec;
	//FAT���ƫ���ֽ�  
	int fatPos = fatBase + FstClus * 3 / 2;
	//��żFAT�����ʽ��ͬ��������д�������0��FAT�ʼ  
	int type;
	if (FstClus % 2 == 0) {
		type = 0;
	}
	else {
		type = 1;
	}
	//�ȶ���FAT�����ڵ������ֽ�  
	u16 bytes;
	u16* bytes_ptr = &bytes;
	//SetOffset(fatPos, NULL, FILE_BEGIN);
	SetOffset(fatPos, NULL, FILE_BEGIN);
	//if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
	if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
		//u16Ϊshort����ϴ洢��Сβ˳���FAT��ṹ���Եõ�  
		//typeΪ0�Ļ���ȡbyte2�ĵ�4λ��byte1���ɵ�ֵ��typeΪ1�Ļ���ȡbyte2��byte1�ĸ�4λ���ɵ�ֵ  
		if (type == 0) {
			// ע���ƻ�����Ҫ��Ȼ������
			bytes = bytes << 4;
			bytes = bytes >> 4;
		}
		else {
			bytes = bytes >> 4;
		}
		return bytes;
	}
	else {
		return 0xFFF;
	}
}

void initFileInfo(RootEntry* FileInfo_ptr, char* FileName, u8 FileAttr, u32 FileSize) {
	time_t ts = getTS();
	FileInfo_ptr->DIR_Attr = FileAttr;
	FileInfo_ptr->DIR_WrtDate = getDOSDate(ts);
	FileInfo_ptr->DIR_WrtTime = getDOSTime(ts);
	int i = 0;
	if (FileAttr == 0x10) {
		FileInfo_ptr->DIR_FileSize = 0;
		while (FileName[i] != '\0' && i < 11) {
			FileInfo_ptr->DIR_Name[i] = FileName[i];
			i++;
		}
		while (i < 11) {
			FileInfo_ptr->DIR_Name[i] = 0x20;
			i++;
		}
	}
	else {
		FileInfo_ptr->DIR_FileSize = FileSize;
		while (FileName[i] != '\0') {
			if (FileName[i] == '.') {
				int j = i;
				while (j < 8) {
					FileInfo_ptr->DIR_Name[j] = 0x20;
					j++;
				}
				i++;
				break;
			}
			else {
				FileInfo_ptr->DIR_Name[i] = FileName[i];
				i++;
			}

		}
		memcpy(&FileInfo_ptr->DIR_Name[8], &FileName[i], 3);
	}
	int clusNum;
	if ((FileSize % BytsPerSec) == 0 && FileSize != 0) {
		clusNum = FileSize / BytsPerSec;
	}
	else {
		clusNum = FileSize / BytsPerSec + 1;
	}
	FileInfo_ptr->DIR_FstClus = setFATValue(clusNum);
}

BOOL writeEmptyClus(u16 FstClus, RootEntry* FileInfo) {
	int dataBase;
	u16 originClus;
	BOOL success = FALSE;
	do {
		int loop;
		originClus = FstClus; // �����0xfff�غ�
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadDisk(rootEntry_ptr, 32, NULL) != 0) {
				// ˵����Ŀ¼�����
				if (rootEntry_ptr->DIR_Name[0] == 0x00 || rootEntry_ptr->DIR_Name[0] == 0xE5) {
					SetOffset(dataBase, NULL, FILE_BEGIN);
					if (WriteDisk(FileInfo, 32, NULL) != 0) {
						success = TRUE;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (success) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
	if (success == FALSE && FstClus != 0) { // Ŀ¼�ռ䲻���Ҳ��Ǹ�Ŀ¼
		u16 bytes;
		u16* bytes_ptr = &bytes;
		int fatBase = RsvdSecCnt * BytsPerSec;
		u16 tempClus = setFATValue(1);
		if (tempClus == 0) return FALSE;
		dataBase = SetOffset((fatBase + originClus * 3 / 2), NULL, FILE_BEGIN); // β�غ�ƫ��
		SetOffset(dataBase, NULL, FILE_BEGIN);
		if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
			if (originClus % 2 == 0) {
				bytes = bytes >> 12;
				bytes = bytes << 12; // ��������λ����12λΪ0
				bytes = bytes | tempClus;
			}
			else {
				bytes = bytes << 12;
				bytes = bytes >> 12; // ��������λ����12λΪ0
				bytes = bytes | (tempClus << 4);
			}
			SetOffset((fatBase + originClus * 3 / 2), NULL, FILE_BEGIN);
			if (WriteDisk(bytes_ptr, 2, NULL) == 0) {
				return FALSE;
			}
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (tempClus - 2) * BytsPerSec;
			SetOffset(dataBase, NULL, FILE_BEGIN);
			WriteDisk(setzero, BytsPerSec, NULL); // ��0
			SetOffset(dataBase, NULL, FILE_BEGIN);
			if (WriteDisk(FileInfo, 32, NULL) != 0) {
				success = TRUE;
			}
		}
	}
	return success;
}

u16 setFATValue(int clusNum) {
	int fatBase = RsvdSecCnt * BytsPerSec;
	int fatPos = fatBase + 3; // ��2�Ŵؿ�ʼ���ң��ſ�3�ֽ�
	//�ȶ���FAT�����ڵ������ֽ�
	u16 clus = 2;
	int i = 0;
	u16 bytes; // ÿ�ζ�ȡ�Ĵغ�
	u16* bytes_ptr = &bytes;
	u16 FstClus;
	u16 preClus;
	int loop = FATSz * BytsPerSec / 3 * 2 - 2; // ���ж��ٸ���
	do {
		SetOffset(fatPos, NULL, FILE_BEGIN);
		if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
			// �غ�Ϊż�� 
			if (clus % 2 == 0) {
				bytes = bytes << 4;
				bytes = bytes >> 4; // ��߲��ƻ���Ҳ���ԣ���������0
			}
			else {
				bytes = bytes >> 4;
			}
			if (bytes == 0x000) {
				if (i > 0) {
					SetOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
					if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
						if (preClus % 2 == 0) {
							bytes = bytes >> 12;
							bytes = bytes << 12; // ��������λ����12λΪ0
							bytes = bytes | clus; // �뵱ǰclus��λ��
						}
						else {
							bytes = bytes << 12;
							bytes = bytes >> 12; // ��������λ����12λΪ0
							bytes = bytes | (clus << 4);
						}
						SetOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
						WriteDisk(bytes_ptr, 2, NULL);
					}
				}
				else {
					FstClus = clus; // �����״�
				}
				preClus = clus;
				if (clusNum == ++i) break; // ��β���˳�ѭ��
			}
		}
		if (clus % 2 == 0) {
			fatPos++; // ����ƫһ���ֽ�
		}
		else {
			fatPos += 2; // ����ƫ2���ֽ�
		}
		clus++; // �غż�һ
		loop--;
	} while (loop > 0);
	// β�ز�0xfff
	SetOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
	if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
		if (preClus % 2 == 0) {
			bytes = bytes >> 12;
			bytes = bytes << 12; // ��������λ����12λΪ0
			bytes = bytes | 0x0FFF;
		}
		else {
			bytes = bytes << 12;
			bytes = bytes >> 12; // ��������λ����12λΪ0
			bytes = bytes | 0xFFF0;
		}
		SetOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
		WriteDisk(bytes_ptr, 2, NULL);
	}
	// ��û����ɹ�����������
	if (clusNum != i) {
		recoverClus(FstClus); // ����ʧ�ܣ��ع�����
		return 0;
	}
	else {
		return FstClus;
	}
}

DWORD createHandle(RootEntry* FileInfo, u16 parentClus) {
	for (int j = 1; j < MAX_NUM; j++) {
		if (dwHandles[j] != NULL) {
			if (dwHandles[j]->fileInfo.DIR_FstClus == FileInfo->DIR_FstClus) {
				return j; // ˵�����ļ��ѱ��򿪣���������������
			}
		}
	}
	FileHandle* hd = (FileHandle*)malloc(sizeof(FileHandle)); // ͳһ������malloc
	for (int i = 1; i < MAX_NUM; i++) {
		if (dwHandles[i] == NULL) {
			memcpy(&hd->fileInfo, FileInfo, 32);
			hd->offset = 0; // ƫ������ʼ��Ϊ0
			hd->parentClus = parentClus;
			dwHandles[i] = hd;
			return i; //���뵽��return i
		}
	}
	return 0; //û�п��þ�� return 0
}

BOOL recoverClus(u16 fileClus) {
	// ���մ�
	u16 bytes;
	u16* bytes_ptr = &bytes;
	// ��һ��Ϊĩβ���˳�ѭ��
	while (fileClus != 0xFFF) {
		int clusBase = RsvdSecCnt * BytsPerSec + fileClus * 3 / 2;
		u16 tempClus = getFATValue(fileClus); // �ݴ���һ�أ���ǰ������ˢ�³�0
		SetOffset(clusBase, NULL, FILE_BEGIN);
		if (ReadDisk(bytes_ptr, 2, NULL) != 0) {
			if (fileClus % 2 == 0) {
				bytes = bytes >> 12;
				bytes = bytes << 12; // ��12λ��0
			}
			else {
				bytes = bytes << 12;
				bytes = bytes >> 12; // ��12λ��0
			}
			SetOffset(clusBase, NULL, FILE_BEGIN);
			WriteDisk(bytes_ptr, 2, NULL); // д�أ����ոô�
		}
		fileClus = tempClus; // ����ƫ����
	}
	return TRUE;
}

void recursiveDeleteDirectory(u16 fClus) {
	u8 del = 0xE5;
	// �ݹ�ɾ���ļ����µ��ļ���Ŀ¼
	// fClus �����ɾ���ļ����״�
	RootEntry fdd;
	RootEntry* fdd_ptr = &fdd;
	int fBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fClus - 2) * BytsPerSec; // �ҵ����ļ��е�����ƫ��
	// ������ɾ��Ŀ¼�µ�����Ŀ¼��ɾ����
	do {
		for (int k = 0; k < BytsPerSec / 32; k++) {
			SetOffset(fBase, NULL, FILE_BEGIN);
			if (ReadDisk(fdd_ptr, 32, NULL) != 0) {
				// �ļ���ֱ�Ӱѵ�һ�ֽڸ��˾ͳ�
				if (fdd_ptr->DIR_Name[0] != 0xE5 && fdd_ptr->DIR_Name[0] != 0 && fdd_ptr->DIR_Name[0] != 0x2E) {
					if (fdd_ptr->DIR_Attr == 0x20) {
						SetOffset(fBase, NULL, FILE_BEGIN);
						WriteDisk(&del, 1, NULL);
						recoverClus(fdd_ptr->DIR_FstClus); // �����ļ���
					}
					else if (fdd_ptr->DIR_Attr == 0x10) {
						// �ļ��еݹ����ɾ������µ�Ŀ¼��
						SetOffset(fBase, NULL, FILE_BEGIN);
						WriteDisk(&del, 1, NULL);
						recursiveDeleteDirectory(fdd_ptr->DIR_FstClus); // �ݹ����
						recoverClus(fdd_ptr->DIR_FstClus); // ����Ŀ¼��
					}
				}
			}
			fBase += 32;
		}
	} while ((fClus = getFATValue(fClus)) != 0xFFF);
}

void syncFat12() {
	int fat1Base = RsvdSecCnt * BytsPerSec;
	int fat2Base = FATSz * BytsPerSec + fat1Base; // fat2ƫ��
	char fat[512 * 9] = { 0 };
	SetOffset(fat1Base, NULL, FILE_BEGIN);
	if (ReadDisk(fat, FATSz * BytsPerSec, NULL) != 0) {
		SetOffset(fat2Base, NULL, FILE_BEGIN);
		WriteDisk(fat, FATSz * BytsPerSec, NULL);
	}
}