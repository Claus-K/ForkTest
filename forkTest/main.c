#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "token.h"
#include "count.h"
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


int fd[2];
# define sizeAccList 100
count accList[sizeAccList];
pthread_mutex_t rt, wt, auxt;
int countThreads;

void initList(count tok[], int size) {
    for(int i = 0; i < size; i++) {
        tok[i].count = i;
        tok[i].value = 0;
        
    }
}

void printList(count list[], int size) {
    printf("\n:::::\tLIST\t:::::\n");
    for(int i = 0; i < size; i++) {
        printf("Account\t::\t%d\t->\tValue\t::\t%d\n", list[i].count, list[i].value);
    }
     printf("\n:::::\tEND\t:::::\n");
}

void* holdToken(void* tok) {
    token* value = (token *)tok;
    token it = *value;
    pthread_mutex_lock(&wt);
    printf("Holding Token\t::\t%d\t%d\n", it.count, it.value);
    close(fd[0]);
    write(fd[1], &it, sizeof(token));
    countThreads++;
    close(fd[1]);
    pthread_mutex_unlock(&wt);
    return NULL;
}

int findAccount(count tok, int size) {
    for(int i = 0; i < size; i++) {
        if(tok.count == accList[i].count) {
            return i;
        }
    }
    return 0;
}

void deposit(count cot) {
    int i = findAccount(cot, sizeAccList);
    accList[i].value += cot.value;
    // printf("Deposit\t::\t%d\t\tAcc\t::\t%d\n", accList[i].count, accList[i].value);
}

void witdraw(count cot) {
    int i = findAccount(cot, sizeAccList);
    if(accList[i].value - cot.value < 0) {
        printf("ACC\t::%d\t\tWitdraw\t->\tMore than you have\n", cot.count);
    } else {
        accList[i].value -= cot.value;
        printf("Witdraw\t::\t%d\t\tAcc\t::\t%d\n", accList[i].value, accList[i].count);
    }
}


void exe(token tok) {
    count cot;
    cot.count = tok.count;
    cot.value = tok.value;        
    switch(tok.oper) {
        case 1:
            deposit(cot);
        break;
        case 0:
            witdraw(cot);
        break;
    }     
}

void doToken() {
    int i = 0;
    int status;
    while(1) {
        token tok;
        close(fd[1]);
        status = read(fd[0], &tok, sizeof(token));
        close(fd[0]);
        
        if(status != 0) {
        printf("SEV\t::\tToken\t->\t%d\t%d\t%d\n", tok.count, tok.oper, tok.value);
        exe(tok);
        } else {
            printf("\n\nBREAK\n\n");
            break;
        } 
        i++;
    }
    printList(accList, sizeAccList);
}

void sendToken(token tok, int pid) {
    pthread_t atm;
    pthread_create(&atm, NULL, holdToken, &tok);
    pthread_join(atm, NULL); 
}

 int readToken(char fileName[], int tid) {
    FILE* file = fopen(fileName, "r");
    int size = 256;
    char line[size];
    int oper;
    int count;
    int value;
    while (fgets(line, sizeof(line), file)) {
        char * pch;
        // printf ("Splitting string \"%s\" into tokens:\n",line);
        
        pch = strtok (line," ,.-");
        int aux = 0;
        while (pch != NULL)  {
            //printf ("%s\n",pch);
            switch(aux) {
                case 0:
                    count = atoi(pch);
                break;
                case 1:
                    oper = atoi(pch);
                break;
                case 2:
                    value = atoi(pch);
                break;
            }
            
            pch = strtok (NULL, " ,.-");
            aux++;
            }
        token cod;
        cod.count = count;
        cod.value = value;
        cod.oper = oper;
        sendToken(cod, tid);
        printf("ATM\t%d\t->\t%d\t%d\t%d\n", tid, count, oper, value);
        // printf("Line %s\n", line); 
    }
     printf("==========\tATM\t%d\tFINISHED\t==========\n", tid);
     printf("Created\t->\t%d\n\n", countThreads);
    
    fclose(file);
    
    return 0;
}

int main()
{
    initList(accList, sizeAccList);
    // printf("PID\t::\t%d\n", getpid());
    if(pipe(fd) < 0) {
        printf("PIPE FD OPEN");
    }
    for(int i = 0; i < 1; i++) {
        if(fork()==0) {
            printf("[son] pid %d from [parent] pid %d\n", getpid(), getppid());
            if(getpid() != 0) {
                // sleep(5);
                printf("SERVER\t::\t%d\n", getpid());
                doToken();
                
            }
            exit(0);
        }
    }
	for(int i=5;i<7;i++)
	{
        char fileName[10];
        snprintf(fileName, 10,"file%d.txt", i);
		if(fork() == 0)
		{
			printf("[son] pid %d from [parent] pid %d\n", getpid(), getppid());
            if(getpid() != 0) {
                printf("PID\t::\t%d\n", getpid());
                printf("Read\t::\t%s\n", fileName);
                readToken(fileName, getpid());
            } else {
                printf("FORK FAIL\n");
            }
			exit(0);
		}
	}
	for(int i=0;i<3;i++) // loop will run 'n' times
	    wait(NULL);
    printf("Hello World\n");
	
}