#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char * firstpatch = "/home/me/myfolder/";


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

/* Это нам не нада

void zRenameBadFiles(const char * path,const char * filename)
{
	char * Lit = "abcdefghijklmnopqrstuvwxyz. ";
	char * LitBig = "ABCDEFGHIJKLMNOPQRSTUVWXYZ. ";
	char RenamedFile[255];
	char RenamedFilePath[255];
	char FileNamePath[255];
	bool isFullNormal = true;
	for (int i = 0; (i < strlen(filename)) && (isFullNormal); i++)
	{
		bool isNormal = false;
		for (int j = 0; (j < strlen(Lit)) && (!isNormal); j++)
		{
			isNormal = (Lit[j] == filename[i]) || (LitBig[j] == filename[i]);
		}
		isFullNormal = isNormal;
	}
	if (isFullNormal)
	{
		int j = 0;
		for (int i = strlen(filename) - 1;i > -1;i--)
		{
			RenamedFile[j] = filename[i];
			j++;
		}
		RenamedFile[j] = '\0';
		strcpy(RenamedFilePath,path);
		strcat(RenamedFilePath,RenamedFile);
		strcpy(FileNamePath,path);
		strcat(FileNamePath,filename);
		if (rename(FileNamePath,RenamedFilePath) != 0)
		{
			printf("Rename eror %s - %s\n",strerror(errno),filename);
		} else {
			printf("File %s Done!\n",RenamedFilePath);
		}
	}
}
*/

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
	dirent * File = NULL;

	FileInfo * LastFile = (FileInfo *)malloc(sizeof(FileInfo));
	if (LastFile == NULL)
	{
		return LastFile;
	}
	LastFile->atime = 0;

	char FileBuffer[500];
	strcpy(FileBuffer,path);

	char Buffer[500];
	//strcpy(Buffer,path);
	//strcat(Buffer,"*");

	DIR * Folder = opendir(path);
	struct stat FileStat;

	bool isWork = true;
	if (Folder == NULL)
	{
		printf("Eror %s - %s\n",strerror(errno), path);
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
				strcpy(StatBuffer,path);
				strcat(StatBuffer,File->d_name);
				if (stat(StatBuffer,&FileStat) == -1)
				{
					printf("Stat eror %s - %s\n",strerror(errno), StatBuffer);
				} else {
					if (S_ISREG(FileStat.st_mode))
					{
						if (FileStat.st_atime > LastFile->atime)
						{
							strcpy(Buffer,path);
							strcat(Buffer,File->d_name);
							strcpy(LastFile->path,Buffer);
							LastFile->atime = FileStat.st_atime;
						}
					} else {
						if (S_ISDIR(FileStat.st_mode))
						{
							if (!((strcmp(File->d_name,"..") == 0) || (strcmp(File->d_name,".") == 0)))
							{

								strcat(StatBuffer,"/");
								FileInfo * Buff = zSearchFiles(StatBuffer);
								if (Buff == NULL)
								{
									printf("Папка - %s пуста\n", StatBuffer);
								} else {
									if (Buff->atime > LastFile->atime)

									{
										strcpy(LastFile->path,Buff->path);
										LastFile->atime = Buff->atime;
										DestroyFileInfo(Buff);
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
	return LastFile;
}

int main()
{
	FileInfo * Buffer = zSearchFiles(firstpatch);
	if (Buffer == NULL)
	{	
		printf("Ошибка - \n", strerror(errno));
	} else {
		if (strcmp(Buffer->path,"") == 0)
		{
			printf("Папка - %s пуста\n", firstpatch);
		} else {
			printf("Этот файл это - %s \n", Buffer->path);
		}
		DestroyFileInfo(Buffer);
	}
	return 0;
}