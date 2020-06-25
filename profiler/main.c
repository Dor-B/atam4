#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#define MAX_STR 257

/*#define CALL( syscall ) do {\
		if( (syscall) == -1 ) {\
			perror( #syscall );\
			exit(1);\
		}\
		} while(0)*/

typedef struct regs_names{
	char* rax;
	int raxSize;
	
	char* rbx;
	int rbxSize;
	
	char* rcx;
	int rcxSize;
	
	char* rdx;
	int rdxSize;
	
	char* rsi;
	int rsiSize;
} RegsNames;

void putNull(RegsNames* regsNames){
	regsNames->rax = NULL;
	regsNames->rbx = NULL;
	regsNames->rcx = NULL;
	regsNames->rdx = NULL;
	regsNames->rsi = NULL;
}

bool checkRegister(char* str, char* reg1, char* reg2, char* reg3, char* reg4){
	return (strcmp(str, reg1) == 0) || (strcmp(str, reg2) == 0) || (strcmp(str, reg3) == 0) || (strcmp(str, reg4) == 0);
}

int getRegBytes(char *reg){
	if(reg[0] == 'r'){
		return 8;
	}
	if(reg[0] == 'e'){
		return 4;
	}
	if((reg[1] == 'l') || (reg[2] == 'l')){
		return 1;
	}
	return 2;
}

void toSize(unsigned long long int* num, int bytes){
	int toShift = 64 - bytes*8;
	*num = (*num << toShift) >> toShift;
}

void printDiff(char* varName, unsigned long long int before, unsigned long long int after, int size){
	toSize(&before, size);
	toSize(&after, size);
	printf("PRF:: %s: %llu->%llu\n", varName, before, after);
}

long CALL(long res){
	if(res == -1){
		perror("err");
		exit(1);
	}
	return res;
}

void run_profiler(pid_t childPid, long int startAddr, long int endAddr, RegsNames regsNames){
	//printf("run_profile\n");
	struct user_regs_struct regsStart;
	struct user_regs_struct regsEnd;
	
	int wait_status;
	CALL(wait(&wait_status));
	long origStart = CALL(ptrace(PTRACE_PEEKTEXT, childPid, (void*) startAddr, NULL));
	//printf("After PEEK\n");
	
	unsigned long startTrap = (origStart & 0xFFFFFFFFFFFFFF00) | 0xCC;
	CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) startAddr, (void *) startTrap));
	//printf("origStart %x | startTrap %x\n", origStart, startTrap);
	
	// let the child run till start
	CALL(ptrace(PTRACE_CONT, childPid, NULL, NULL));
	wait(&wait_status);
	
	//printf("After start\n");
	
	// recover start
	CALL(ptrace(PTRACE_GETREGS, childPid, 0, &regsStart));
	CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) startAddr, (void *) origStart));
	regsStart.rip -= 1;
	CALL(ptrace(PTRACE_SETREGS, childPid, 0, &regsStart));
	//printf("After recover start\n");
	
	// set breakpoint at end
	long origEnd = CALL(ptrace(PTRACE_PEEKTEXT, childPid, (void*) endAddr, NULL));
	unsigned long endTrap = (origEnd & 0xFFFFFFFFFFFFFF00) | 0xCC;
	CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) endAddr, (void *) endTrap));
	//printf("origEnd %x | endTrap %x\n", origEnd, endTrap);
	
	// let the child run till end
	CALL(ptrace(PTRACE_CONT, childPid, NULL, NULL));
	CALL(wait(&wait_status));
	
	// recover end
	CALL(ptrace(PTRACE_GETREGS, childPid, 0, &regsEnd));
	CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) endAddr, (void *) origEnd));
	regsEnd.rip -= 1;
	CALL(ptrace(PTRACE_SETREGS, childPid, 0, &regsEnd));
	
	// continue till next command
	CALL(ptrace(PTRACE_SINGLESTEP, childPid, NULL, NULL));
	CALL(wait(&wait_status));
	// read regs
	CALL(ptrace(PTRACE_GETREGS, childPid, 0, &regsEnd));
	
	// COMPARE
	// iterate over regsNames where not null
	if(regsNames.rax != NULL){
		printDiff(regsNames.rax, regsStart.rax, regsEnd.rax, regsNames.raxSize);
	}
	if(regsNames.rbx != NULL){
		printDiff(regsNames.rbx, regsStart.rbx, regsEnd.rbx, regsNames.rbxSize);
	}
	if(regsNames.rcx != NULL){
		printDiff(regsNames.rcx, regsStart.rcx, regsEnd.rcx, regsNames.rcxSize);
	}
	if(regsNames.rdx != NULL){
		printDiff(regsNames.rcx, regsStart.rcx, regsEnd.rcx, regsNames.rcxSize);
	}
	if(regsNames.rsi != NULL){
		printDiff(regsNames.rsi, regsStart.rsi, regsEnd.rsi, regsNames.rsiSize);
	}
	
	// wait till it diesss
	CALL(ptrace(PTRACE_CONT, childPid, NULL, NULL));
	CALL(wait(&wait_status));
}


