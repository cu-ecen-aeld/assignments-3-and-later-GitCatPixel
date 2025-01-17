#include "systemcalls.h"

//error
#include <errno.h>
#include <string.h>

//PROCESS
#include <stdlib.h>

//EXEC
#include <unistd.h>

// MACROS
#include <sys/wait.h>

// SYSTEMLOG
#include <syslog.h>

//FILE stuff
#include <fcntl.h>     //open()
#include <stdio.h>

bool evalChildProcess(int ret)
{
    if(WIFEXITED(ret))
    {
        //The command terminated normally
        int cmdExitCode = WEXITSTATUS(ret);

        if(0 == cmdExitCode)
        {
             syslog(LOG_DEBUG, "cmd execute success.");
             return true;
        }
        else if(127 == cmdExitCode)
        {
            syslog(LOG_ERR, "cmd not found.");
        }
        else if(126 == cmdExitCode)
        {
            syslog(LOG_ERR, "cmd could not be executed (permissions?)");
        }
        else
        {
            syslog(LOG_ERR, "cmd returned errcode: %d", cmdExitCode);
        }
    }
    else if(WIFSIGNALED(ret)) 
    {
        int signalNum = WTERMSIG(ret);

        //The command did not terminate normally - due to signal
        syslog(LOG_ERR, "cmd was aborted by signal: %d", signalNum);
    }
    else
    {
        //cmd did not terminate
        syslog(LOG_ERR, "cmd did not terminate for unspecified reasons.");
    }

    return false;
}

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    bool bRet = false;

    //Open syslogs to be able to print log and error messages
    openlog("do_system", 0 /* no options set */, LOG_USER /* facility set to user */);

    /*
     * Call the system() function with the command set in the cmd
     * and return a boolean true if the system() call completed with success
     * or false() if it returned a failure
     */
    int ret = system(cmd);

    //On success of system(), the return value is the return status of the command 
    if(0 == ret)
    {
        syslog(LOG_DEBUG, "systemcall ok");
        bRet = true;
    }
    else if (-1 == ret)
    {
        //If system() returns -1, the error is in the system() function itself. Use errno to define the cause
        syslog(LOG_ERR, "system() returned error: %s", strerror(errno));

        bRet = false;
    }
    else
    {
        //Child Process failed in some way
        bRet = false;
        evalChildProcess(ret);
    }

    closelog();

    return bRet;
}

char * basename(char * pPath) 
{
    char * pLastMatch = strrchr(pPath, '/');

    // If path contains one or more '/' a pointer to the next char is returned, if no match, return path
    return pLastMatch ? pLastMatch + 1 : pPath;

}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    //Open log and set facility to "do_exec"
    openlog("do_exec", 0, LOG_USER);

#define FOLLOW_UNIX_CONVENTION
#ifdef FOLLOW_UNIX_CONVENTION
    va_list args;
    va_start(args, count);
    char * command[count+2];    //holds: cmd path, cmd name, args, NULL
    int i;

    //Command path
    command[0] = va_arg(args, char *);
    syslog(LOG_DEBUG, "Command path: %s", command[0]);

    //Command name - we add the command's name as first argument of the command, to ensure conformity with Unix convention - see LSP p.141
    command[1] = basename(command[0]);
    syslog(LOG_DEBUG, "Command name: %s", command[1]);

    //Command args - we add the command's args to the vector
    for(i=2; i<(count+1); i++)
    {
        command[i] = va_arg(args, char *);
        syslog(LOG_DEBUG, "Command arg%d: %s", i, command[i]);
    }

    //Command end - We add NULL to mark end of vector
    command[i] = NULL;

#else

    //Original code
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

#endif
/*
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    int ret;
    pid_t pid = fork();

    if(-1 == pid)
    {
        //fork failed
        syslog(LOG_ERR, "fork failed: %s", strerror(errno));
    }
    else if(0 == pid)
    {
        //We are in the child
        //Pass command path, 1st arg = command name, and arguments
        ret = execv(command[0], &command[1]);

        //Why did excv fail?
        perror("execv failed");
        syslog(LOG_ERR, "execv failed with ret: %d, errno-str: %s", ret, strerror(errno));

        //Important: this code is only executed by the child. So on error we must use exit(!0) rather then return
        // then the error will be picked up by wait() in the parent
        exit(errno);
    }

    //We are in the parent
    syslog(LOG_DEBUG, "parent speaking: child's PID: %d", pid);

    //Evaluate the child's result
    int status;
    bool bRet;
    wait(&status);

    bRet = evalChildProcess(status);

    //Clean-up
    va_end(args);
    closelog();

    return bRet;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    openlog("do_exec_redirect", 0, LOG_USER);

#ifdef FOLLOW_UNIX_CONVENTION
    va_list args;
    va_start(args, count);
    char * command[count+2];    //holds: cmd path, cmd name, args, NULL
    int i;

    //Command path
    command[0] = va_arg(args, char *);
    syslog(LOG_DEBUG, "Command path: %s", command[0]);

    //Command name - we add the command's name as first argument of the command, to ensure conformity with Unix convention - see LSP p.141
    command[1] = basename(command[0]);
    syslog(LOG_DEBUG, "Command name: %s", command[1]);

    //Command args - we add the command's args to the vector
    for(i=2; i<(count+1); i++)
    {
        command[i] = va_arg(args, char *);
        syslog(LOG_DEBUG, "Command arg%d: %s", i, command[i]);
    }

    //Command end - We add NULL to mark end of vector
    command[i] = NULL;

#else

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

#endif

/*
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0664);
    if(fd < 0)
    {
        syslog(LOG_ERR, "open output file failed: %s", strerror(errno));
        return false;
    }

    pid_t pid;
    int ret = 0;

    pid = fork();

    if(-1 == pid)
    {
        //fork failed
        syslog(LOG_ERR, "fork failed: %s", strerror(errno));
    }
    else if(0 == pid)
    {
        //We are in the child

        //Redirect STDOUT to outputfile
        //dup2() replaces a file's fd with the 2nd param, if the new fd already refers to an open file, the opern file (here STDOUT) is silently closed
        ret = dup2(fd, 1);      
        if(ret < 0)
        {
            syslog(LOG_ERR, "dup2 failed: %s", strerror(errno));
            return false;
        }

        //Pass command path, 1st arg = command name, and arguments
        ret = execv(command[0], &command[1]);

        //Why did excv fail?
        perror("execv failed");
        syslog(LOG_ERR, "execv failed: %s", strerror(errno));

        //Important: this code is only executed by the child. So on error we must use exit(!0) rather then return
        // then the error will be picked up by wait() in the parent
        exit(errno);
    }

    //We are in the parent
    syslog(LOG_DEBUG, "parent speaking: child's PID: %d", pid);

    //Evaluate the child's result
    int status;
    wait(&status);

    bool bRet = evalChildProcess(status);

    //Clean-up
    closelog();
    va_end(args);

    return bRet;
}
