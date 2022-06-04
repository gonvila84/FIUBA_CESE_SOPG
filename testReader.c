#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "writer_fifo"
#define BUFFER_SIZE 300

enum operationState {
    success,
    failed,
};

typedef enum operationState operationState_t;
typedef int fileDescriptor_t;
operationState_t createNamedFifo (const char * name);
fileDescriptor_t openNamedFifo (const char * name);
operationState_t readMessage (char * message, int fileDescriptor, int * bytesRead);
fileDescriptor_t fileDescriptor;


int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
	int32_t bytesRead, returnCode, fd;
    
    createNamedFifo (FIFO_NAME);
    fileDescriptor=openNamedFifo(FIFO_NAME);

    
    printf("A writer was detected and ready to send messages.\n");    

	do
	{
        printf("Output> ");
        if (success == readMessage (inputBuffer, fileDescriptor, &bytesRead))
        {
			inputBuffer[bytesRead] = '\0';
			printf("%s\n", inputBuffer);
        }
        else
        {
			perror("read");            
        }
	}
	while (bytesRead > 0);

	return 0;
}

operationState_t createNamedFifo (const char * name)
{
    operationState_t operationState;
    int returnCode = mknod(name, S_IFIFO | 0666, 0);

    switch (returnCode)
    {
        case 0:
            printf("Information: The named fifo was created. (return code: %d).\n", returnCode);
            operationState = success;
        break;

        case -1:
            printf("Information: The named fifo was found. No need to recreate. (return code: %d).\n", returnCode);
            operationState = failed;
        break;

        default:
            printf("Error: The named fifo creation failed. (return code: %d).\n", returnCode);
            operationState = failed;
        break;
    }
    return operationState;
}

fileDescriptor_t openNamedFifo (const char * name)
{
    int fd;
    if ( (fd = open(name, O_RDONLY) ) < 0 )
    {
        printf("Error: There was a failure when opening the FIFO.\n");
        return failed;
    }
    printf("Information: The named fifo was opened correctly.\n");
    
    return fd;
}

operationState_t readMessage (char * message, int fileDescriptor, int * bytesRead)
{
    if ((*bytesRead = read(fileDescriptor, message, BUFFER_SIZE)) == -1)
    {
        return failed;
    }
    return success;
}