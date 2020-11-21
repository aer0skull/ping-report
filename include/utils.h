#ifndef UTILS_H
#define UTILS_H

#define STATUS_LOG "/var/log/ping-report/status.log"

int get_ping_interval();
void write_pid_file();
void remove_file(char* filename);
int init_globals();
char* get_all_ping();
char* get_last_ping();

#endif
