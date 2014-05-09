#include <kernel.h>
#include <stdio.h>
#include <stdlib.h> //for atoi
#include <ctype.h>
#include <string.h>
#include <tty.h>
#include <uart.h>
#include <shell.h>
#define MAX 4

struct user_info {
	char user[20];//username
	char pass[20];//password
	tid_typ tid;  //thread id
	unsigned int end; //0=continue program
};			  //1=exit program
			  //2=new user
void add_users (struct user_info (*u)[MAX]);
void login (struct user_info (*u)[MAX], int, int);
int regster (struct user_info (*u)[MAX],int);
int check_user (struct user_info (*u)[MAX], char input[20], void*);
int check_pass (struct user_info (*u)[MAX], char input[20], int);
void print_user (char user[20], int);
void* chat(struct user_info (*u)[MAX], int, int);
//semaphore sem;

shellcmd xsh_xinu_chat (int nargs, char *args[])
{
	if (nargs == 2 && strncmp(args[1],"--help",6) == 0) {
		fprintf(stdout, "Usage: xinu_chat\n");
		fprintf(stdout, "\t--help\t display this help and exit\n");
		return SYSERR;
	} 

	//get device number
	int device=stdout;	

//	sem = semcreate(1);
	//initialize struct
	struct user_info u[MAX];

	//add users to struct
	add_users (&u);

	//validate user and get user number	
	int usr_num = regster (&u, device);

	//create threads and login users	
	login(&u, device, usr_num);
	u[usr_num].end = 0;

	//don't exit program
	while(1){
		if (u[usr_num].end == 1) //if exit
		{
			return OK;
		} else if (u[usr_num].end == 2) { //if new user
			u[usr_num].end = 0;
			usr_num = regster (&u, device);
			login (&u, device, usr_num);
		}
		sleep(500);
		//signal(sem);
	}
}

/*
 * Adds users and passwords to the struct
 *
 * @parm u: Takes in an array of struct that hold user information
 */
void add_users (struct user_info (*u)[MAX])
{
	char user0[10]={'b','r','i','a','n','\0'};
	char pass0[10]={'b','r','i','a','n','\0'};
	char user1[10]={'b','r','u','c','e','\0'};
	char pass1[10]={'b','r','u','c','e','\0'};
	char user2[10]={'l','i','n','\0'};
	char pass2[10]={'l','i','n','\0'};
	char user3[10]={'b','i','n','a','\0'};
	char pass3[10]={'b','i','n','a','\0'};
	strncpy((*u)[0].user, user0, sizeof(user0));
	strncpy((*u)[0].pass, pass0, sizeof(pass0));
	strncpy((*u)[1].user, user1, sizeof(user1));
	strncpy((*u)[1].pass, pass1, sizeof(pass1));
	strncpy((*u)[2].user, user2, sizeof(user2));
	strncpy((*u)[2].pass, pass2, sizeof(pass2));
	strncpy((*u)[3].user, user3, sizeof(user3));
	strncpy((*u)[3].pass, pass3, sizeof(pass3));
}

/*
 * Creates a chat thread for logged in user
 *
 * @parm u: Takes in an array of structs containing user information
 * @parm dev: Takes in device number (TTY0/TTY1)
 * @parm usr_num: Takes in the user's id
 */
void login (struct user_info (*u)[MAX], int dev, int usr_num) 
{
	(*u)[usr_num].tid = create((void *)chat, INITSTK, INITPRIO,
			"user", 3, u, dev, usr_num);
	ready((*u)[usr_num].tid, RESCHED_YES);
	(*u)[usr_num].end = 0;
}

/*
 *  Asks the user to input stored name and password
 *
 *  @parm u: Takes in struct of user information
 *  @parm dev: Takes in device number (TTY0/TTY1)
 */
