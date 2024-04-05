/*Se va actualiza functionalitatea programului in asa fel incat acesta sa primeascã un numär nespecificat de argumente in linia de comandã, dar nu mai mult de 10, cu mentinea ca niciun argument nu se va repeta.
Programul va procesa numai directoarele, alte tipuri de argumente vor fi ignorate. Logica de captur a metadatelor se va aplica acum tuturor argumentelor primite valide, ceea ce inseamna că programul va actualiza snapshot-urile pentru toate directorele specificate de utilizator.
* In cazul in care se vor inregistra modificari la nivelul directoarelor, utilizatorul va putea sã compare snapshot-ul anterior al directorului specificat cu cel curent. In cazul in care exist diferente intre cele doua snapshot-uri, snapshot-ul vechi va fi actualizat cu noile informatii din snapshot-ul curent.
* Functionalitate codului va fi extins astfel incât programul sã primeasca un argument suplimentar, care va reprezenta directorul de iesire in care vor fi stocate toate snapshot-urile inträrilor din directorele specificate in linia de comandã. Acest director de iesire va fi specificat folosind optiunea
*-0. De exemplu, comanda de rulare a programului va fi: program _exe -o output input1 input2....*/

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

void checkDir(char *path, char *outputDir) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;
    char pathaux[1000];

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
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
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            checkDir(pathaux, outputDir);
        } else {
            // TODO
			printf("File: %s\n", pathaux);
        }
    }

    closedir(dir);
}

void takeSnapshot(char *dirPath, char *outputDir) {
    DIR *dir;
    struct dirent *entry;
    char snapshotPath[1024];
    FILE *snapshotFile;

    // Open the directory
    if ((dir = opendir(dirPath)) == NULL) {
        perror("opendir");
        return;
    }

    // Create the snapshot file path
    snprintf(snapshotPath, sizeof(snapshotPath), "%s/%s_snapshot.txt", outputDir, dirPath);

    // Open the snapshot file
    snapshotFile = fopen(snapshotPath, "w");
    if (snapshotFile == NULL) {
        perror("fopen");
        return;
    }

    // Write the names of all files and subdirectories to the snapshot file
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            fprintf(snapshotFile, "%s\n", entry->d_name);
        }
    }

    // Close the snapshot file and the directory
    fclose(snapshotFile);
    closedir(dir);
}

int main(int argc, char *argv[]) {
    char *outputDir = NULL;
    int i;

    if (argc > 11) {
        fprintf(stderr, "Too many arguments. Maximum is 10.\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputDir = argv[++i];
            } else {
                fprintf(stderr, "Missing output directory after -o option.\n");
                return 1;
            }
        } else {
            checkDir(argv[i], outputDir);
        }
    }

	takeSnapshot(argv[i], outputDir);

    return 0;
}