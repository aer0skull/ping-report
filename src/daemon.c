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
#include "../include/db-sqlite.h"

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
    -- ping_request --
    Desc :
        Ping the DNS (1.1.1.1) then write the ping in log file
    In-param :
        None
    Out-param :
        None
    Return value :
        None
*/
static void ping_request(){
    
    /* Variables */    
    char* ping;
    static char command[128] = "";

    if(!strcmp(command,"")){
        /* Create ping command (with output in filename) */
        (void) snprintf(command,128,"ping -c 1 1.1.1.1 > %s",get_last_ping());
    }

    /* ping command */
    (void) system(command);
    /* Get ping value as a string */
    ping = get_ping_from_temp_log();
    if(ping != NULL) {
        /* Write ping in all-ping.log */
        write_ping_log(ping);
    }

}

/*
    -- send_check --
    Desc :
        Send if send_stats_ping is needed and if so, do it
    In-param :
        None
    Out-param :
        None
    Return value :
        None
*/
static void send_check(){

    /* Variables */
    time_t t;
    struct tm* utc_time;
    static int flag = 1;

    /* Get time */
    t = time(NULL);
    utc_time = localtime(&t);

    /* Set flag to avoid sending multiple time the same data at HH:00 */
    if((utc_time->tm_min != 0)&& (flag == 0)){
        flag = 1;
    }

    /* if time == HH:00, insert stats in db */
    if((utc_time->tm_min == 0) && (flag != 0)){
        set_stats_ping();
        remove_file(get_all_ping());
        flag = 0;
    }

}

/*
    -- check_keep_working --
    Desc :
        Check if ping-report need to be ended
    In-param :
        None
    Out-param :
        None
    Return value :
        keep_working : 1 continue, 0 end
*/
static int check_keep_working(){

    /* Variables */
    FILE* fd;
    char read_line[5];

    fd = fopen(STATUS_LOG,"r");

    if(fd == NULL){
        // Error occured while reading status log file, stop ping-report
        printf("Err\n");
        return 0;
    }

    /* Read file */
    if(fread(read_line,sizeof(char),4,fd) != 0){
        
        (void) fclose(fd);
        read_line[4] = '\0';

        if(strcmp(read_line,"STOP") == 0){
            // stop ping-report
            printf("Stop\n");
            return 0;
        }else{
            // continue ping-report
            printf("%s : Continue\n",read_line);
            return 1;
        }
    }else{
        // Error occured while reading status log file, stop ping-report
        printf("Nothing to read\n");
        (void) fclose(fd);
        return 1;
    }
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
    int ping_interval;

    /* Init utils globals */
    if(init_globals() != 0){
        return;
    }

    /* Write daemon pid in log file */
    write_pid_file();

    /* ping sleep time (from config file) */
    ping_interval = get_ping_interval();

    /* Connect db sqlite */
    if(db_connect()){
        return;
    }
    /* Main loop */
    while(keep_working != 0){
       
        /* Launch ping command */
        ping_request();

        /* Send stats if time is correct */    
        send_check();

        /* Check end ping-report */
        keep_working = check_keep_working();

        /* ping_interval */
        usleep(ping_interval*1000);

    }

    /* Disconnect sqlite db */
    db_disconnect();
}