int main(int argc, char** argv){
	long int startAddr = strtol(argv[1], NULL, 16);
	long int endAddr = strtol(argv[2], NULL, 16);
	RegsNames regsNames;
	putNull(&regsNames);
	char firstStr[MAX_STR];
	char secondStr[MAX_STR];
	
	while(scanf("%s%s", firstStr, secondStr)){
		//printf("%s|%s\n", firstStr, secondStr);
		if((strcmp(firstStr, "run") == 0) && (strcmp(secondStr, "profile") == 0)){
			break;
		}
		char* allocStr;
		if(checkRegister(secondStr, "rax", "eax", "ax", "al")){
			regsNames.rax = malloc(MAX_STR);
			strcpy(regsNames.rax, firstStr);
			regsNames.raxSize = getRegBytes(secondStr); 
		}
		else if(checkRegister(secondStr, "rbx", "ebx", "bx", "bl")){
			regsNames.rbx = malloc(MAX_STR);
			strcpy(regsNames.rbx, firstStr);
			regsNames.rbxSize = getRegBytes(secondStr); 
		}
		else if(checkRegister(secondStr, "rcx", "ecx", "cx", "cl")){
			regsNames.rcx = malloc(MAX_STR);
			strcpy(regsNames.rcx, firstStr);
			regsNames.rcxSize = getRegBytes(secondStr); 
		}
		else if(checkRegister(secondStr, "rdx", "edx", "dx", "dl")){
			regsNames.rdx = malloc(MAX_STR);
			strcpy(regsNames.rdx, firstStr);
			regsNames.rdxSize = getRegBytes(secondStr); 
		}
		else if(checkRegister(secondStr, "rsi", "esi", "si", "sil")){
			regsNames.rsi = malloc(MAX_STR);
			strcpy(regsNames.rsi, firstStr);
			regsNames.rsiSize = getRegBytes(secondStr); 
		}
		else{
			printf("No such register!\n");
			exit(1);
		}
	}
	/*printf("start: %p\n", startAddr);
	printf("end: %p\n", endAddr);
	printf("rax %s of size %d\n", regsNames.rax, regsNames.raxSize);
	printf("rbx %s of size %d\n", regsNames.rbx, regsNames.rbxSize);
	printf("rcx %s of size %d\n", regsNames.rcx, regsNames.rcxSize);
	printf("rdx %s of size %d\n", regsNames.rdx, regsNames.rdxSize);
	printf("rsi %s of size %d\n", regsNames.rsi, regsNames.rsiSize);
	*/
	pid_t pid = CALL(fork());
	if(pid == 0){
		CALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
		CALL(execv(argv[3], argv + 3));
	}
	else{
		run_profiler(pid, startAddr, endAddr, regsNames);
	}
	
	return 0;
}
