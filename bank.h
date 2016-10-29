#include        <sys/time.h>
#include        <sys/types.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <errno.h>
#include        <semaphore.h>
#include        <pthread.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>

struct Account
{
	char name[100]; //can be up to 100 characters long
	float balance;
	int insession; //flag
	pthread_mutex_t accountLock;
	
};
typedef struct Account* account;

struct Bank
{	
     account bankaccount[20]; //cant have more than 20 accounts
     int accountnum;						
     pthread_mutex_t banklock;	

};
typedef struct Bank* bank;

account mallocAccount();
bank  mallocBank();
int createAccount(char* name,int socket,bank x);
account accounttoStart(char* name, int socket, bank x); 