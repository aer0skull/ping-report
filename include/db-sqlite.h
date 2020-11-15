#ifndef DB_SQLITE_H
#define DB_SQLITE_H

int db_connect();
int db_disconnect();
int insert_ping(double ping);
int insert_ping_stats(stats_ping stats);

#endif
