#include <errno.h>
#include <stddef.h>
#include <string.h>

//FileIO 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     //close(), open()

//Syslog
#include <syslog.h>

int main(int argc, char * argv[])
{
    //Open syslogs to be able to print log and error messages
    openlog(NULL, 0 /* no options set */, LOG_USER /* facility set to user */);

    //Handle arguments - must be at least 3 as 1st is script name
    if(argc < 3) 
    {
        //Too little arguments supplied
        syslog(LOG_ERR, "Too little arguments.");
        return 1;
    }

    //Assign the correct arguments
    char * pFile = argv[1];
    char * pString = argv[2];

    if(NULL == pFile || NULL == pString)
    {
        //Invalid arguments
        syslog(LOG_ERR, "Some arguments are invalid.");
        closelog();
        return 1;
    }

    int fd = 0;
    u_int32_t numBytesWritten = 0;
    int creationFlags = O_WRONLY    // Write
                        | O_CREAT   // Create if does not exist
                        | O_TRUNC;  // Overwrite

    int permissionFlags =  S_IWUSR  // User Write
                        | S_IRUSR   // User Read
                        | S_IRGRP   // Group Read
                        | S_IWGRP;  // Group Write

    //Create file at specified path
    fd = open(  pFile, creationFlags, permissionFlags);


    if(-1 == fd)
    {
        syslog(LOG_ERR, "Open file %s failed due to: %s", pFile, strerror(errno));
        closelog();
        return 1;
    }

    //Write string to file
    numBytesWritten = write(fd, pString, strlen(pString));

    if(numBytesWritten < 0)
    {
        syslog(LOG_ERR, "Writing to file %s failed due to: %s", pFile, strerror(errno));
        close(fd);
        closelog();
        return 1;
    }

    //Write log msg
    syslog(LOG_DEBUG, "Writing \"%s\" to \"%s\"", pString, pFile);

    //Close file
    close(fd);

    closelog();

}
