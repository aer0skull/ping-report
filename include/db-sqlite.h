#ifndef DB_SQLITE_H
#define DB_SQLITE_H

int db_connect();
int db_disconnect();
int insert_ping(double ping);
int insert_ping_stats(double mean,double max,double min,int high,int loss,int reached);

#endif
