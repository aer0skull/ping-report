#ifndef UTILS_H
#define UTILS_H

typedef struct configuration{
    char *log_file_dir;
    int ping_interval;
} configuration;

int get_ping_interval();
void write_pid_file();
void remove_file(char* filename);
int init_globals();
char* get_all_ping();
char* get_last_ping();

#endif
