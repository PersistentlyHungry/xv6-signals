#include "user.h"

#define BUF_SIZE 128

int numOfIdleChildren;
int n;
int table[3][62]; //this table holds the idle childs   (pid, idle=0/busy=1, value from shell)
char buf[BUF_SIZE];

int findPrim(int value){
	// int sol = value;
	int i;
	while (1) {
		value++;
		for (i = 2; i < value; i++){
			if (value % i == 0)
				break;
		}
		if (i == value) // sol is a prime number
			return value;
	}
}

void workerPrimHandler(int fatherPid, int value){
	if (value == 0){
		printf(1, "worker %d exits\n", getpid());
		exit();
	}
	int primNum = findPrim(value);
	sigsend(fatherPid, primNum);
}

void fatherHandler(int childPid, int value){
	int i;
	for (i = 0; i < n; i++){
		if (table[0][i] == childPid)
			break;
	}
	numOfIdleChildren++;
	printf(1, "worker %d returned %d as a result for %d\n", childPid, value, table[2][i]);
	table[1][i] = 0; // mark child as idle
}

int main(int argc, char *argv[]) {
	if (argc != 2){
				printf(1, "Usage: primsrv <n>\n");
				exit();
	}
	// Set initial signal handler to be that of the children. When the children
	// are created with fork(), this signal handler is duplicated as well.
	// After spawning all children, set the father's signal handler to its
	// desired handler.
	sigset((sig_handler)&workerPrimHandler);
	n = atoi(argv[1]);
	if (n > 62)	{
				printf(1, "62 is the maximum number of children\n");
				exit();
	}
	numOfIdleChildren = n;
	int i;
	int numRead;
	printf(1, "workers pids:\n");
	for (i = 0; i < n; i++){
			table[0][i] = fork();
			if (table[0][i] == 0) { // child code
				while(1)
					sigpause();	//the handler itself suposed to exit
				printf(1, "ERROR\n");
				exit();
			}
			else { // father code
				table[1][i] = 0; // mark child as idle
				printf(1, "pid: %d\n", table[0][i]);
			}
	}
	printf(1, "				***       \n");
	sigset((sig_handler)&fatherHandler);

	while (1) {
		printf(1, "please enter a number: ");
		gets(buf, 128);
		if(strlen(buf) == 1 && buf[0]=='\n')
			continue;
		numRead = atoi(buf);
		memset(buf, '\0', BUF_SIZE);
		if (numRead == 0)
			break; // commence graceful shutdown
		if (numOfIdleChildren == 0){
			printf(1, "no idle workers\n");
			continue;
		}
		for (i = 0; i < n; i++)
			if (table[1][i] == 0){ // idle child
				table[1][i] = 1;	// mark child as busy
				table[2][i] = numRead;
				numOfIdleChildren--;
				sigsend(table[0][i], numRead);
				break;
			}
	}

	for (i = 0; i < n; i++)
		sigsend(table[0][i], 0); // signal children to self-terminate

	for (i = 0; i < n; i++){
		if(table[1][i] == 1)
			sigpause(); // wait for a result
		wait();
	}

	printf(1, "primesrv exit\n");
	exit();
}
