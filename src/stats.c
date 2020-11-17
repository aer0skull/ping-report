#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>

#include "../include/utils.h"
#include "../include/stats.h"
#include "../include/db-sqlite.h"

/*
    -- set_stats_ping_default --
    Desc :
        Set default values for stats_ping struct
    In-param :
        None
    Out-param :
        stats : stats_ping struct to be initialized
    Return value :
        None
*/
void set_stats_ping_default(/*@out@*/stats_ping* stats){

    stats->sum = 0.0;
    stats->max = 0.0;
    stats->min = 100.0;
    stats->mean = 0.0;
    stats->nb_high = 0;
    stats->nb_loss = 0;
    stats->nb_ping = 0;

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
/*@null@*/char* get_ping_from_temp_log(){

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
    fd = fopen(get_last_ping(),"r");
    if(fd == NULL){
        free(p_reg);
        return ping; /* NULL */
    }

    /* Construct regex to get ping from log file */
    if(regcomp(p_reg,"time=(.*) ms",REG_EXTENDED) != 0){
        if(p_reg != NULL){
            free(p_reg);
        }
        (void) fclose(fd);
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
                read_line = NULL;
                n = 0;
                break;
            }

            /* Create ping string */
            (void) strncpy(ping, &read_line[start], size_ping);
            ping[size_ping]='\n';
            ping[size_ping+1]='\0';

            /* Free memory */
            free(read_line);
            read_line = NULL;
            n = 0;
            break;
        }

        free(read_line);
        read_line = NULL;
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
void write_ping_log(char* new_ping){
    
    /* Variables */
    FILE* fd;

    /* Open log file */
    fd = fopen(get_all_ping(),"a+");
    
    if(fd != NULL){
        if(new_ping == NULL){
            new_ping = (char *) malloc(5*sizeof(char));
            if(new_ping == NULL){
                (void) fclose(fd);
                return;
            }
            (void) snprintf(new_ping,5*sizeof(char),"LOSS");
        }
        (void) fwrite(new_ping, sizeof(char), strlen(new_ping), fd);
        (void) insert_ping(strtod(new_ping,NULL));
        (void) fclose(fd);
    }else{
        perror("write ping : ");
    }

    free(new_ping);
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
void send_stats_mail(stats_ping *stats){
    
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
void get_stats_ping(stats_ping *stats){
    
    /* Variables */
    double ping = 0.0;
    FILE* fd;
    char* read_line = NULL;
    size_t n = 0;

    /* Open log file */
    fd = fopen(get_all_ping(),"r");
    
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


