#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

char * firstpatch = "тут вписать папку где будут происходить эксперементы";

struct HandleList
{
	pid_t Obj;
	HandleList * Next;
};

typedef HandleList * PHandleList;

void zAddInHandleList(PHandleList & List, pid_t Pid)
{
	if (List == NULL)
	{
		List = (PHandleList)malloc(sizeof(HandleList));
		if (List == NULL)
		{
			printf("Ошибка выделения памяти %s\n",strerror(errno));
		} else {
			List->Obj = Pid;
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
			printf("Wait оишибка %s - %d\n",strerror(errno), Next->Obj);
		} else {
			if (WIFEXITED(status) == 0)
			{
				printf("Ошибка закрытия процесса %s - %d\n",strerror(errno), Next->Obj);
			}
		}
		Next = Next->Next;
	}
}

void zDestroyPids(PHandleList & List)
{
	PHandleList Next = List;
	while (Next != NULL)
	{
		int * status;
		List = Next;
		Next = Next->Next;
		free(List);
	}
}

void zSearchFiles(const char * path)
{
	PHandleList List = NULL;

	dirent * File;

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
		pid_t NewPid;

		bool isWork = true;
		if (Folder == NULL)
		{
			printf("Ошибка  %s - %s\n",strerror(errno), Buffer);
		} else {
			while (isWork)
			{
				int checkerror = 0;
				File = readdir(Folder);
				if (File == NULL)
				{
					if (checkerror != errno)
					{
						printf("Ошибка чтения файла - %s\n",strerror(errno));
					}
					isWork = false;
				} else {
					char StatBuffer[500];
					strcpy(StatBuffer,Buffer);
					strcat(StatBuffer,File->d_name);
					if (stat(StatBuffer,&FileStat) == -1)
					{
						printf("Stat ошибка %s - %s\n",strerror(errno), StatBuffer);
					} else {
						if (S_ISREG(FileStat.st_mode) && S_IRUSR & FileStat.st_mode)
						{
						//zRenameBadFiles(FileBuffer,File->d_name);
							char RenameBuffer[500];
							strcpy(RenameBuffer, StatBuffer);
							strcat(RenameBuffer, ".read");
							rename(StatBuffer, RenameBuffer);
						} else {
							if (S_ISDIR(FileStat.st_mode))
							{
								if (!((strcmp(File->d_name,"..") == 0) || (strcmp(File->d_name,".") == 0)))
								{
									strcat(StatBuffer,"/");
									NewPid = fork();
									if (NewPid > 0)
									{
										zAddInHandleList(List,NewPid);
										//printf("Процесс дублирован pid - %d\n", NewPid);
									}
									if (NewPid == 0)
									{
										isNewThread = true;
										isWork = false;

										strcpy(Buffer,StatBuffer);
										strcpy(FileBuffer,StatBuffer);

										zDestroyPids(List);

										File = NULL;
										List = NULL;
									}
									if (NewPid < 0)
										printf("Ошибка дублирования %s\n",strerror(errno));
								}
							}
						}
					}
				}
			}
		}
		if (closedir(Folder) == -1)
		{
			printf("Ошибка закрытия потока файлов- %s\n",strerror(errno));
		}
	}
	zWaitPids(List);
	zDestroyPids(List);
}

int main()
{
	zSearchFiles(firstpatch);
	return 0;
}
