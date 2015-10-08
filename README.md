# Job-Control

This is a simple shell program called "msh" which exhibits the feature of job control which is widely present in linux shells. Job control is a feature of shells that allowsyou to run processes concurrently to the shell, i.e., the shell does not wait for the termination of a child process. A shell command is executed under job control if the last argument is a single ’&’ character. The built-in commands jobs and wait can be used to obtain a list of jobs and to wait for the termination of jobs. 

Here is a simple example for the "msh shell":

msh > jobs
msh > sleep 1 &
[1] 27979
msh > sleep 60 &
[1] 27979 terminated sleep
[2] 27980
msh > jobs
[2] 27980 running sleep
msh > wait 27980
msh >

Initially, there are no jobs and hence the jobs command shows nothing on the standard output. Subsequently, a job is created that is sleeping for one second (sleep 1). The shell prints a job number ([1]) followed by the process identifier (27979) of the process executing the job. The user then types another command to create a second job sleeping for 60 seconds. Before executing this command (which leads to the creation of another job), the shell checks whether any pending jobs have terminated. If so, the shell prints the job number ([1]) followed by the process identifier (27979) followed by the job status (terminated) followed by the job’s command (argv[0]).

Subsequently the new job is created and the shell prints the job number ([2]) followed by the process identifier (27980).
The jobs command lists the currently known jobs. For each known job (which may be running or terminated), a line is created on the standard output showing the job number, the process identifier, the job’s status, and the job’s command (argv[0]). 

Finally, the wait command is used to wait for the completion of the job identified by the given process identifier. The wait command without an argument will wait for all pending jobs to terminate.
