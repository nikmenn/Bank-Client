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
#include 	    "bank.h"

#define CLIENT_PORT	5235

static pthread_attr_t	user_attr;
static pthread_attr_t	kernel_attr;
static sem_t		actionCycleSemaphore;
static pthread_mutex_t	mutex;
static int		connection_count = 0;
int 			y;

set_iaddr( struct sockaddr_in * sockaddr, long x, unsigned int port )
{
	memset( sockaddr, 0, sizeof(*sockaddr) );
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons( port );
	sockaddr->sin_addr.s_addr = htonl( x );
}

static char *
ps( unsigned int x, char * s, char * p )
{
	return x == 1 ? s : p;
}


void periodic_action_handler( int signo, siginfo_t * ignore, void * ignore2 )
{
	if ( signo == SIGALRM )
	{
		sem_post( &actionCycleSemaphore );		
	}
}

void * periodic_action_cycle_thread( void * arg)
{
	struct sigaction	action;
	struct itimerval	interval;
	bank x = arg;
	pthread_detach( pthread_self() );			
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	action.sa_sigaction = periodic_action_handler;
	sigemptyset( &action.sa_mask );
	sigaction( SIGALRM, &action, 0 );			
	interval.it_interval.tv_sec = 20;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 20;
	interval.it_value.tv_usec = 0;
	setitimer( ITIMER_REAL, &interval, 0 );	 /* 3 seconds */		
	while (1)
	{
		sem_wait( &actionCycleSemaphore );		
	pthread_mutex_lock(&(x->banklock));
		
  
          if(x->accountnum == 0)
          {
                  printf("\nThere are zero accounts");
          }
         else
	{
	int i = 0;
          for(; i < x->accountnum; i++)
          {
                  printf("\nThe name of the account is %s",x->bankaccount[i]->name);
                  printf("\nThe balance of %s is %.2f", x->bankaccount[i]->name, x->bankaccount[i]->balance);
                  if(x->bankaccount[i]->insession == 1)
                          printf("\nThe account is in service");
                 else
                         printf("\nThe account is not in service\n"); //indicating whether the account is being serviced or not--sometimes not working properly
                        
          }
	}
 
         pthread_mutex_unlock(&(x->banklock));
	 sched_yield();
	
	 	}
	return 0;
}

void * client_session_thread( void * bank ) 
{
	int			sd;
	char* request = malloc(512);
	char* argument = malloc(512);
	char* newrequest = malloc(512);
	char* newarg = malloc(512);
	char			temp;
	int			i;
	int 			sdcopy;
	int			limit, size;
	float			ignore;
	long			senderIPaddr;
	char *			func = "client_session_thread";

	printf("\nClient has been accepted");
	int flag = 1;
	sd = y;	
	sdcopy = sd;				
	pthread_detach( pthread_self() );		
	pthread_mutex_lock( &mutex );
	++connection_count;				
	pthread_mutex_unlock( &mutex );
	while ( read( sd, request,256) > 0 )      
	{
		if(isValid(request) == 0)     
		{
			read(sd,argument,256);  /*Read the name of the account */
			createAccount(argument,sdcopy,bank);	/*Create the account(bank.c)*/	
				
		}

		else if(isValid(request) == 1)  /*Check if the user wants to start*/
		{
			read(sd,argument,256);
			account temp = accounttoStart(argument,sdcopy,bank);
			if(temp == NULL)
			{
				char prompt[] = "\nEnter finish to leave this prompt and return";
				write(sd,prompt,256); 
				continue;
			} 
			while(1)              /*block the thread every 2 seconds */
			{
				if(pthread_mutex_trylock(&(temp->accountLock)) == EBUSY)
				{
		char prompt[] = "\nAnother client is using this account wait until a prompt says you are in session";	
					write(sd,prompt,256);
				}      
				else
				{
					char prompt[] = "\nYou can now enter in commands related to the bank account";
					write(sd,prompt,256);
					break;

				}
				sleep(2);

			}

		while(flag)
	{
			read(sd,newrequest,256);
			read(sd,newarg,256);
			float money = atof(newarg);
			 if(isValid(newrequest) == 5)   /*The user wants to check their balance*/                                 
				{
					 
                char prompt[256];
                sprintf(prompt,"\nThe balance is % .2f",temp->balance);
                write(sd,prompt,256);
			    continue;

                    }


			else if(isValid(newrequest) == 6)   /*User has entered in finish*/
                                 {
					  break;
                                                              
			         } 
		
			else
			{	
		
			
				if(isValid(newrequest) == 3)  /*User want to credit their account */
				{
					char prompt[256];
					if(money < 0 || money == 0)
					{
						char error[] = "\nInvalid entry"; /* user has to enter in proper numbers */
						write(sd,error,256);
						continue;

					}
					temp->balance = temp->balance + money;
					sprintf(prompt,"\n%.2f was credited to your account",money);
                                         write(sd,prompt,256);

				}
				
				if(isValid(newrequest) == 4)  /* User wants to take out money*/	
				{
					if(money > temp->balance)
					{
					char prompt[] = "\nThe amount you entered was greater than the current balance"; /*Cannot debit more than what was credited */
					write(sd,prompt,256);
					continue;
					}
					char prompt[256];
					temp->balance = temp->balance - money;
					  sprintf(prompt,"\n%.2f was debited out of the account",money);
                                          write(sd,prompt,256);
				}

			}
	}
					
				
				pthread_mutex_unlock(&(temp->accountLock));
		}	
	
		
		

	}
	

	close(sd);
	pthread_mutex_lock( &mutex );
	--connection_count;				
	pthread_mutex_unlock( &mutex );
	return 0;
}

