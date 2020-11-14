#ifndef STATS_H
#define STATS_H

/* struct stats_ping : Handle ping data */
typedef struct stats_ping{
    double sum;
    double max;
    double min;
    double mean;
    int nb_high;
    int nb_loss;
    int nb_ping;
} stats_ping;

/*@null@*/char* get_ping_from_temp_log();
void write_ping_log(char* new_ping);
void get_stats_ping(stats_ping *stats);
void send_stats_mail(stats_ping *stats);
void set_stats_ping_default(/*@out@*/stats_ping* stats);

#endif
