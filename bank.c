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
 #include      "bank.h"


account mallocAccount()
{
	account x = malloc(sizeof(struct Account));
	pthread_mutex_init(&(x->accountLock),NULL);
	return x;		
}

bank mallocBank()
{
	bank x = malloc(sizeof(struct Bank));
	x->accountnum = 0;
	pthread_mutex_init(&(x->banklock),NULL);	
	return x;
}

int createAccount(char* accountname,int socket, bank x)
{
	if(pthread_mutex_trylock(&(x->banklock)) == EBUSY)  /*The other thread has to wait until it's done printing or creating a new account */
	{
		char prompt[] = "\nNo new account can be created currently";
		write(socket,prompt,256);
		pthread_mutex_lock(&(x->banklock));
	}

	/*Check if there's already 20 accounts in the bank */
	if(x->accountnum == 20)
	{
		char prompt[] = "\nThe bank cannot exceed more than 20 accounts";
		write(socket,prompt,256);
		pthread_mutex_unlock(&(x->banklock));
		return -1;
	}

	for(int i = 0; i < 20; i++)   /*Check to see if there's another account already with the same name */
	{
		if((x->bankaccount[i] != NULL) && (strcmp(accountname,x->bankaccount[i]->name) == 0))
		{
			char prompt[] = "\nAccount cannot be created because another account already exists with the same name";
			write(socket,prompt,256);
			pthread_mutex_unlock(&(x->banklock));
			return -1;

		}
	}

	account new = mallocAccount();
	memcpy(new->name,accountname,256);   /*Account can be created */
	new->balance = 0.00; /* Accounts start at 0 */
	new->insession = 1; /* Account is in service now(so value changed to 1) */
	x->bankaccount[x->accountnum] = new;
	x->accountnum++;
	pthread_mutex_unlock(&(x->banklock));
	char prompt[] = "\nThe account was opened successfully";
	write(socket,prompt,256);
	return 1;
}	

account accounttoStart(char* name,int socket, bank x)   /*Look for account in array */
{
	for(int i = 0; i <( x->accountnum);i++)
	{
		if((x->bankaccount[i] != NULL) && (strcmp(name,x->bankaccount[i]->name) == 0))       
		{
			return x->bankaccount[i];
		}

	}
	//if it gets here that means the user entered an account which isn't there or hasn't been created
	char prompt[] = "\nEntered bank account does not exist";
	write(socket,prompt,256);
	return NULL;
}
	
