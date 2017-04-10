#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char * firstpatch = "/home/me/myfolder/";

bool isFirstFather = true;
/*

	Структура FileInfo:
		path - путь к файлу
		atime - время последнего обращения

*/
struct FileInfo
{
	char path[500];
	int atime;
};


struct HandleList
{
	pid_t Obj;
	int rHandle;
	HandleList * Next;
};

typedef HandleList * PHandleList;

void zAddInHandleList(PHandleList & List, pid_t Pid, int Pipe)
{
	if (List == NULL)
	{
		List = (PHandleList)malloc(sizeof(HandleList));
		if (List == NULL)
		{
			printf("Ошибка выделения памяти %s\n",strerror(errno));
		} else {
			List->Obj = Pid;
			List->rHandle = Pipe;
			List->Next = NULL;
		}
	} else {
		PHandleList Next = List;
		while (Next->Next != NULL)
		{
			Next = Next->Next;
		}
		Next->Next = (PHandleList)malloc(sizeof(HandleList));
		if (Next->Next == NULL)
		{
			printf("Ошибка выделения памяти %s\n",strerror(errno));
		} else {
			Next->Next->rHandle = Pipe;
			Next->Next->Obj = Pid;
			Next->Next->Next = NULL;
		}
	}
}

void zWaitPids(PHandleList & List)
{
	PHandleList Next = List;
	while (Next != NULL)
	{
		int status;
		if (waitpid(Next->Obj,&status,0) == -1)
		{
			printf("Wait ошибка %s - %d\n",strerror(errno), Next->Obj);
		} else {
			if (WIFEXITED(status) == 0)
			{
				printf("Ошибка закрытия процесса %s - %d\n",strerror(errno), Next->Obj);
			}
		}
		Next = Next->Next;
	}
}

bool zSearchLastFile(PHandleList & List, FileInfo & Result)
{
	if (List == NULL)
	{
		return false;
	}

	FileInfo Buffer,Res;
	PHandleList Next = List;

	Res.atime = 0;

	while (Next != NULL)
	{
		if (read(Next->rHandle,&Buffer,sizeof(FileInfo)) == -1)
		{
			printf("Read eror %s\n",strerror(errno));
		} else {
			//printf("%s \n", Buffer.path);
			if (Buffer.atime > Res.atime)
			{
				char Buff[500];;
				int atime = 0;

				atime = Res.atime;
				strcpy(Buff,Res.path);

				Res.atime = Buffer.atime;
				strcpy(Res.path,Buffer.path);

				Buffer.atime = atime;
				strcpy(Buffer.path,Buff);
			}
		}
		Next = Next->Next;
	}

	Result.atime = Res.atime;
	strcpy(Result.path,Res.path);

	return true;
}

void zDestroyPids(PHandleList & List)
{
	PHandleList Next = List;
	while (Next != NULL)
	{
		List = Next;
		if (close(List->rHandle) == -1)
		{
			printf("Close eror %s\n",strerror(errno));
		}
		Next = Next->Next;
		free(List);
	}
}

/*

	Возвращает true если удаление произошло успешно
	Иначе false

*/

bool DestroyFileInfo(FileInfo * & target)
{
	if (target != NULL)
	{
		free(target);
		return true;
	} else {
		return false;
	}
}

/*

	Возвращает NULL если произошла ошибка
	Иначе возвращает адрес на переменную типа FileInfo
	Переменную FileInfo нужно после использования удалить!

*/

