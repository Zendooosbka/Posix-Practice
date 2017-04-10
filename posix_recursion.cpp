#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char * firstpatch = "/home/me/myfolder/";

void zSearchFiles(const char * path)
{
	dirent * File;

	char FileBuffer[500];
	strcpy(FileBuffer,path);

	//char Buffer[500];
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
					if (S_ISREG(FileStat.st_mode) && S_IRUSR & FileStat.st_mode)
						{
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
								zSearchFiles(StatBuffer);
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

int main()
{
	zSearchFiles(firstpatch);
	return 0;
}