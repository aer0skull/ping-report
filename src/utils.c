#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "../include/utils.h"

/* File globals */
static const char CONF_FILE[] = "/etc/ping-report/ping-report.conf";
static configuration configs = {NULL, 0};

/*
    -- get_ping_interval --
    Desc :
        Get ping interval from configuration
    In-param :
        None
    Out-param :
        None 
    Return value :
        None 
*/
int get_ping_interval(){
    return configs.ping_interval;
}

/*
    -- set_configuration --
    Desc :
        Set configs from config file data
    In-param :
        None
    Out-param :
        None 
    Return value :
        None 
*/
void set_configuration(){
    
    /* Variables */
    FILE* fd = NULL;
    char* config_line = NULL;
    char* string_value = NULL;
    int   int_value = 0;

    fd = fopen(CONF_FILE,"r");

    if(fd != NULL){
        config_line = (char *) malloc(128*sizeof(char));
        if(config_line != NULL){
            string_value = (char *) malloc(128*sizeof(char));
            if(string_value != NULL){
                while(fgets(config_line, 128, fd) != NULL){
                    if(config_line[0] == '#'){
                        // Comment line : ignore it
                    }else{
                        if(sscanf(config_line,"log_file_dir=%s\n",string_value) == 1){
                            if(configs.log_file_dir == NULL){
                                configs.log_file_dir = strdup(string_value);
                                if(configs.log_file_dir == NULL){
                                    // Error, quit function
                                    break;
                                }
                            }
                        }
                        if(sscanf(config_line,"ping_interval=%d\n",&int_value) == 1){
                                configs.ping_interval = int_value;
                        }
                    }
                }
                free(string_value);
            }
            free(config_line);
        }
        (void) fclose(fd);
    }
}

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
    char log_filename[32];

    (void) snprintf(log_filename,32,"%s/pid.log",configs.log_file_dir);

    fd = fopen(log_filename,"w+");

    if(fd == NULL){
        return;
    }
    
    (void) snprintf(pid_str,16,"%d",(int) pid);
    (void) fwrite(pid_str,strlen(pid_str),1,fd);
    (void) fclose(fd);

}

/*
    -- get_all_ping --
    Desc :
        return ALL_PING variable
    In-param :
        None
    Out-param :
        None
    Return value :
        ALL_PING pointer
*/
char* get_all_ping(){
    static char ALL_PING[64] = "\0";
    if(ALL_PING[0] == '\0'){
        (void) snprintf(ALL_PING,64*sizeof(char),"%s/all-ping.log",configs.log_file_dir);
    }
    return ALL_PING;
}

/*
    -- get_last_ping --
    Desc :
        return LAST_PING variable
    In-param :
        None
    Out-param :
        None
    Return value :
        LAST_PING pointer
*/
char* get_last_ping(){
    static char LAST_PING[64] = "\0";
    if(LAST_PING[0] == '\0'){
        (void) snprintf(LAST_PING,64*sizeof(char),"%s/last-ping.log",configs.log_file_dir);
    }
    return LAST_PING;
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
        strcpy(remove_cmd,"rm -f ");
        strcat(remove_cmd,filename);        
        (void) system(remove_cmd);

}

/*
    -- init_globals --
    Desc :
        Init globals variables
    In-param :
        None
    Out-param :
        None
    Return value :
        0 on succes, 1 when an error occured
*/
int init_globals(){
    set_configuration();
    (void)get_all_ping();
    (void)get_last_ping();
    return 0;
}

