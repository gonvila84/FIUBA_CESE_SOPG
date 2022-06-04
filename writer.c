#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define FIFO_NAME   "writer_fifo"
#define BUFFER_SIZE     300

enum operationState {
    success,
    failed,
};

enum messageType {
    data,
    sign
};

typedef enum operationState operationState_t;
typedef enum messageType messageType_t;
typedef int32_t fileDescriptor_t;

operationState_t createNamedFifo (const char * name);
operationState_t openNamedFifo (const char * name, fileDescriptor_t * fileDescriptor);
operationState_t mapSignalToHandler(struct sigaction * sa, int8_t sig);
void readInput (char * inputBuffer);
void formatDataMessage (char * inputBuffer, char * outputBuffer, messageType_t messageT);
operationState_t sendMessage (char * message, int32_t fileDescriptor);
void sigHandler(int32_t sig);

fileDescriptor_t fileDescriptor;

int main (void)
{
    char inputBuffer[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE+5];

	struct sigaction sa;
	sa.sa_handler = sigHandler;
	sa.sa_flags = 0; //SA_RESTART;

    if (failed == mapSignalToHandler(&sa, 10))
    {
		exit(1);
    }

    if (failed == mapSignalToHandler(&sa, 12))
    {
		exit(1);
    }

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
    printf("A reader is listening and ready to receive messages.\n");    
    while(1)
    {
        //Leo texto de la consola
        printf("Input> ");
        readInput(inputBuffer);
        //Formatear Texto
        formatDataMessage(inputBuffer,outputBuffer, data);
        //Enviar Texto
        if (failed == sendMessage(outputBuffer,fileDescriptor))
        {
		    exit(1);
        }        
    }
    exit(0);
}

void sigHandler(int sig)
{
    if (sig == 10)
    {
        write(fileDescriptor, "SIGN:1", 6);
    }
    if (sig == 12)
    {
        write(fileDescriptor, "SIGN:2", 6);
    }
}

operationState_t mapSignalToHandler(struct sigaction * sa, int8_t sig)
{
	if (-1 == sigemptyset(&sa->sa_mask))
    {
        perror("Error: Could not perform sigemptyset (return code: -1).\n");
        return failed;
    }
	if (-1 == sigaction(SIGUSR1,sa,NULL))
    {
        perror("Error: Could nor map SIGUSR1 signal to handler. (return code: -1).\n");
        return failed;
    }
	if (-1 == sigaction(SIGUSR2,sa,NULL))
    {
        perror("Error: Could nor map SIGUSR2 signal to handler. (return code: -1).\n");
        return failed;
    }
    return success;
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
    *fileDescriptor = open(name, O_WRONLY);
    if ( *fileDescriptor < 0 )
    {
        perror("Error: There was a failure when opening the FIFO.\n");
        return failed;
    }
    printf("Information: The named fifo was opened correctly.\n");
    return success;
}

void readInput (char * inputBuffer)
{
    inputBuffer[0] = '\0';
    do {
        fgets(inputBuffer, BUFFER_SIZE, stdin);
    } while (inputBuffer[0] == '\0');
}

void formatDataMessage (char * inputBuffer, char * outputBuffer, messageType_t messageT)
{
    switch (messageT)
    {
        case data:
            strcpy(outputBuffer,"DATA:");
            break;
        case sign:
            strcpy(outputBuffer,"SIGN:");
            break; 
    }

    strncat(outputBuffer,inputBuffer,BUFFER_SIZE);
}

operationState_t sendMessage (char * message, int32_t fileDescriptor)
{
    uint32_t bytesWrote = write(fileDescriptor, message, strlen(message)-1);
    if (bytesWrote == -1)
    {
        perror("Error: Transmition to the reader failed.\n");
        return failed;
    }
    return success;
}
