#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <errno.h>

bool isNumber(char * str)
{
	char * digits = "0123456789";
	bool isFullNormal = true;
	for (int i = 0; (i < strlen(str)) && (isFullNormal); i++)
	{
		bool isNormal = false;
		for (int j = 0; (j < strlen(digits)) && (!isNormal); j++)
		{
			isNormal = (str[i] == digits[j]);
		}
		isFullNormal = isNormal;
	}
	return isFullNormal;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Неправильное количесвто аргументов! Пример: myhead file.txt 5\n");
	} else {
		char FileName[255];
		//if (getcwd(FileName,255) == NULL)
		//{
		//	printf("Внутреняя ошибка - %s\n",strerror(errno));
		//} else {}
			//strcat(FileName,"/");
		strcpy(FileName,argv[1]);
		struct stat sb;
		if((stat(FileName, &sb) == -1))
		{
			printf("Файл - %s не существует!!!\n", FileName);
		} else {
			if (!isNumber(argv[2]))
			{
				printf("Второй аргумент должен быть числом! Пример: myhead file.txt 5\n");
			} else {
				int coll = atoi(argv[2]);
				FILE * ptrFile = fopen(FileName,"rb");
 				if (ptrFile == NULL)
 				{
     				printf("Ошибка файла - %s\n", strerror(errno));
  				} else {
  					bool isWork = true;
  					char buffer;
  					int j = 1;
  					//int err = errno;
  					while (isWork)
  					{
  						if (fread(&buffer, sizeof(char), 1, ptrFile) != sizeof(char))
  						{
  							if (ferror(ptrFile))
  							{
  								printf("Ошибка вывода из фала - %s\n", strerror(errno));
  							}
  							isWork = false;
  						} else {
  							printf("%c", buffer);
  							if (buffer == '\n')
  							{
  								j++;
  								if (j == coll+1) 
  								{
  									isWork = false;
  								}
  							}
 						}
  					}
  				}
  				if (fclose(ptrFile) != 0)
  				{
  					printf("Ошибка закртыия фала - %s\n", strerror(errno));
  				}
			}
		}	
	}
	return 1;
}