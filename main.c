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

// Functie care scrie in fisierul snapshot
void writeSnapshot(char *dirPath, char *snapshotPath, struct dirent *entry) {
	int snapshotFile = open(snapshotPath, O_RDWR | O_CREAT, 0644); // 0644 permite read si write pentru owner si doar read pentru ceilalti
	printf("%s    ", snapshotPath); 
	struct stat fileStat;

	if (stat(dirPath, &fileStat) == -1) {// Obține informații despre fișier
		return;
	}

	time_t current_time = time(NULL); // Obține timpul curent
	char *ora = ctime(&current_time);

	write(snapshotFile, "Timestamp: ", strlen("Timestamp: "));// Scrie in fisier timpul curent
	write(snapshotFile, ora, strlen(ora));

	write(snapshotFile, "Entry: ", 7); 
	write(snapshotFile, entry->d_name, strlen(entry->d_name)); // Scrie in fisier numele fisierului
	write(snapshotFile, "\n", 1);

	write(snapshotFile, "Last Modified: ", strlen("Last Modified: "));
	write(snapshotFile, ctime(&fileStat.st_mtime), strlen(ctime(&fileStat.st_mtime))); // Scrie in fisier timpul ultimei modificari

	write(snapshotFile, "Size: ", strlen("Size: "));
	char file_size[20];

	sprintf(file_size, "%lld", fileStat.st_size); // Scrie in fisier dimensiunea fisierului
	write(snapshotFile, file_size, strlen(file_size)); 
	write(snapshotFile, " bytes\n", strlen(" bytes\n"));

	write(snapshotFile, "Permissions: ", strlen("Permissions: ")); 
	write(snapshotFile, ((S_ISDIR(fileStat.st_mode)) ? "d" : "-"), 1); // Scrie in fisier permisiunile fisierului
	write(snapshotFile, ((fileStat.st_mode & S_IRUSR) ? "r" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IWUSR) ? "w" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IXUSR) ? "x" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IRGRP) ? "r" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IWGRP) ? "w" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IXGRP) ? "x" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IROTH) ? "r" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IWOTH) ? "w" : "-"), 1);
	write(snapshotFile, ((fileStat.st_mode & S_IXOTH) ? "x" : "-"), 1);

	char file_inode[20];
	sprintf(file_inode, "%llu", entry->d_ino); // Scrie in fisier inode-ul fisierului
	write(snapshotFile, "\nInode no: ", strlen("\nInode no: "));
	write(snapshotFile, file_inode, strlen(file_inode));
}

// Functie care face snapshot
void takeSnapshot(char *dirPath, char *outputDir, struct dirent *entry) { 
	char snapshotName[1000];
	strcpy(snapshotName, strtok(entry->d_name, "."));// Iau numele fisierului fara extensie
	strcat(snapshotName, "_snapshot.txt"); // Adaug _snapshot.txt la numele fisierului

	char snapshotPath[1000];
	strcpy(snapshotPath, outputDir); // Path-ul unde se va salva snapshotul
	strcat(snapshotPath, "/");
	strcat(snapshotPath, snapshotName); // Adaug numele fisierului la path

	writeSnapshot(dirPath, snapshotPath, entry); // Scriu in fisierul snapshot
}

