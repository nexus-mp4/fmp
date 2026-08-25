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

void spawnbox() {
    WINDOW *win = newwin(9, 30, 13, 57); 
    
    wborder(win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
            ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    
    mvwprintw(win, 1, 1, " 'K': decrease speed");
    mvwprintw(win, 2, 1, " 'L': increase speed");
    mvwprintw(win, 3, 1, " '[': seek 5");
    mvwprintw(win, 4, 1, " ']': seek -5");
    mvwprintw(win, 5, 1, " '>': next");
    mvwprintw(win, 6, 1, " '<': previous");
    
    wgetch(win);
    werase(win);
    wrefresh(win);
    delwin(win);
}

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
        if (title) result[1] = strdup(title->value);                                   
        if (artist) result[2] = strdup(artist->value);                                  
        avformat_close_input(&ctx);                                             
    }                                                                           
    return result;                                                              
}


void check_file(char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file.\n");
        exit(1);
    }   
}

int cmdgen(char *path) {
    int ret;
    DIR *dp = opendir(path);
    FILE *fp = fopen(path, "r");
    if (dp != NULL) {
        ret = 0;
    } else if (fp != NULL) {
        ret = 1;
    }
    return ret;
}

void curseyou(char *file, mpv_handle *fmp) {
    WINDOW *win = initscr();
    cbreak();        
    noecho();
    curs_set(0);
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
    mvprintw(max_y - 1, 0, "'Q': quit | SPACEBAR: pause/play | 'H': keybinds");
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
            case 'h': {
                spawnbox(); 
                break;
            }
            case '.': {
                const char *cmd[] = {"playlist-next", NULL};
                mpv_command(fmp, cmd);
            }
            case ',': {
               const char *cmd[] = {"playlist-previous", NULL};
               mpv_command(fmp, cmd);
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
    const char *command[] = {"loadfile", file, NULL};
    mpv_set_option_string(fmp, "video", "no");
    mpv_command(fmp, command);
    while(1) { 
        curseyou(file, fmp);
    }
}

int main(int argc, char **argv) {    
    check_file(argv[1]);
    play(argv[1]);
}
