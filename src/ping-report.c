#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <time.h>

/* Globals */

/* struct stats_ping : Handle ping data */
typedef struct stats_ping{
    double sum;
    double max;
    double min;
    double mean;
    int nb_high;
    int nb_loss;
    int nb_ping; 
} stats_ping;

/* stats_ping_default : default value for stats_ping (init) */
static stats_ping stats_ping_default = {
    0.0,    /* sum */
    0.0,    /* max */
    100.0,  /* min */
    0.0,   /* mean */
    0,  /* nb_high */
    0,  /* nb_loss */
    0   /* nb_ping */
};

/*@observer@*/static char HOME[32];
static char ALL_PING[64];
static char LAST_PING[64];

/* End globals */

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
    -- get_ping_from_temp_log --
    Desc :
        Function which get the ping from a temp log containing the last ping did by the program
    In-param :
        None
    Out-param :
        None
    Return value :
        Ping value as a string or NULL if an error occured
*/
/*@null@*/static char* get_ping_from_temp_log(){

    /* Variables */
    FILE* fd = NULL;
    char* read_line = NULL;
    size_t n = 0;
    int rc;
    size_t nmatch = 2;
    regex_t *p_reg;
    regmatch_t* pmatch;
    char* ping = NULL;
    int start;
    int end;
    size_t size_ping;

    /* regex struct memory allocation */
    p_reg = (regex_t *) malloc(sizeof(*p_reg));
    if(p_reg == NULL){
        return ping; /* NULL */
    }

    /* Open ping log file */ 
    fd = fopen(LAST_PING,"r");
    if(fd == NULL){
        free(p_reg);
        return ping; /* NULL */
    }

    /* Construct regex to get ping from log file */
    if(regcomp(p_reg,"time=(.*) ms",REG_EXTENDED) != 0){
        if(p_reg != NULL){
            free(p_reg);
        }
        return ping; /* NULL */
    }    

    /* match info memory allocation */
    pmatch = malloc(sizeof(*pmatch) * nmatch);
    if(pmatch == NULL){
        (void) fclose(fd);
        regfree(p_reg);
        free(p_reg);
        return ping; /* NULL */
    }

    /* Read file */
    while(getline(&read_line,&n,fd) != -1){

        if(read_line == NULL){
            break;
        }

        /* Exec regex to find ping */
        rc = regexec(p_reg,read_line, nmatch, pmatch, 0);

        if(rc == 0){

            /* Extract ping position from read line */
            start = (int) pmatch[1].rm_so;
            end = (int) pmatch[1].rm_eo;
            size_ping = (size_t) (end - start);

            /* ping string memory allocation */
            ping = malloc(sizeof(char) * (size_ping+2));
            if(ping == NULL){
                free(read_line);
                n = 0;
                break;
            }

            /* Create ping string */
            strncpy(ping, &read_line[start], size_ping);
            ping[size_ping]='\n';
            ping[size_ping+1]='\0';

            /* Free memory */
            free(read_line);
            n = 0;
            break;                
        }
        free(read_line);
        n = 0;
    }
    
    /* free allocated memory */
    regfree(p_reg);
    free(p_reg);
    free(pmatch);
    if(read_line != NULL){
        free(read_line);
    }
    (void) fclose(fd);

    /* ping may be null, then it must mean that the ping request was lost */
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

    /* Open log file */
    fd = fopen(ALL_PING,"a+");
    
    if(fd != NULL){
        if(new_ping == NULL){
            new_ping = (char *) malloc(5*sizeof(char));
            if(new_ping == NULL){
                return;
            }
            (void) snprintf(new_ping,sizeof(new_ping),"LOSS");
        }
        (void) fwrite(new_ping, sizeof(char), strlen(new_ping), fd);
        (void) fclose(fd);
    }else{
        perror("write ping : ");
    }

    free(new_ping);
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
static void remove_file(char* filename){

        /* Variable */
        char remove_cmd[128];
        
        /* remove file */
        strcpy(remove_cmd,"rm -f ");
        strcat(remove_cmd,filename);        
        (void) system(remove_cmd);

}

/*
    -- send_stats_mail --
    Desc :
        Send a mail with all ping stats data
    In-param :
        stats : ping stats data
    Out-param :
        None
    Return value :
        None
*/
static void send_stats_mail(stats_ping *stats){
    
    /* Variable */
    char mail_msg[256];
    char dest_mail[] = "maxime.menault@gmail.com";
    char command[512];       
        
    /* Sendmail command */
    (void) snprintf(mail_msg,256,"ping-report\n - Mean = %lf\n - Max = %lf\n - Min = %lf\n - High = %d\n - Loss = %d\n - Reached = %d\n",
                    stats->mean,stats->max,stats->min,stats->nb_high,stats->nb_loss,stats->nb_ping);
    (void) snprintf(command,512,"echo \"%s\" | msmtp %s",mail_msg,dest_mail);
    (void) system(command);
}

/*
    -- get_stats_ping --
    Desc :
        Function which calculate statistics about ping values, from log file.
    In-param :
        None
    Out-param :
        stats : struct with all ping stats data filled
    Return value :
        None
*/
static void get_stats_ping(stats_ping *stats){
    
    /* Variables */
    double ping = 0.0;
    FILE* fd;
    char* read_line = NULL;
    size_t n = 0;


    /* Open log file */
    fd = fopen(ALL_PING,"r");
    
    if(fd != NULL){
        /* Read file */
        while(getline(&read_line,&n,fd) != -1){
            
            /* Check getline error */
            if(read_line == NULL){
                break;
            }

            /* Check if the ping is flagged as LOSS */
            if(strcmp(read_line,"LOSS") == 0){
                stats->nb_loss++;
            }else{
                /* Evaluate the ping as a double */
                ping = strtod(read_line,NULL);
                /* Test null ping */
                if(ping < 0.1){
                    /* Ignore null ping */
                }else{
                    /* Number of ping readed (for mean calculation) */
                    stats->nb_ping++;
                    /* Max ping */
                    if(ping > stats->max){
                        stats->max = ping;
                    }
                    /* Min ping */
                    if(ping < stats->min){
                        stats->min = ping;
                    }
                    /* Number of ping above 100 ms */
                    if(ping > 100.0){
                        stats->nb_high++;
                    }
                    /* Sum (for mean calculation) */
                    stats->sum += ping;
                }
            } 
            free(read_line);
            n = 0;
        }
    
        /* Mean calculation */
        stats->mean = stats->sum / (double) stats->nb_ping;
        (void) fclose(fd);

    }else{
        perror("stats : ");
    }

    if(read_line != NULL){
        free(read_line);
    }
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
static int init_globals(){
    /* Set globals values */
    char *home = getenv("HOME");
    if(home == NULL){
        return 1;
    }

    /* HOME */
    (void) snprintf(HOME,sizeof(HOME),"%s",home);
    
    /* ALL_PING */
    (void) snprintf(ALL_PING,sizeof(ALL_PING),"%s/log/all-ping.log",HOME);

    /* LAST_PING */
    (void) snprintf(LAST_PING,sizeof(LAST_PING),"%s/log/last-ping.log",HOME);
 
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
static void daemon_work(){

    /* Variables */
    int keep_working = 1;
    char* ping = NULL;
    char command[128];
    int flag = 1;
    time_t t;
    struct tm* utc_time;
    stats_ping stats = stats_ping_default;

    if(init_globals() != 0){
        return;
    }
   
    /* Create ping command (with output in filename) */
    strcpy(command,"ping -c 1 1.1.1.1 > ");
    strcat(command,LAST_PING);

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
            remove_file(ALL_PING);
            /* Send mail */
            send_stats_mail(&stats);
            /* Set flag to avoid sending numerous mail at HH:00 */
            flag = 0;
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
