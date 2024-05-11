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
#include <time.h>
#include <sys/wait.h>


void takeSnapshot(char *dirPath, char *outputDir, char * pathaux, struct dirent *entry) {
    char snapshotName[1000];
    strcpy(snapshotName, strtok(entry->d_name, "."));  
    strcat(snapshotName, "_snapshot.txt");

    char snapshotPath[1000];
    strcpy(snapshotPath, outputDir);
    strcat(snapshotPath, "/");
    strcat(snapshotPath, snapshotName);

    int snapshotFile = open(snapshotPath, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    printf("%s    ",snapshotPath);
    struct stat fileStat;

    if (stat(pathaux, &fileStat) == -1) 
	{
		exit(-10);
	}

    time_t current_time = time(NULL); // Obține timpul curent
	char *ora = ctime(&current_time);

	write(snapshotFile, "Timestamp: ", strlen("Timestamp: "));
	write(snapshotFile, ora, strlen(ora));	
	
	write(snapshotFile, "Entry: ", 7);
	write(snapshotFile, entry->d_name,strlen(entry->d_name));
	write(snapshotFile, "\n", 1);
	
	write(snapshotFile, "Last Modified: ",strlen("Last Modified: "));
	write(snapshotFile, ctime(&fileStat.st_mtime), strlen(ctime(&fileStat.st_mtime)));
	
	write(snapshotFile, "Size: ", strlen("Size: "));
	char file_size[20];
	//printf("File size: %ld bytes\n", fileStat.st_size);
	sprintf(file_size,"%lld",fileStat.st_size);
	write(snapshotFile, file_size, strlen(file_size));
	write(snapshotFile, " bytes\n", strlen(" bytes\n"));
	
	write(snapshotFile, "Permissions: ",strlen("Permissions: "));
	write(snapshotFile, ((S_ISDIR(fileStat.st_mode)) ? "d" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IRUSR) ? "r" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IWUSR) ? "w" : "-"),1);	
	write(snapshotFile, ((fileStat.st_mode & S_IXUSR) ? "x" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IRGRP) ? "r" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IWGRP) ? "w" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IXGRP) ? "x" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IROTH) ? "r" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IWOTH) ? "w" : "-"),1);
	write(snapshotFile, ((fileStat.st_mode & S_IXOTH) ? "x" : "-"),1);
	
	char file_inode[20];
	sprintf(file_inode,"%llu",entry->d_ino);
	//printf("%s  ", file_inode);
	write(snapshotFile, "\nInode no: ", strlen("\nInode no: "));
	write(snapshotFile,file_inode,strlen(file_inode));

}

void checkDir(char *path, char *outputDir) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;
    char pathaux[1000];

    if ((dir = opendir(path)) == NULL) {
        perror("OpernDir Error!\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        strcpy(pathaux, path);
        strcat(pathaux, "/");
        strcat(pathaux, entry->d_name);

        if (stat(pathaux, &file_info) == -1) {
            perror("Stat Error!\n");
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            checkDir(pathaux, outputDir);
        } else {
            // TODO
            if (S_ISREG(file_info.st_mode)){
                if(strstr(entry->d_name,"_snapshot"))
                    continue;
			    printf("File: %s\n", pathaux);
               takeSnapshot(pathaux, outputDir, pathaux, entry);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
		printf("Argc Error!\n");
        return 1;
	}
	
	if (strcmp(argv[1], "-o") != 0) {
		printf("String Error!\n");
        return 1;
	}

    for (int i = 3 ; i < argc ; i++)
	{	
		pid_t pid = fork();
		if (pid == 0){
			checkDir(argv[i],argv[2]);
			exit(2);
		} else if (pid < 0){
			printf("Fork Error\n");
			exit(-2);
		}
	}

	for (int i = 3; i < argc ; i++){	
		int status;
		pid_t pid = waitpid(-1, &status, 0);

		if (pid < 0) {
			printf("Waiting Error!\n");
			exit(-2);
		}

		if (WIFEXITED(status)){
			printf("Process %d has ended with status %d\n", pid, WEXITSTATUS(status));
		} else {
			printf("eroare incheiere proces\n");
		}
	}
    
    return 0;
}