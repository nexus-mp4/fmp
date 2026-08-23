#include <mpv/client.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <ncurses.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#define EFFECT "\x1b[30;44m"
#define RESET "\x1b[0m"
#define EFFECT_BG "\x1b[37;40m"


char** get_audio_metadata(const char* filepath) {
    AVFormatContext* ctx = NULL;
    char** result = malloc(3 * sizeof(char*));
    char* filename = strrchr(filepath, '/');
    if (!filename) filename = strrchr(filepath, '\\');
    result[0] = strdup(filename ? filename + 1 : filepath);
    result[1] = result[2] = NULL;
    if (avformat_open_input(&ctx, filepath, NULL, NULL) == 0) {
        AVDictionaryEntry* title = av_dict_get(ctx->metadata, "title", NULL, 0);
        AVDictionaryEntry* artist = av_dict_get(ctx->metadata, "artist", NULL, 0);
        if (title && artist) {
            result[1] = strdup(title->value);
            result[2] = strdup(artist->value);
        }
        avformat_close_input(&ctx);
    }
    return result;
}

void displayqueue(char *path) {
    DIR *dir = opendir(path);
    struct dirent *entry;
    if ((entry = readdir(dir)) != NULL) {
        printw ( EFFECT "queue:" RESET);
    }
    while ((entry = readdir(dir)) != NULL) {
        printw("%s\n", entry->d_name);
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
    char **data = get_audio_metadata(file);
    if (data[1] && data[2]) {
        mvprintw(0, 2, "%s - %s ", data[1], data[2]);
    } else {
        mvprintw(0, 2, "%s ", data[1]);
    }

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    double speed = 1.0;
    mvprintw(max_y - 1, 0, "'Q': quit | SPACEBAR: pause/play | '[': seek -5 | ']': seek 5 | 'k': decrease speed | 'l': increase speed");
    while ((key = getch()) != 'q') {
        switch(key) {
            case ']':
                mpv_command_string(fmp, "seek 5 relative+exact");
                mvprintw(3, 0, " +5s");
                break;
            case '[':
                mpv_command_string(fmp, "seek -5 relative+exact");
                mvprintw(3, 0, " -5s");
                break;
            case 'l': {
                speed += 0.1;
                mpv_set_property(fmp, "speed", MPV_FORMAT_DOUBLE, &speed);
                mvprintw(2, 0, "speed: %.1f", speed);
                break;
            }
            case 'k': {
                speed -= 0.1;
                mpv_set_property(fmp, "speed", MPV_FORMAT_DOUBLE, &speed);
                mvprintw(2, 0, "speed: %.1f", speed);
                break;
            }
            case ' ': {
                mpv_command_string(fmp, "cycle pause");
                break;
            }
        }
    wrefresh(win); refresh();
    }
    clear();
    exit(0);
}


int play(char *file) {
    mpv_handle *fmp = mpv_create();
    mpv_initialize(fmp);
    char *c = cmdgen(file);
    const char *command[] = {"loadfile", file, NULL};
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
        play(argv[1]);
        displayqueue(argv[1]);
    }
}
