#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mailbox.h>


void printMenu(int x);
void func_deposit(int receiver[256], char * message);
void func_receive(char * response);
void func_addPublisher();
void func_removePublisher();
void func_createMailbox(int publisher[256]);

int main(){
    int choice=0;
    int publisher[256];
    int receiver[256];
    char * message = malloc(4096);
    char * response = malloc(4096);
    
    publisher[0]=getpid();
	printf("publisher[0] is %d\n",publisher[0]);
	printf("going to call create mailbox\n");
    int x=createMailbox(publisher,1);
	printf("\nfinished calling create mailbox and returned %d\n",x);
	
    addPublisher(x);
    while(choice != 6){
        printMenu(x);
        printf("Enter choice: ");
        scanf("%d",&choice);
        
        switch (choice) {
            case 0:
                printMenu(x);
                func_deposit(receiver, message);
                break;
            case 1:
                printMenu(x);
                func_receive(response);
                break;
            case 2:
                printMenu(x);
                func_addPublisher();
                break;
            case 3:
                printMenu(x);
                func_removePublisher();
                break;
            case 4:
                printMenu(x);
                func_createMailbox(publisher);
                break;
            case 5:
                removeMailbox();
                break;
            case 6:
                printMenu(x);
                printf("ended\n");
                sleep(3);
                break;
            default:
                printMenu(x);
                printf("Wrong input\n\nType enter to continue...");
                getchar();
                break;
        }
    }
    removeMailbox();
	
    return 0;
}


void printMenu(int x){
    //system("clear");
    printf("\n\n*************** Your id: %d ***************\n\n",x);
    printf("0. Deposit message\n");
    printf("1. Retrieve most ancient message\n");
    printf("2. Add publisher\n");
    printf("3. Remove publisher\n");
    printf("4. Create mailbox\n");
    printf("5. Remove mailbox\n");
    printf("6. Quit mailbox\n\n\n");
}

void func_deposit(int receiver[256], char * message){
    int i = 0;
    int receiverChoice = 0;
    printf("Enter message:");
    //scanf("%[^\n]%*c",message);
    getchar();
    fgets(message,512,stdin);
    while ((receiverChoice != 1) && (i <256)) {
        printf("Enter id of receiver:");
        scanf("%d",&receiver[i]);
        system("clear");
        printf("\n\n0.Enter other receiver.\n1.Send message\n");
        printf("\n\nEnter choice:");
        scanf ("%d",&receiverChoice);
        i++;
    }
    deposit(message,receiver,i,strlen(message));
}

void func_receive(char * response){
    int receiveFrom=0;
    printf("Receive from which process (0 mean most ancient message):");
    scanf("%d",&receiveFrom);
    retrieve (response,receiveFrom);
    printf("Message: %s\n",response);
    printf("\nType enter to continue");
    getchar();
    getchar();

}

void func_addPublisher(){
    int tempPublisher=0;
    printf("Enter publisher id:");
    scanf("%d",&tempPublisher);
    addPublisher(tempPublisher);
}

void func_removePublisher(){
    int tempPublisher=0;
    printf("Enter publisher id:");
    scanf("%d",&tempPublisher);
    removePublisher(tempPublisher);
}

void func_createMailbox(int publisher[256]){
    int i = 0;
    int x;
    int createChoice = 0;
    i++;
    while ((createChoice != 1) && (i <256)) 
	{
        printf("Enter id of publisher:");
        scanf("%d",&publisher[i]);
        //system("clear");
        printf("\n\n0.Allow another publisher.\n1.Create mailbox\n");
        printf("\n\nEnter choice:");
        scanf ("%d",&createChoice);
        i++;
    }
    x = createMailbox(publisher,i);
    if(x!=-1)addPublisher(x);
}