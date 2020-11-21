#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "../include/utils.h"

/*
    -- write_pid_file --
    Desc :
        write pid in a log file
    In-param :
        None
    Out-param :
        None
    Return value :
        None
*/
void write_pid_file(){

    /* Variables */
    FILE* fd;
    pid_t pid = (pid_t) getpid();
    char pid_str[16];

    fd = fopen("/var/log/ping-report/pid.log","w+");

    if(fd == NULL){
        return;
    }
    
    (void) snprintf(pid_str,16,"%d",(int) pid);
    (void) fwrite(pid_str,strlen(pid_str),1,fd);
    (void) fclose(fd);

}

/*
    -- remove_file --
    Desc :
        Remove a file from filesystem
    In-param :
        filename : the name of the file to remove
    Out-param :
        None
    Return value :
        None
*/
void remove_file(char* filename){

        /* Variable */
        char remove_cmd[128];
        
        /* remove file */
        (void) snprintf(remove_cmd, 128, "rm -f %s",filename);        
        (void) system(remove_cmd);

}

