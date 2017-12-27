#include <lib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>


void initMessage(message * m)
{
	m->m1_i1=0;
	m->m1_i2=0;
	m->m1_i3=0;
}


int deposit (char * mes, int * receivers, int n, int l)
{
	message m;
	initMessage(&m);

	m.m1_i1=l;        	//Message length
	m.m1_i2=n;          	//Number of receivers
	m.m1_p1=mes;
	m.m1_p2=(char*)receivers;
	//printf("\nReady to call deposit... %d\n",(int)m.m3_p1);
	int x=_syscall(PM_PROC_NR,DEPOSIT,&m);

	return x;
}

int retrieve (char * r, int source)
{
	message m;
	initMessage(&m);

	m.m1_p1=r;
	m.m1_i1=source;

	//printf("\nReady to call retrieve...\n");
	int x=-1;
	while(x==-1){
		//printf("loop1\n");
		x=_syscall(PM_PROC_NR,RETRIEVE,&m);
		//printf("loop\n");
		//sleep(3);
	}

	return x;
}



int createMailbox(int * publishers, int n)
{
    message m;
    initMessage(&m);
    m.m1_i1=n;
	printf("publishers is %d\n",publishers[0]);
    m.m1_p1=(char*)publishers;
    printf("\nReady to create new mailbox...\n");
    int x=_syscall(PM_PROC_NR,CREATEMAILBOX,&m);
	//printf("going to call!\n");
	printf("%d\n",x);
	printf("finished calling this do_createMailbox...\n");
	return x;
}




int removeMailbox()
{
    message m;
    initMessage(&m);

    //printf("\nReady to remove new mailbox...\n");
	int x=_syscall(PM_PROC_NR,REMOVEMAILBOX,&m);

	return x;
}



int addPublisher(int publisher)
{
    message m;
    initMessage(&m);
    m.m1_i1=publisher;

    //printf("\nReady to add publisher...\n");
	int x=_syscall(PM_PROC_NR,ADDPUBLISHER,&m);

	return x;
}


int removePublisher(int publisher)
{
    message m;
    initMessage(&m);
    m.m1_i1=publisher;

    //printf("\nReady to remove publisher...\n");
	int x=_syscall(PM_PROC_NR,REMOVEPUBLISHER,&m);

	return x;
}
