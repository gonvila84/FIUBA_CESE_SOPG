#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define FIFO_NAME "writer_fifo"
#define BUFFER_SIZE 300

enum operationState {
    success,
    failed,
};

typedef enum operationState operationState_t;
typedef int fileDescriptor_t;
operationState_t createNamedFifo (const char * name);
operationState_t openNamedFifo (const char * name, fileDescriptor_t * fileDescriptor);
operationState_t readMessage (char * message, int fileDescriptor, int * bytesRead);
fileDescriptor_t fileDescriptor;


int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
	int32_t bytesRead, returnCode, fd;

    FILE *signFile;
    FILE *dataFile;
    signFile = fopen("./Sign.txt", "w");
    dataFile = fopen("./Log.txt", "w");

     //Creo el named fifo
    if (failed == createNamedFifo (FIFO_NAME))
    {
		exit(1);
    }
    //Abrir un named fifo
    if (failed == openNamedFifo(FIFO_NAME,&fileDescriptor))
    {
		exit(1);
    }
    printf("A writer was detected and ready to send messages.\n");    

    const char dataPattern[5] = "DATA:";
	do
	{
        printf("Output> ");
        if (success == readMessage (inputBuffer, fileDescriptor, &bytesRead))
        {
			inputBuffer[bytesRead] = '\0';
			printf("%s\n", inputBuffer);
            if (0 == strncmp(inputBuffer,dataPattern,5))
            {
                fprintf(dataFile, "%s\n", inputBuffer);
                fflush(dataFile);
            }
            else
            {
                fprintf(signFile, "%s\n", inputBuffer);
                fflush(signFile);
            }
        }
        else
        {
			perror("read");            
        }
	}
	while (bytesRead > 0);
    fclose(dataFile);
	return 0;
}

operationState_t createNamedFifo (const char * name)
{
    operationState_t operationState;
    int returnCode = mknod(name, S_IFIFO | 0666, 0);

    switch (returnCode)
    {
        case 0:
            printf("Information: The named fifo was created.\n");
            operationState = success;
        break;

        case -1:
            printf("Information: The named fifo was found. No need to recreate.\n");
            operationState = success;
        break;

        default:
            perror("Error: The named fifo creation failed.\n");
            operationState = failed;
        break;
    }
    return operationState;
}

operationState_t openNamedFifo (const char * name, fileDescriptor_t * fileDescriptor)
{
    *fileDescriptor = open(name, O_RDONLY);
    if ( *fileDescriptor < 0 )
    {
        perror("Error: There was a failure when opening the FIFO.\n");
        return failed;
    }
    printf("Information: The named fifo was opened correctly.\n");
    return success;
}

operationState_t readMessage (char * message, int fileDescriptor, int * bytesRead)
{
    if ((*bytesRead = read(fileDescriptor, message, BUFFER_SIZE)) == -1)
    {
        return failed;
    }
    return success;
}