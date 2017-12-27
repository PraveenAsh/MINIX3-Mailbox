#include "pm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/vm.h>
#include <string.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include "param.h"
#include <string.h>
#include <mailbox.h>


extern message m_in;
//message m_in;//new

struct blockedProc{
	int blocked;
	int source;
	char * address;
	struct blockedProc * next;
};

struct blockedProc * firstBlocked=NULL;

struct publisher
{
    int id;
    struct publisher * next;
};

struct messageList
{
    message m;
    int sender;
    struct messageList * next;
};

struct mailbox
{
    int nbOfMes;
    int owner;
    struct publisher * publishers;
    struct messageList * mList;
    struct mailbox * next;
};

struct mailbox * defaultBox=NULL;


//////////////////////////////////////////////////
//
// Find the mailboxes of the receivers and add a message
//
//////////////////////////////////////////////////


int do_deposit()
{
printf("**********deposit function**********\n");
    if(defaultBox==NULL){
        printf("No existing mailbox...\n");
        return -1;
    }

    int * rec=malloc(m_in.m1_i2*sizeof(int));
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p2, SELF, (vir_bytes)rec, m_in.m1_i2*sizeof(int));

    //printf("There are mailboxes\n");
    //printf("sender: %d\n",m_in.m1_i3);
    int i;
    for(i=0;i<m_in.m1_i2;i++){
        //Create a cursor
        struct mailbox * current=defaultBox;
        //For each receiver, find if their is an existing mailbox
        while(current!=NULL){
            //First check if the mailbox is full
            if(current->nbOfMes==10){
                printf("Mailbox %d full, cannot deliver message...\n",current->owner);
            }
            //Check if this is the mailbox of a receiver and if we are authorized to publish
            if(current->owner==rec[i]&&canPublish(current)&&current->nbOfMes<10){

                struct messageList * ml = current->mList;
                //If first message, initialize
                if(ml==NULL){
                    ml = malloc(sizeof(struct messageList));
                    ml->next=NULL;
                //Else, go to the end of the list and add a new message
                }else{
                    while(ml->next != NULL) ml=ml->next;
                    ml->next=malloc(sizeof(struct messageList));
                    ml->next->next=NULL;
                    ml=ml->next;
                }
                memcpy(&ml->m,&m_in,sizeof(message));
                ml->m.m1_p1=malloc(4096);
                if(sys_datacopy(who_e, (vir_bytes)m_in.m1_p1, SELF, (vir_bytes)ml->m.m1_p1, m_in.m1_i1+1)==OK)printf("Copy OK\n");
                ml->sender=who_e;
                printf("message: %s\n",ml->m.m1_p1);
                if(current->mList==NULL)current->mList=ml;
                current->nbOfMes++;
                current=current->next;

                //we check if the process was waiting for a message
                struct blockedProc * bp=firstBlocked;
                while(bp!=NULL&&bp->blocked!=rec[i]) bp=bp->next;
                if(bp!=NULL)sys_resume(bp->blocked);
                break;

            }else{
                current=current->next;
                if(current==NULL) printf("Mailbox for %d not found, cannot deliver message\n",rec[i]);
            }
        }
    }

    //printf("\nMessage(s) sent...\n");

    return 1;
}



/////////////////////////////////////////////
//
// Find the mailbox of the caller, and get the first message of the list
// --> it is the most ancient
//
// To be modified when possibility of retrieve a message from specific sender
//
/////////////////////////////////////////////
int do_retrieve()
{
printf("**********retrieve function**********\n");

    char * sourceAddress;
    int found=0;
    char * returnAddress=m_in.m1_p1;;

    //Check if there are mailboxes
    if(defaultBox==NULL){
        printf("No existing mailbox...\n");
        return -1;
    }

    if(firstBlocked!=NULL){
        struct blockedProc * bl=firstBlocked;
        if(bl->blocked==who_e){
            returnAddress=bl->address;
            m_in.m1_i1=bl->source;
            firstBlocked=bl->next;
            free(bl);
        }else{
            while(bl->next!=NULL){
                if(bl->next->blocked==who_e){
                    returnAddress=bl->next->address;
                    m_in.m1_i1=bl->next->source;
                    bl->next=bl->next->next;
                    free(bl->next);
                    break;
                    }
                bl=bl->next;
            }
        }
    }


    struct mailbox * current=defaultBox;
    //Go through all the mailboxes
    while(current!=NULL){
        //printf("Check a mailbox\n");
        if(current->owner==who_e){
            printf("Mailbox found\n");
            //Do we have a message from the right process ?
            struct messageList * ml=current->mList;
            if(ml!=NULL){
                sourceAddress=ml->m.m1_p1;
                if(ml->sender==m_in.m1_i1){
                    found=1;
                    sourceAddress=ml->m.m1_p1;
                }else{
                    while(ml->next!=NULL){
                        if(ml->next->sender==m_in.m1_i1){
                            found=1;
                            sourceAddress=ml->next->m.m1_p1;
                            break;
                        }
                        ml=ml->next;
                    }
                }
            }

            //If mailbox empty, process has to wait, we store important parameters
            if(current->mList==NULL||(found==0&&m_in.m1_i1!=0)){
                printf("No message found...\n");
                struct blockedProc * bp=malloc(sizeof(struct blockedProc));
                bp->blocked=who_e;
                bp->address=m_in.m1_p1;
                bp->source=m_in.m1_i1;
                bp->next=firstBlocked;
                firstBlocked=bp;
                sys_stop(who_e);
                return -1;

            }else{

			}

            //printf("%d\n",(int)m_in.m1_p1);
            if(sys_datacopy(SELF, (vir_bytes)sourceAddress, who_e, (vir_bytes)returnAddress, strlen(sourceAddress)+1)==OK)printf("Copy OK\n");
            if(m_in.m1_i1==0){
                struct messageList * tmp=current->mList->next;
                free(current->mList);
                current->mList=tmp;
            }else{
                deleteMessage(ml);
            }
            return 1;

        }else{
            current=current->next;
        }
    }

    return -1;
}