FileInfo * zSearchFiles(const char * path)
{
	int Output = -1;

	dirent * File = NULL;

	PHandleList List = NULL;

	FileInfo * LastFile = (FileInfo *)malloc(sizeof(FileInfo));
	if (LastFile == NULL)
	{
		return LastFile;
	}
	LastFile->atime = 0;

	char FileBuffer[500];
	strcpy(FileBuffer,path);

	char Buffer[500];
	strcpy(Buffer,path);
	//strcat(Buffer,"*");
	bool isNewThread = true;
	while (isNewThread)
	{
		isNewThread = false;
		DIR * Folder = opendir(Buffer);
		struct stat FileStat;

		bool isWork = true;
		if (Folder == NULL)
		{
			printf("Eror %s - %s\n",strerror(errno), Buffer);
		} else {
			while (isWork)
			{
				int checkerror = errno;
				File = readdir(Folder);
				if (File == NULL)
				{
					if (checkerror != errno)
					{
						printf("Eror %s\n",strerror(errno));
					}
					isWork = false;
				} else {
					char StatBuffer[500];
					strcpy(StatBuffer,Buffer);
					strcat(StatBuffer,File->d_name);
					if (stat(StatBuffer,&FileStat) == -1)
					{
						printf("Stat eror %s - %s\n",strerror(errno), StatBuffer);
					} else {
						if (S_ISREG(FileStat.st_mode))
						{
							if (FileStat.st_atime > LastFile->atime)
							{
								char Buffer2[500];
								strcpy(Buffer2,Buffer);
								strcat(Buffer2,File->d_name);
								strcpy(LastFile->path,Buffer2);
								LastFile->atime = FileStat.st_atime;
							}
						} else {
							if (S_ISDIR(FileStat.st_mode))
							{
								if (!((strcmp(File->d_name,"..") == 0) || (strcmp(File->d_name,".") == 0)))
								{
									int IOFiles[2];
									if (pipe(IOFiles) == -1)
									{
										printf("Pipe eror %s - %s\n",strerror(errno), StatBuffer);
									} else {
										strcat(StatBuffer,"/");
										int mypid = fork();
										if (mypid == -1)
										{
											printf("Fork eror %s - %s\n",strerror(errno), StatBuffer);
										}
										if (mypid == 0)
										{
											printf("Создан новый процесс... \n");	
											Output = IOFiles[1];

											if (close(IOFiles[0]) == -1)
											{
												printf("Close eror %s\n",strerror(errno));
											}

											isFirstFather = false;
											isWork = false;
											isNewThread = true;

											LastFile->atime = 0;

											strcpy(Buffer,StatBuffer);
											strcpy(FileBuffer,StatBuffer);
											strcpy(LastFile->path,"");

											zDestroyPids(List);

											File = NULL;
											List = NULL;
										}
										if (mypid > 0)
										{	
											if (close(IOFiles[1]) == -1)
											{
												printf("Close eror %s\n",strerror(errno));
											}

											zAddInHandleList(List,mypid,IOFiles[0]);
										}
									}
								}
							}
						}
					}
				}
			}
			if (closedir(Folder) == -1)
			{
				printf("Eror %s\n",strerror(errno));
			}
		}
	}

	zWaitPids(List);

	FileInfo wList;

	if (zSearchLastFile(List,wList) == true)
	{
		if (LastFile->atime < wList.atime)
		{
			LastFile->atime = wList.atime;
			strcpy(LastFile->path,wList.path);
		}
	}


	if (Output != -1)
	{
		//printf("%s \n", LastFile->path);
		if (write(Output,LastFile,sizeof(FileInfo)) == -1)
		{
			printf("Write eror %s\n",strerror(errno));
		}
		if (close(Output) == -1)
		{
			printf("Close eror %s\n",strerror(errno));
		}
	}

	zDestroyPids(List);

	return LastFile;
}

int main()
{
	FileInfo * Buffer = zSearchFiles(firstpatch);
	if (Buffer == NULL)
	{	
		printf("Ошибка - %s\n", strerror(errno));
	} else {
			//printf("%s \n", Buffer->path);
		if (isFirstFather)
		{
			if (strcmp(Buffer->path,"") == 0)
			{
				printf("Папка - %s пуста\n", firstpatch);
			} else {
				printf("Этот файл это - %s \n", Buffer->path);
			}
		}
		DestroyFileInfo(Buffer);
	}
	return 0;
}