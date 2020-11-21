#ifndef UTILS_H
#define UTILS_H

#define STATUS_LOG "/var/log/ping-report/status.log"

void write_pid_file();
void remove_file(char* filename);

#endif
