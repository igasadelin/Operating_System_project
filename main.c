#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>


void checkDir(DIR *dir, char path[1000], int k)
{
	struct dirent* entry;
	while((entry=readdir(dir))!=NULL)
	{
		if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0 )
		{
			continue;
		}
		struct stat file_info;
		
		char pathaux[1000];
		strcpy(pathaux, path);
		strcat(pathaux, "/");
		strcat(pathaux, entry->d_name);
		
		for(int i=0;i<k;i++)
		{
			printf("\t");
		}
		printf("--->%s\n", entry->d_name);
		
		stat(pathaux, &file_info);
		if(S_ISDIR(file_info.st_mode))
		{
			//printf("e director\n\n");
			DIR *d=opendir(pathaux);
			if(d==NULL)
			{
				exit(-7);
			}
			checkDir(d, pathaux,k+1);
		}
		else
		{
			if(S_ISREG(file_info.st_mode))
			{
				
				
				if(strstr(entry->d_name, "_snapshot.txt"))
				{
					continue;
				}
				
				char numefis[100];
				strcpy(numefis, entry->d_name);
				
				char *aux=strtok(numefis, ".txt");
				strcpy(numefis, aux);
				
				strcat(numefis, "_snapshot.txt");
				
				char new_path[1000];
				strcpy(new_path, path);
				strcat(new_path,"/");
				strcat(new_path, numefis);
				
				//printf("%s\n\n", new_path);
				
				
				int file=open(new_path, O_CREAT  |O_RDWR, S_IWUSR| S_IRUSR);
				if(file==-1)
				{
					printf("ba nujuu\n");
					exit(-13);
				}
				
				printf( "dimensiune: %ld\n", file_info.st_size);
				
				
				char buffer[500]="dimensiune::";
				printf("%s\n", buffer);
				
				//char conversie[35];
				//ltoa(file_info.st_size, conversie, 10);
				//printf("%s\n", conversie);
				
				//strcat(buffer, (char*)file_info.st_ino);
				printf("%s\n", buffer);
				strcat(buffer,"bytes\n");
				//write(file,buffer,strlen(buffer));
			}
		}
	}


}



int main(int argc, char ** argv)
{
	if(argc!=2)
	{
		exit(-3);
	}
	
	DIR *dir;
	dir=opendir(argv[1]);
	if(dir==NULL)
	{
		exit(-4);
	}
	
	
	printf("%s\n", argv[1]);
	char path[1000]="";
	strcpy(path, argv[1]);
	checkDir(dir, path,1);
	
	
	
}