int regster (struct user_info (*u)[MAX], int dev) 
{
	char user[20]={0};//username
	char pass[20]={0};//password
	int usr_num=0;
	//char buffer[256];

	while(1)
	{
		//write to port 
		write(dev, "Enter username: ", 16);
		//get input
		fgets(user, 20, dev);
		if((strlen(user) > 0) && check_user(u, user, &usr_num)) 
			break;
		else
			printf("username invaild\n");
	}
	
	while(1)
	{
		//write to port 
		write(dev, "Enter password: ", 16);
		//get input
		fgets(pass, 20, dev);
		if((strlen(pass) > 0) && check_pass(u, pass, usr_num))
			break;
		else
			printf("password invalid\n");
	}

	//copy username to user struct
//	strncpy((*u)[usr_num].user, user, 20);
//	copy password to user struct
//	strncpy((*u)[usr_num].pass, pass, 20);

	return usr_num;
}

/*
 * Checks if the inputted username is valid
 *
 * @parm u: Takes in a struct of user information
 * @parm input: Takes in the user's inputted username
 * @parm user_num: Takes in user id
 */
int check_user (struct user_info (*u)[MAX], char input[20], void* user_num) 
{
	int* pint = (int*) user_num;

	for ((*pint)=0;(*pint)<MAX;++(*pint))
	{
		char buff[20]={0};
//		printf("%d length: ", strlen((*u)[(pint)].user));
		strncpy(buff, (*u)[(*pint)].user, 20);
		if(strncmp(buff, input, strlen(buff))==0)
		{
			return 1;
		}
	}
	return 0;

}

/*
 * Checks if the inputted password is valid
 *
 * @parm u: Takes in a struct of user information
 * @parm input: Takes in the user's inputted username
 * @parm user_num: Takes in user id
 */

int check_pass (struct user_info (*u)[MAX], char input[20], int usr_num)
{
	printf("user: ", usr_num);
	char buff[10]={0};
	strncpy(buff, (*u)[usr_num].pass, 20);
	if(strncmp(buff, input, strlen(buff))==0)
	{
		return 1;
	}

	return 0;

}

/* 
 * Prints user's name
 *
 * @parm user: Takes in char array to be printed
 * @parm dev: Takes in device number (TTY0/TTY1)
 */
void print_user (char user[20], int dev)
{
	int i = 0;
	//	fprintf(dev, "\n");

	for (;i<strlen(user);i++)
	{	
		fprintf(dev, "%c",  user[i]);
	}	
	fprintf(dev, ": ");
}

/*
 * Is a loop to print out user's name and input to TTY0 and TTY1
 *
 * @parm u: Takes in struct of user information
 * @parm dev: Takes in device number (TTY0/TTY1)
 * @parm user_num: Takes in user id
 */
void* chat (struct user_info (*u)[MAX], int dev, int usr_num)
{
	char logoff[6]={'/','e','x','i','t','\0'};	
	char new_user[5]={'/','n','e','w','\0'};

	fprintf(dev, "Enter /new to login new user\n");
	fprintf(dev, "Enter /exit to logout\n");	
	
	while (1) //loop until user asks to exit or logoff
	{
	//	fprintf(dev, "%d\n", thrcount);
		//sleep(5500);
		//wait (sem);
		char msg[100]={0};
		fgets(msg, 100, dev);

		if (strncmp(msg, logoff,5)==0) //if user types /exit
		{
			(*u)[usr_num].end = 1;
			kill((*u)[usr_num].tid);
		} else if (strncmp(msg, new_user,4)==0) { //if user types /new
			(*u)[usr_num].end = 2;
			kill((*u)[usr_num].tid);
		}

		if (dev ==4) //if serial0
		{
			fprintf(dev+1, "\n\033[1;35m");
			print_user((*u)[usr_num].user, dev+1);
			fprintf(dev+1, "\033[1;39m");
			write (dev+1, msg, strlen(msg));
		} else if (dev ==5) { //if serial1 
			fprintf(dev-1, "\n\033[1;36m");
			print_user((*u)[usr_num].user, dev-1);
			fprintf(dev-1, "\033[1;39m");
			write (dev-1, msg, strlen(msg));
		}
		
		fprintf(dev,"\033[1;32m");
                print_user((*u)[usr_num].user, dev);
                fprintf(dev,"\033[1;39m");

		write(dev, msg, sizeof(msg));
		//signal(sem);
	}
}
