#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    struct stat file_stat;
    char name[NAME_MAX + 1];
} file_info;

int sizesort(const void * a, const void * b) {
    off_t a1 = ((file_info *)a)->file_stat.st_size;
    off_t b1 = ((file_info *)b)->file_stat.st_size;
    return a1 < b1 ? 1 : a1 > b1 ? -1 : 0;
}

void check_alloc(void * ptr) {
    if (ptr == NULL) {
        perror("Alloc fail");
        exit(1);
    }
}

void ls(char * dir_path) {

    DIR *dir;
    struct dirent *ent;
    char (*dirs)[PATH_MAX] = NULL;
    int dirs_count = 0;
    int file_count = 0;
    file_info * entries = NULL;

    puts(dir_path);

    if ((dir = opendir (dir_path)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            char file_path[PATH_MAX];
            snprintf(file_path, PATH_MAX, "%s/%s", dir_path, ent->d_name);
            entries = realloc(entries, (file_count + 1) * sizeof(file_info));
            check_alloc(entries);
            strncpy(entries[file_count].name, ent->d_name, NAME_MAX);
            if (stat(file_path, &(entries[file_count].file_stat)) < 0) {
                puts(file_path);
                perror("File error");
                exit(1);
            } else {
                if (S_ISDIR(entries[file_count].file_stat.st_mode)) {
                    if (strcmp(".", entries[file_count].name) != 0 && strcmp("..", entries[file_count].name) != 0) {
                        dirs = realloc(dirs, sizeof(*dirs) * (dirs_count + 1));
                        check_alloc(dirs);
                        strncpy(dirs[dirs_count], file_path, PATH_MAX);
                        dirs_count++;
                    }
                }
            }
            file_count++;
        }
        closedir(dir);

        qsort(entries, file_count, sizeof(file_info), sizesort);

        for (int i = 0; i < file_count; i++) {
            printf( S_ISLNK(entries[i].file_stat.st_mode) ? "l" : (S_ISDIR(entries[i].file_stat.st_mode)) ? "d" : "-");
            printf( (entries[i].file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (entries[i].file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (entries[i].file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (entries[i].file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (entries[i].file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (entries[i].file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (entries[i].file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf( (entries[i].file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (entries[i].file_stat.st_mode & S_IXOTH) ? "x" : "-");
            
            struct passwd *pwd = getpwuid(entries[i].file_stat.st_uid);
            if (pwd != NULL) {
                printf(" %s", pwd->pw_name);
            }
            struct group *gr = getgrgid(entries[i].file_stat.st_gid);
            if (gr != NULL) {
                printf(" %s", gr->gr_name);
            }

            printf(" %8llu ", entries[i].file_stat.st_size);
            printf("%s",entries[i].name);
            
            printf("\n");
        }
        free(entries);
        printf("\n");

        for (int i = 0; i < dirs_count; i++) {
            ls(dirs[i]);
        }
        free(dirs);

    } else {
        perror("Cannot open dir");
        fprintf(stderr, "\n");
    }
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        ls(".");
    } else {
        for (int i = 1; i < argc; i++) {
            ls(argv[i]);
        }
    }
}
