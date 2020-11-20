#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

#include "../include/stats.h"
#include "../include/db-sqlite.h"

/* Globals for this file */
static sqlite3 *db = NULL;
static char db_filename[] = "/srv/ping-report/ping-report.db";

/*
    -- db_connect --
    Desc :
        Connect the sqlite db
    In-param :
        None
    Out-param :
        None
    Return value :
        sqlite3_open rc
*/
int db_connect(){
    return sqlite3_open(db_filename,&db);
}

/*
    -- db_disconnect --
    Desc :
        Disconnect the sqlite db
    In-param :
        None
    Out-param :
        None
    Return value :
        sqlite3_close rc
*/
int db_disconnect(){
    return sqlite3_close(db);
}

/*
    -- insert_hourly_report --
    Desc :
        Insert ping hourly stats
    In-param :
        ping value
    Out-param :
        None
    Return value :
        sqlite3_exec rc
*/
int insert_hourly_report(double mean, double max, double min, int high, int loss, int reached){
    
    int rc = 0;
    char statement[128];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    (void) snprintf(statement,128,"INSERT INTO HourlyReport VALUES (%lf,%lf,%lf,%d,%d,%d,'%d-%d-%d',%d)",
                    max,min,mean,high,loss,reached,tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour);

    rc = sqlite3_exec(db,statement,NULL,NULL,NULL);

    return rc;
}