// Functie care verifica daca snapshotul s-a schimbat
void verificare_snapshot(char *path, char *dirPath, struct dirent *entry, char *outputDir) {
	char tmpPath[1000]; // pt verif dintre fisiere
	strcpy(tmpPath, path);
	strcat(tmpPath, "/");
	strcat(tmpPath, "tmp.txt"); // path fisier temporar

	writeSnapshot(dirPath, tmpPath, entry); // Scriu in fisierul temporar

	char snapshotName[1000];
	strcpy(snapshotName, strtok(entry->d_name, ".")); // Iau numele fisierului fara extensie
	strcat(snapshotName, "_snapshot.txt");  // Adaug _snapshot.txt la numele fisierului

	char snapshotPath[1000];
	strcpy(snapshotPath, outputDir); // Path-ul unde se va salva snapshotul
	strcat(snapshotPath, "/");
	strcat(snapshotPath, snapshotName);
	int snapshotOpen = open(snapshotPath, O_RDWR, 0644); // deschid snapshotul
	int tmpOpen = open(tmpPath, O_RDWR, 0644); // deschid fisierul temporar

	lseek(snapshotOpen, 0, SEEK_SET); // muta cursorul la inceputul fisierului
	lseek(tmpOpen, 0, SEEK_SET); 

	char c_tmp = 'a', c_snapshot = 'a'; // citim cate un caracter din fiecare fisier
	while (c_tmp != '\n') {
		read(tmpOpen, &c_tmp, sizeof(char)); // sarim peste linia cu timpul curent
	}
	while (c_snapshot != '\n') { // sarim peste linia cu timpul curent
		read(snapshotOpen, &c_snapshot, sizeof(char)); 
	} // sarim peste linia cu timpul curent

	int ok = 1;
	while ((read(tmpOpen, &c_tmp, sizeof(char)) > 0) && (read(snapshotOpen, &c_snapshot, sizeof(char)) > 0)) { // citim cate un caracter din fiecare fisier
		if (c_tmp != c_snapshot)
		{
			ok = 0; // cat timp caracerele sunt la fel in ambele fisiere, citim
			break;
		}
	}

	if (ok == 0) {// Ok este 0 doar daca un caracter nu coincide in ambele fisiere
		printf("%s -snapshotPath\n%s -tmpPath\n\n", snapshotPath, tmpPath); // afisam path-urile fisierelor
		remove(snapshotPath);
		rename(tmpPath, snapshotPath); // daca snapshotul s-a schimbat, stergem snapshotul vechi si redenumim fisierul temporar
	}
	else {
		remove(tmpPath); // Daca snapshotul nu s-a schimbat, stergem fisierul temporar
	}
	close(snapshotOpen);
	close(tmpOpen); // inchidem fisierele
}


