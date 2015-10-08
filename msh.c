/*
*
* The program implements a very basic job control feature for a shell.
*
*
*
* Created by: Asad Zia
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdbool.h>

#define JOB_STATUS_NEW 0x01
#define JOB_STATUS_RUNNING 0x02
#define JOB_STATUS_TERMINATED 0x03

/* the BACKGROUND bool variable is used for checking if & is present*/
bool BACKGROUND = false;
static const char *progname = "msh";


/* the job datatype */
typedef struct job
{
    int num;                /* unique small job number */
    pid_t pid;              /* process id */
    int status;             /* job status, see defines above */
    char *cmd;              /* cmd executed by the job (argv[0]) */
    struct job* next;       /* the next next job*/
    struct job* previous;   /* the previous job*/
} job_t;

/* the Queue datatype for storing the jobs */
typedef struct Queue
{
    int count;       /* the number of jobs which are present in the queue */
    job_t* front;    /* the front queue pointer */
    job_t* rear;     /* the rear queue pointer */
} Queue;

/* making the arbitary jobs queue */
Queue* jobs = NULL;

/*
* the function for adding jobs in to the queue 
*/
void enqueue(job_t* job)
{
    /* in the case when  there are 0 items in the queue*/
    if(jobs->count == 0)
    {
        jobs->front = job;
        jobs->rear = job;
        jobs->count++;
        job->num = 1;
        return;
    }

    /* when there are more than 0 items in the queue*/
    job->previous = NULL;
    job->next = jobs->rear;
    jobs->rear->previous = job;
    jobs->rear = job;

    /* updating the count in the queue and the job datatype*/
    jobs->count++;
    job->num = jobs->count;
}

/*
* the function to remove elements from the queue
*/
void dequeue()
{
    /* if no jobs present int he queue, then
       dequeing is not done. */
    if(jobs->count == 0)
    {
        return;
    }

    /* rearranging the pointers and
       decrementing the count */
    job_t* job = jobs->front;
    jobs->front = job->previous;
    free(job);
    jobs->count--;
}

/*
* function for showing the prompt
*/
static void
show_prompt(void)
{
    printf("%s > ", progname);
    fflush(stdout);
}

static char*
next_token(char **s)
{
    char *token, *p;

    assert(s && *s);

    for (p = *s; *p && isspace(*p); p++) 
    {
        *p = '\0';
    }
    token = p;
    for (; *p && !isspace(*p); p++) ;
    for (; *p && isspace(*p); p++) 
    {
        *p = '\0';
    }
    *s = p;
    return token;
}

/*
* function for deleting a particular job
*/
void deleteJobs(job_t* job)
{
    // if empty
  if(jobs->front == NULL || job == NULL) 
  {
        return;
  }

  // changing the pointer of the front to point to next
  if(jobs->front == job)
  {
        jobs->front = job->next;
  }

  // if the job is in the middle of the queue
  if(job->next != NULL) 
  {
        job->next->previous = job->previous;
  }

  if(job->previous != NULL) 
  {
        job->previous->next = job->next;
  }
  // freeing the job after resetting the pointers
  free(job);
}

/*
*  the function for printing the command
*/
static void
read_command(FILE *stream, int *argc, char ***args)
{
#ifndef BUFLEN
#define BUFLEN 512
#endif
    static char line[BUFLEN];
    static char* argv[BUFLEN];
    char *p;

    assert(argc && args);

    memset((char *) argv, 0, sizeof(argv));
    p = fgets(line, sizeof(line), stream);
    for (*argc = 0; p && *p; (*argc)++) 
    {
        argv[*argc] = next_token(&p);
    }

    // here we check for the presence of the '&'
    if(*argv[(*argc)-1] == '&')
    {
        *argc = *argc -1;
        BACKGROUND = true;
        // BACKGORUND is turned to true if '&' is there
        argv[*argc] = NULL;
    }
    else
    {
        BACKGROUND = false;
    }
    *args = argv;
}

/*
* the function for printing the jobs and the processes
*/
void printing()
{

    job_t* job = jobs->rear;

    //set the state of newly created child to running
    if(job && BACKGROUND)
    {
        printf("[%d] %d\n", job->num, job->pid);
        // once the job starts running in the background
        // we will update the status
        job->status = JOB_STATUS_RUNNING;
        job = job->next;
    }

    pid_t state;
    int status;
    while(job)
    {
        state = waitpid(job->pid, &status, WNOHANG | WUNTRACED);
        if(state == -1)
        {
            return;
            job = job->next;
        }
        else if (state == 0)
        {
            printf("[%d] %d Running           %s\n", job->num, job->pid, job->cmd);
            job = job->next;
            // if the state os greater than 1
        }
        else
        {
            // the status is changed to terminated
            job->status = JOB_STATUS_TERMINATED;
            printf("[%d] %d Terminated        %s\n", job->num, job->pid, job->cmd);
            job_t* Job_new = job->next;
            deleteJobs(job);
            job = Job_new;
        }
    }
}

int main()
{
    // variable declarations
    pid_t pid;
    int status;
    int argc;
    char **argv;

    //intialize job queue by allocating memory
    jobs = (Queue*) malloc(sizeof(Queue));

    while (1) 
    {
        show_prompt();
        read_command(stdin, &argc, &argv);
        if (argv[0] == NULL || strcmp(argv[0], "exit") == 0) 
        {
            break;
        }
        if (strlen(argv[0]) == 0) 
        {
            continue;
        }

        // here i have implemented the functionality of "jobs"
        if(argc == 1 && strcmp(argv[0], "jobs") == 0)
        {
            printing();
            continue;
        }

        // here i have implemented the functionality of "wait"
        else if (strcmp(argv[0], "wait") == 0)
        {

            if(argc == 2)
            {
                int error = waitpid(atoi(argv[1]), NULL, 0);

                if (error == -1)
                {
                    printf("ERROR!! The PID is wrong!\n");
                }
                continue;
            }
        }

        // the fork function is called here to make child process
        pid = fork();

        if (pid == -1) {
            fprintf(stderr, "%s: fork: %s\n", progname, strerror(errno));
            continue;
        }

        if (pid == 0) 
        {			    /* child */
            execvp(argv[0], argv);
            fprintf(stderr, "%s: execvp: %s\n", progname, strerror(errno));
            _exit(EXIT_FAILURE);

        } 
        else 
        {			        /* parent */
            if(BACKGROUND)
            {
              job_t* job = (job_t*) malloc(sizeof(job_t));
              // adding data to the job_t datatype
              job->cmd = argv[0];
              job->previous = NULL;
              job->next = NULL;
              job->status = JOB_STATUS_NEW;
              job->pid = pid;
              enqueue(job); // adding job to the queue
              // printing the job
              printing();
            }
            else
            {
                waitpid(pid, &status, 0);
                printing();
            }
        }
    }
    // freeing the queue memory
    free(jobs);
    return EXIT_SUCCESS;
}
