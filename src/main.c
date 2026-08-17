#include <mpv/client.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <ncurses.h>
#define EFFECT "\x1b[30;44m"
#define RESET "\x1b[0m"
#define EFFECT_BG "\x1b[37;40m"

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

void curseyou(char *file, mpv_handle *fmp) {
    WINDOW *win = initscr();
    cbreak();
    noecho();
    int key;
    char *filename = strrchr(file, '/'); printw("Now Playing: %s ", filename);    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    double ispeed = 1.0; double dspeed = 1.0;
    mvprintw(max_y - 1, 0, "'Q': quit | '[': seek -5 | ']': seek 5 | 'k': decrease speed | 'l': increase speed");
    while ((key = getch()) != 'q') {
        switch(key) {
            case ']':
                mpv_command_string(fmp, "seek 5 relative+exact");
                break;
            case '[':
                mpv_command_string(fmp, "seek -5 relative+exact");
                break;
            case 'l': {
                ispeed += 0.1;
                mpv_set_property(fmp, "speed", MPV_FORMAT_DOUBLE, &ispeed);
                break;
            }
            case 'k': {
                dspeed -= 0.1;
                mpv_set_property(fmp, "speed", MPV_FORMAT_DOUBLE, &dspeed);
                break;
            }
        }
    wrefresh(win); refresh();
    }
    exit(0);
}


int play(char *file) {
    mpv_handle *fmp = mpv_create();
    mpv_initialize(fmp);
    char *c = cmdgen(file);
    const char *command[] = {"loadfile", file};
    mpv_set_option_string(fmp, "video", "no");
    mpv_command(fmp, command);
    while(1) { 
        curseyou(file, fmp);
    }
}

int main(int argc, char **argv) {    
    check_file(argv[1]);
    if (cmdgen(argv[1]) == "file") {
        play(argv[1]);
    } else if (cmdgen(argv[1]) == "dir") {
        displayqueue(argv[1]);
        play(argv[1]);
    }  
}
