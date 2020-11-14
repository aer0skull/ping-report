#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/daemon.h"
#include "../include/utils.h"
#include "../include/stats.h"

/*
    -- create_daemon --
    Desc :
        Function which create a daemon
    In-param :
        None
    Out-param :
        None
    Return value :
        -1 : Error fork
        0  : Daemon branch
        1  : Parent process branch

*/
int create_daemon(){

    /* Variables */
    pid_t pid;
    int fd;

    /* Fork a new process */
    pid = (pid_t) fork();

    if(pid < 0){
        /* Error while forking */
        /* todo : add log */
        return -1;
    }
    if(pid > 0){
        /* Parent process branch : quit function */
        return 1;
    }

    /* Son process branch */

    /* Set file permissions */
    (void) umask(0);

    /* Change working directory to root directory */
    (void) chdir("/");

    /* Close all open file descriptors */
    for(fd = (int) sysconf(_SC_OPEN_MAX); fd >= 0; fd--){
        (void) close(fd);
    }

    return 0;
}

/*
    -- daemon_work --
    Desc :
        Function which contain main loop of the daemon
    In-param :
        None
    Out-param :
        None
    Return value :
        None
*/
void daemon_work(){

    /* Variables */
    int keep_working = 1;
    char* ping = NULL;
    char command[128];
    int flag = 1;
    time_t t;
    struct tm* utc_time;
    stats_ping stats;

    set_stats_ping_default(&stats);

    if(init_globals() != 0){
        return;
    }

    /* Create ping command (with output in filename) */
    strcpy(command,"ping -c 1 1.1.1.1 > ");
    strcat(command,get_last_ping());

    /* Main loop */
    while(keep_working != 0){
        /* Ping request */
        (void) system(command);
        /* Get ping value as a string */
        ping = get_ping_from_temp_log();
        if(ping != NULL) {
            /* Write ping in all-ping.log */
            write_ping_log(ping);
        }

        /* Get time */
        t = time(NULL);
        utc_time = localtime(&t);

        /* Set flag to avoid sending numerous mail at HH:00 */
        if((utc_time->tm_min != 0)&& (flag == 0)){
            flag = 1;
        }

        /* if time = HH:00, send mail */
        if((utc_time->tm_hour == 0) && (utc_time->tm_min == 0) && (flag != 0)){
            /* Get ping stats */
            get_stats_ping(&stats);
            /* Remove all-ping.log file */
            remove_file(get_all_ping());
            /* Send mail */
            send_stats_mail(&stats);
            /* Set flag to avoid sending numerous mail at HH:00 */
            flag = 0;
        }
    }
}