////////////////////////////////////////////////////////////
//
// Delete the first message in the list because it is the most ancient
//
// To be modified when possibility a retrieve a message from a specific sender
//
////////////////////////////////////////////////////////////
void deleteMessage(struct messageList * mes)
{
    struct messageList * tmp=mes->next->next;
    free(mes->next);
    mes->next=tmp;
}



/////////////////////////////////////////////////
//
// Go through the publishers to check the calling process
// is one of them
//
//////////////////////////////////////////////////
int canPublish(struct mailbox * mail)
{
    //printf("**********can publish function**********\n");
    if(mail->publishers==NULL){
        return 0;
    }else{
        struct publisher * pub=mail->publishers;
        //printf("We check the publishers\n");
        while(pub!=NULL){
            if(pub->id==who_e)return 1;
            else pub=pub->next;
        }
        //printf("We are going to return 0\n");
        return 0;
    }
}




//////////////////////////////////////////////////
//
// Create a mailbox for the calling process
//
/////////////////////////////////////////////////
int do_createMailbox()
{
	//printf("entered the do_createMailbox function!!\n");
	printf("**********creating mailbox**********\n");

    //first we need to check there is no existing mailbox for ths process
    struct mailbox * check=defaultBox;
    while(check!=NULL){
        if(check->owner==who_e){
            printf("This process already has a mailbox\n");
            return -1;
        }else{
            check=check->next;
        }
    }

    printf("Creating a mailbox for %d\n",who_e);

    struct mailbox * current=malloc(sizeof(struct mailbox));
    current->mList=NULL;
    current->nbOfMes=0;
    current->owner=who_e;
	printf("who_e is %d\n",who_e);
    int * pubs=malloc(4096);
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p1, SELF, (vir_bytes)pubs, m_in.m1_i1*sizeof(int));

    struct publisher * p=malloc(sizeof(struct publisher));
    p->id=pubs[0];
    current->publishers=p;

    int i;
    for(i=1;i<m_in.m1_i1;i++){
        p->next=malloc(sizeof(struct publisher));
        p=p->next;
        p->id=pubs[i];
    }

    current->next=defaultBox;
    defaultBox=current;

    return who_e;
}



//////////////////////////////////////////////////
//
// Remove a mailbox for the calling process
//
/////////////////////////////////////////////////
int do_removeMailbox()
{
printf("**********remove function**********\n");

    printf("Try to delete mailbox %d\n",who_e);
    struct mailbox * current = defaultBox;
    if(current->owner==who_e){
        defaultBox=current->next;
        if(current->mList!=NULL) free(current->mList);
        if(current->publishers!=NULL) free(current->publishers);
        free(current);
        return 1;
    }else{
        while(current->next!=NULL){
            if(current->next->owner==who_e){
                printf("Mailbox found\n");
                if(current->next->mList!=NULL) free(current->next->mList);
                if(current->next->publishers!=NULL) free(current->next->publishers);
                current->next=current->next->next;
                free(current->next);
                return 1;
            }
            current=current->next;
        }
        printf("Mailbox not found\n");
        return -1;
    }
}



//////////////////////////////////////////////////
//
// ADD  a publisher in mailbox by pid
//
/////////////////////////////////////////////////
int do_addPublisher()
{
    printf("**********add publisher**********\n");
    printf("add publisher %d to box %d\n",m_in.m1_i1,who_e);
    struct mailbox * current=defaultBox;
    while(current!=NULL){
        if(current->owner==who_e){
            //printf("Mailbox found...\n");
            struct publisher * p=current->publishers;
            if(p!=NULL){ 
                while(p->next!=NULL){
                    if(p->next->id==m_in.m1_i1){
                        printf("Publisher already exists...\n");
                        return -1;
                    }
                    p=p->next;
                }
                p->next=malloc(sizeof(struct publisher));
                p->next->id=m_in.m1_i1;
                p->next->next=NULL;
                return 1;
            }else{
                p=malloc(sizeof(struct publisher));
                p->id=m_in.m1_i1;
                p->next=NULL;
                return 1;
            }
        }
        current=current->next;
    }
    return -1;
}




int do_removePublisher()
{
printf("**********remove publisher**********\n");


    printf("remove publisher %d from box %d\n",m_in.m1_i1,who_e);
    struct mailbox *  current=defaultBox;
    while(current!=NULL){
        if(current->owner==who_e){
            //printf("Mailbox found...\n");
            struct publisher * p=current->publishers;

            if(p->id == m_in.m1_i1 ){
                    p= p->next;
                    free(p);
                    return 1;
            }else{
                while(p->next!=NULL) {
                    if(p->next->id ==m_in.m1_i1 ){
                        p->next = p->next->next;
                        free(p->next);
                        return 1;
                    }
                    p=p->next;
                }
            }

        }
        current=current->next;

    }
    printf("Not found...\n");

    return -1;
}