// Functie care verifica daca un fisier are permisiuni
void checkDir(char *path, char *outputDir, char *safeDir) {
	DIR *dir;
	struct dirent *entry;
	struct stat file_info;
	char dirPath[1000];

	if ((dir = opendir(path)) == NULL) { // Deschidem directorul
		printf("%s\n", path);
		perror("OpenDir Error!\n");
		return;
	}

	while ((entry = readdir(dir)) != NULL) { // Citim fiecare fisier din director
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { // Daca fisierul este . sau .., sarim peste el
			continue;
		}

		strcpy(dirPath, path); // Path-ul fisierului
		strcat(dirPath, "/"); // Adaugam / la path
		strcat(dirPath, entry->d_name); // Adaugam numele fisierului la path

		if (stat(dirPath, &file_info) == -1) { // Obține informații despre fișier
			perror("Stat Error!\n");
			continue; // Daca nu putem obtine informatii despre fisier, sarim peste el
		}

		if (S_ISDIR(file_info.st_mode)) {
			checkDir(dirPath, outputDir, safeDir); // Daca fisierul este director, apelam recursiv functia
		} else {
			// TODO
			if (S_ISREG(file_info.st_mode)) { // Daca fisierul este fisier
				if (strstr(entry->d_name, "_snapshot"))
					continue;

				int userNoPermission = (file_info.st_mode & S_IRWXU) == 0; // Verificam daca fisierul are permisiuni
				int groupNoPermission = (file_info.st_mode & S_IRWXG) == 0; // Verificam daca fisierul are permisiuni
				int othersNoPermission = (file_info.st_mode & S_IRWXO) == 0; // Verificam daca fisierul are permisiuni

				if (userNoPermission && groupNoPermission && othersNoPermission) { // Daca fisierul nu are permisiuni
					int pipes[2]; 

					if (pipe(pipes) == -1) { 
						perror("Pipe Error!\n");
						continue; // Daca nu putem crea pipe, sarim peste fisier
					}

					pid_t pid = fork(); // Creem un proces copil

					if (pid == 0) {
						close(pipes[0]); // Inchidem capatul de citire al pipe-ului

						dup2(pipes[1], STDOUT_FILENO); // Redirectionam iesirea standard catre pipe

						execl(
							"/bin/bash",
							"/bin/bash",
							"script.sh",
							dirPath,
							NULL);

						perror("Failed to run script!\n"); // Daca nu putem rula script-ul, afisam eroare
						exit(1);
					} else if (pid < 0) { // Daca nu putem crea procesul copil, afisam eroare
						perror("Fork Error!\n");
						continue;
					} else { // Daca am creat procesul copil
						close(pipes[1]);

						char buffer[1000];
						read(pipes[0], buffer, sizeof(buffer)); // Citim din pipe
						close(pipes[0]); // Inchidem capatul de scriere al pipe-ului

						if (strcmp(buffer, "SAFE") == 0) { // Daca script-ul a returnat SAFE
							continue; // Sarim peste fisier
						}

						char *safeDirPath = malloc(strlen(safeDir) + strlen(entry->d_name) + 2); // Path-ul directorului sigur
						strcpy(safeDirPath, safeDir); // Copiem path-ul directorului sigur
						strcat(safeDirPath, "/"); 
						strcat(safeDirPath, entry->d_name); // Adaugam numele fisierului la path-ul directorului sigur

						if (rename(dirPath, safeDirPath) == -1) { 
							perror("Rename Error!\n");
						}
					}

					continue;
				}

				char snapshotName[1000];
				strcpy(snapshotName, strtok(entry->d_name, ".")); // Iau numele fisierului fara extensie
				strcat(snapshotName, "_snapshot.txt"); // Adaug _snapshot.txt la numele fisierului

				char snapshotPath[1000];
				strcpy(snapshotPath, outputDir); // Path-ul unde se va salva snapshotul
				strcat(snapshotPath, "/"); 
				strcat(snapshotPath, snapshotName); // Adaug numele fisierului la path

				if (access(snapshotPath, F_OK) == -1) { // verific daca exista fisierul. daca nu exista il creez cu  open si scriu in el cu write
					int snapshotOpen = open(snapshotPath, O_RDWR | O_CREAT, 0644); // 0644 permite read si write pentru owner si doar read pentru ceilalti
					takeSnapshot(dirPath, outputDir, entry);
					close(snapshotOpen);
				} else {
					verificare_snapshot(path, dirPath, entry, outputDir); // Daca fisierul exista, verificam daca s-a schimbat
				}
			}
		}
	}

	closedir(dir);
}


int main(int argc, char *argv[]) {
	// Verficam sa avem cel putin 4 argumente
	if (argc < 4) {
		printf("Argc Error!\n");
		return 1;
	}

	// Verificam sa nu avem mai mult de 10 argumente
	if (argc > 12) {
		printf("Too many arguments!\n");
		return 1;
	}

	if (strcmp(argv[1], "-o") != 0) { // Verificam daca primul argument este -o
		printf("String Error!\n");
		return 1;
	}

	if (strcmp(argv[3], "-s") != 0) { // Verificam daca al treilea argument este -s
		printf("String Error!\n");
		return 1;
	}

	// Verirficare proceselor copil
	for (int i = 5; i < argc; i++) { // Pentru fiecare fisier
		pid_t pid = fork();
		if (pid == 0) {
			checkDir(argv[i], argv[2], argv[4]); // Verificam daca fisierul are permisiuni
			exit(2);
		} else if (pid < 0) {
			printf("Fork Error\n"); // Daca nu putem crea procesul copil, afisam eroare
			exit(-2);
		}
	}

	for (int i = 5; i < argc; i++) { 
		int status;
		pid_t pid = waitpid(-1, &status, 0); // Asteptam ca procesul copil sa se termine

		if (pid < 0) {
			printf("Waiting Error!\n");
			exit(-2);
		}

		if (WIFEXITED(status)) { // Daca procesul copil s-a terminat
			printf("Process %d has ended with status %d\n", pid, WEXITSTATUS(status)); // Afisam statusul procesului
		} else {
			printf("eroare incheiere proces\n");
		}
	}

	return 0;
}
