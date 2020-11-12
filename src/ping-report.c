#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <time.h>

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
static int create_daemon(){

    /* Variables */
    pid_t pid;
    int fd;

    /* Fork a new process */
    pid = fork();

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
    umask(0);

    /* Change working directory to root directory */
    chdir("/");

    /* Close all open file descriptors */
    for(fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--){
        close(fd);
    }

    return 0;
}

/*
    -- get_ping_from_temp_log --
    Desc :
        Function which get the ping from a temp log containing the last ping did by the program
    In-param :
        None
    Out-param :
        None
    Return value :
        Ping value as a string
*/
static char* get_ping_from_temp_log(){

    /* Variables */
    FILE* fd = NULL;
    char filename[64];
    char* read_line = NULL;
    size_t n = 0;
    regex_t *p_reg = malloc(sizeof(*p_reg));
    int rc;
    size_t nmatch = 2;
    regmatch_t* pmatch;
    char* ping = NULL;
    int start;
    int end;
    size_t size_ping;

    /* Create filename string */
    strcpy(filename,getenv("HOME"));
    strcat(filename,"/log/last-ping.log");

    /* Open ping log file */ 
    fd = fopen(filename,"r");

    /* Construct regex to get ping from log file */
    regcomp(p_reg,"time=(.*) ms",REG_EXTENDED);    
    pmatch = malloc(sizeof(*pmatch) * nmatch);

    if(fd != NULL){

        /* Read file */
        while(getline(&read_line,&n,fd) != -1){

            /* Exec regex to find ping */
            rc = regexec(p_reg,read_line, nmatch, pmatch, 0);

            if(rc == 0){

                /* Extract ping from read line */
                start = pmatch[1].rm_so;
                end = pmatch[1].rm_eo;
                size_ping = end - start;
                ping = malloc(sizeof(char) * (size_ping+2));
                strncpy(ping, &read_line[start], size_ping);
                ping[size_ping]='\n';
                ping[size_ping+1]='\0';
                free(read_line);
                n = 0;
                break;                
            }

            free(read_line);
            n = 0;
        }
    }else{
        regfree(p_reg);
        free(pmatch);
        return NULL;
    }
    
    regfree(p_reg);
    free(p_reg);
    free(pmatch);
    fclose(fd);

    /* if ping is null, then it must mean that the ping request was lost */
    if(ping == NULL){
        ping = malloc(sizeof(char) * 5);
        strcpy(ping,"LOSS");
    }

    return ping;
}

/*
    -- write_ping_log --
    Desc :
        Function which write a given ping in log file
    In-param :
        new_ping : string value of a ping
    Out-param :
        None
    Return value :
        None
*/
static void write_ping_log(char* new_ping){
    
    /* Variables */
    FILE* fd;
    char filename_log[64] = "";

    /* Create filename string */
    strcpy(filename_log,getenv("HOME"));
    strcat(filename_log,"/log/all-ping.log");

    /* Open log file */
    fd = fopen(filename_log,"a+");
    
    if(fd != NULL){
        fwrite(new_ping, sizeof(char), strlen(new_ping), fd);
        fclose(fd);
    }else{
        perror("write ping : ");
    }

    free(new_ping);
}

/*
    -- stats_ping --
    Desc :
        Function which calculate statistics about ping values, from log file, and then send a mail with stats.
    In-param :
        None
    Out-param :
        None
    Return value :
        None
*/
static void stats_ping(){
    
    /* Variables */
    double ping = 0.0;
    double sum = 0.0;
    double max = 0.0;
    double min = 100.0;
    double mean = 0.0;
    int nb_high = 0;
    int nb_loss = 0;
    int nb_ping = 0;
    FILE* fd;
    char filename_log[64] = "";
    char* read_line = NULL;
    size_t n = 0;
    char mail_msg[256];
    char dest_mail[] = "maxime.menault@gmail.com";
    char command[512];
    char remove_cmd[128];

    /* Create filename string */
    strcpy(filename_log,getenv("HOME"));
    strcat(filename_log,"/log/all-ping.log");

    /* Open log file */
    fd = fopen(filename_log,"r");
    
    if(fd != NULL){
        /* Read file */
        while(getline(&read_line,&n,fd) != -1){
            /* Check if the ping is flagged as LOSS */
            if(!strcmp(read_line,"LOSS")){
                nb_loss++;
                free(read_line);
                n = 0;
            }else{
                /* Evaluate the ping as a double */
                ping = strtod(read_line,NULL);
                /* Test null ping */
                if(ping == 0.0){
                    /* Ignore null ping */
                    free(read_line);
                    n = 0;
                }else{
                    /* Number of ping readed (for mean calculation) */
                    nb_ping++;
                    /* Max ping */
                    if(ping > max){
                        max = ping;
                    }
                    /* Min ping */
                    if(ping < min){
                        min = ping;
                    }
                    /* Number of ping above 100 ms */
                    if(ping > 100.0){
                        nb_high++;
                    }
                    /* Sum (for mean calculation) */
                    sum += ping;
                    free(read_line);
                    n = 0;
                }
            }   
        }
    
        if(read_line != NULL){
            free(read_line);
        }
    
        /* Mean calculation */
        mean = sum / (double) nb_ping;
        fclose(fd);

        /* remove all-ping.log */
        strcpy(remove_cmd,"rm -f ");
        strcat(remove_cmd,filename_log);        
        system(remove_cmd);

        /* Sendmail command */
        snprintf(mail_msg,256,"ping-report\n - Mean = %lf\n - Max = %lf\n - Min = %lf\n - High = %d\n - Loss = %d\n - Reached = %d\n",mean,max,min,nb_high,nb_loss,nb_ping);
        sprintf(command, "echo \"%s\" | msmtp %s",mail_msg,dest_mail);
        fprintf(stderr,"%s\n",command);
        system(command);

    }else{
        perror("stats : ");
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
static void daemon_work(){

    /* Variables */
    int keep_working = 1;
    char* ping = NULL;
    char command[128];
    char filename[64];
    time_t t;
    struct tm* utc_time;

    /* Create filename string */
    strcpy(filename,getenv("HOME"));
    strcat(filename,"/log/last-ping.log");
    
    /* Create ping command (with output in filename) */
    strcpy(command,"ping -c 1 1.1.1.1 > ");
    strcat(command,filename);

    /* Main loop */
    while(keep_working){
        /* Ping request */
        system(command);
        /* Get ping value as a string */
        ping = get_ping_from_temp_log();
        if(ping != NULL) {
            /* Write ping in all-ping.log */
            write_ping_log(ping);
        }

        /* Get time */
        t = time(NULL);
        utc_time = localtime(&t);
        /* if time = HH:00, send mail */
        if(utc_time->tm_min == 0){
            stats_ping();
            sleep(60);
        }
    }
}

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
int main(int argc, char** argv){

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
