#ifndef STATS_H
#define STATS_H

/*@null@*/char* get_ping_from_temp_log();
void write_ping_log(char* new_ping);
void set_stats_ping();
void send_stats_mail();
void set_stats_ping_default();

double get_max();
double get_min();
double get_mean();
int get_high();
int get_loss();
int get_reached();

#endif
