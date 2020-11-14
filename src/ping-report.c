#include "../include/daemon.h"

/*
    -- main --
    Desc :
        Main function
    In-param :
        argc : argument count
        argv : argument list
    Out-param :
        None
    Return value :
        0 : Normal end of program
        1 : Error while creating daemon
        2 : Parent process quit
        3 : Unknown error
*/
int main(/*int argc, char** argv*/){

    /* Daemon creation */
    switch(create_daemon()){
         case 0:
            /* Daemon execution */
            daemon_work();
            break;
        case -1:
            /* Error : quit program */
            return 1;
        case 1:
            /* Parent process : quit program */
            return 2;
        default:
            return 3;
    }

    return 0;

}
