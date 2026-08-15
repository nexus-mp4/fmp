#include <mpv/client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#define EFFECT "\x1b[30;44m"
#define RESET "\x1b[0m"

void displayqueue(char *path) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    if ((entry = readdir(dir)) != NULL) {
        printf ( EFFECT "queue:" RESET);
    } 
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
}

void check_file(char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file.\n");
        exit(1);
    }   
}

char *cmdgen(char *path) {
    char *ret;
    DIR *dp = opendir(path);
    FILE *fp = fopen(path, "r");
    if (dp != NULL) {
        ret = "dir";
    } else if (fp != NULL) {
        ret = "file";
    }
    return ret;
}

int play(char *file) {
    mpv_handle *fmp = mpv_create();
    mpv_initialize(fmp);
    char *c = cmdgen(file);
    const char *command[] = {"loadfile", file};
    mpv_set_option_string(fmp, "video", "no");
    mpv_command(fmp, command);
    while(1);
}

int main(int argc, char **argv) {    
    check_file(argv[1]);
    char *filename = strrchr(argv[1], '/');
    //memmove(filename, filename + 1, strlen(filename));
    if (cmdgen(argv[1]) == "file") {
        printf("Now playing: %s \n", filename);
    } else if (cmdgen(argv[1]) == "dir") {
        displayqueue(argv[1]);
    }   
    play(argv[1]);
}