int isValid(char* prompt)
{
	if(memcmp(prompt,"create",6) == 0)
		return 0;
	else if(memcmp(prompt,"start",5) == 0)
		return 1;
	else if(memcmp(prompt,"exit",4) == 0)
		return 2;
	else if(memcmp(prompt,"credit",6) == 0)
		return 3;
	else if(memcmp(prompt,"debit",5) == 0)
		return 4;
	else if(memcmp(prompt,"balance",7) == 0)
		return 5;
	else if(memcmp(prompt,"finish",6) == 0)
		return 6;
	else
		return -1;
}

void * session_acceptor_thread( void * ignore )
{
	int			sd;
	int			fd;
	int *			fdptr;
	struct sockaddr_in	addr;
	struct sockaddr_in	senderAddr;
	socklen_t		ic;
	pthread_t		tid;
	int			on = 1;
	char *			func = "session_acceptor_thread";

	pthread_detach( pthread_self() );
	if ( (sd = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
	{
		printf( "socket() failed in %s()\n", func );
		return 0;
	}
	else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
	{
		printf( "setsockopt() failed in %s()\n", func );
		return 0;
	}
	else if ( set_iaddr( &addr, INADDR_ANY, CLIENT_PORT ), errno = 0,
			bind( sd, (const struct sockaddr *)&addr, sizeof(addr) ) == -1 )
	{
		printf( "bind() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else if ( listen( sd, 100 ) == -1 )
	{
		printf( "listen() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else
	{
		int abc = 0;
		int*  abcptr;
		ic = sizeof(senderAddr);
		while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
		{
			y = fd;
			fdptr = (int *)malloc( sizeof(int) );
			*fdptr = fd;					
			if ( pthread_create( &tid, &kernel_attr, client_session_thread, ignore ) != 0 )
			{
				printf( "pthread_create() failed in %s()\n", func );
				return 0;
			}			

			else
				continue;
		}


		
		close( sd );
		return 0;
	}
}

int main( int argc, char ** argv )
{
	pthread_t	tid;
	char *	    func = "server main";
	bank			xyz;             			 
	xyz = mallocBank();
	
	

	if ( pthread_attr_init( &user_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &user_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}
	else if ( pthread_attr_init( &kernel_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}
	else if ( sem_init( &actionCycleSemaphore, 0, 0 ) != 0 )
	{
		printf( "sem_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_mutex_init( &mutex, NULL ) != 0 )
	{
		printf( "pthread_mutex_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_create( &tid, &kernel_attr, session_acceptor_thread, xyz ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_create( &tid, &kernel_attr, periodic_action_cycle_thread, xyz ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	
	else
	{
		printf( "Server can receive client connections\n" );
		pthread_exit( 0 );
	}
}
