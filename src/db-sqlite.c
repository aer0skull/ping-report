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
    -- insert_ping --
    Desc :
        Insert ping in db
    In-param :
        ping value
    Out-param :
        None
    Return value :
        sqlite3_exec rc
*/
int insert_ping(double ping){
    
    int rc = 0;
    char statement[64];

    (void) snprintf(statement,64,"INSERT INTO ping VALUES (%lf,%d)",ping,(int) time(NULL));

    rc = sqlite3_exec(db,statement,NULL,NULL,NULL);

    return rc;
}

/*
    -- insert_ping_stats --
    Desc :
        Insert ping stats in db
    In-param :
        ping stats
    Out-param :
        None
    Return value :
        sqlite3_exec rc
*/
int insert_ping_stats(stats_ping stats){
    
    int rc = 0;
    char statement[128];

    (void) snprintf(statement,128,"INSERT INTO ping_stats VALUES (%lf,%lf,%lf,%d,%d,%d,%d)",
                    stats.max,
                    stats.min,
                    stats.mean,
                    stats.nb_high,
                    stats.nb_loss,
                    stats.nb_ping,
                    (int) time(NULL));

    rc = sqlite3_exec(db,statement,NULL,NULL,NULL);

    return rc;
}



