#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"

/*
    -- get_home --
    Desc :
        return HOME variable
    In-param :
        None
    Out-param :
        None
    Return value :
        HOME pointer
*/
static char* get_home(){
    static char HOME[32] = "\0";
    static char DEFAULT_HOME[32] = "/var";
    if(HOME[0] == '\0'){
        char *home = getenv("HOME");
        if(home == NULL){
            return DEFAULT_HOME;
        }
        (void) snprintf(HOME,32*sizeof(char),"%s",home);
    }
    return HOME;
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
        (void) snprintf(ALL_PING,64*sizeof(char),"%s/log/all-ping.log",get_home());
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
        (void) snprintf(LAST_PING,64*sizeof(char),"%s/log/last-ping.log",get_home());
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
    (void)get_home();
    (void)get_all_ping();
    (void)get_last_ping();
    return 0;
}

