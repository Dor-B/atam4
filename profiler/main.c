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
#define XH_SIZE -2

typedef struct reg_node{
	char* var;
	char* reg;
	struct reg_node* next;
} RegNode;

RegNode* createRegNode(char *var, char*reg){
	RegNode* regNode = malloc(sizeof(RegNode));
	regNode->var = malloc(MAX_STR);
	strcpy(regNode->var, var);
	regNode->reg = malloc(4);
	strcpy(regNode->reg, reg);
	regNode->next =  NULL;
	return regNode;
}

void destroyRegNode(RegNode* node){
	if(node == NULL)
		return;
	free(node->var);
	free(node->reg);
	free(node);
	destroyRegNode(node->next);
}

void insertSorted(RegNode** head, RegNode* node){
	if(*head == NULL){
		*head = node;
		return;
	}
	if(strcmp((*head)->var, node->var) > 0){
		node->next = *head;
		*head = node;
		return;
	}
	RegNode* curr = *head;
	for(; curr->next != NULL && (strcmp(curr->next->var, node->var) < 0); curr = curr->next){}
	node->next = curr->next;
	curr->next = node;
}



bool checkRegister(char* str, char* reg1, char* reg2, char* reg3, char* reg4, char* reg5){
	return (strcmp(str, reg1) == 0) || (strcmp(str, reg2) == 0) || (strcmp(str, reg3) == 0) || (strcmp(str, reg4) == 0) || (strcmp(str, reg5) == 0);
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
	if((reg[1] == 'h') || (reg[2] == 'h')){
		return XH_SIZE;
	}
	return 2;
}

unsigned long long int toSize(unsigned long long int num, int bytes){
	if(bytes == XH_SIZE){
		return (num << (64 - 16)) >> (64 - 8);
	}
	int toShift = 64 - bytes*8;
	return (num << toShift) >> toShift;
}

void printDiff(char* varName, unsigned long long int before, unsigned long long int after, int size){
	before = toSize(before, size);
	after = toSize(after, size);
	
	if(before != after){
		printf("PRF:: %s: %llu->%llu\n", varName, before, after);
	}
}

long CALL(long res){
	if(res == -1){
		perror("err");
		exit(1);
	}
	return res;
}

void run_profiler(pid_t childPid,unsigned long long int startAddr,unsigned long long int endAddr, RegNode* varList){
	//printf("run_profile\n");
	struct user_regs_struct regsStart;
	struct user_regs_struct regsEnd;
	
	int wait_status;
	CALL(wait(&wait_status));
	
	// set breakpoint at start
	long origStart = CALL(ptrace(PTRACE_PEEKTEXT, childPid, (void*) startAddr, NULL));
	unsigned long startTrap = (origStart & 0xFFFFFFFFFFFFFF00) | 0xCC;
	
	// compute breakpoint at end
	long origEnd = CALL(ptrace(PTRACE_PEEKTEXT, childPid, (void*) endAddr, NULL));
	unsigned long endTrap = (origEnd & 0xFFFFFFFFFFFFFF00) | 0xCC;
	
	// let the child run till start
	CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) startAddr, (void *) startTrap));
	CALL(ptrace(PTRACE_CONT, childPid, NULL, NULL));
	
	wait(&wait_status);
	
	while(WIFSTOPPED(wait_status)){
		
		// recover start
		CALL(ptrace(PTRACE_GETREGS, childPid, 0, &regsStart));
		CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) startAddr, (void *) origStart));
		regsStart.rip -= 1;
		CALL(ptrace(PTRACE_SETREGS, childPid, 0, &regsStart));
		
		
		// set breakpoint at end
		CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) endAddr, (void *) endTrap));
		
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
		// iterate over list
		for(RegNode* curr = varList; curr != NULL; curr = curr->next){
			if(checkRegister(curr->reg, "rax", "eax", "ax", "al", "ah")){
				printDiff(curr->var, regsStart.rax, regsEnd.rax, getRegBytes(curr->reg));
			}
			else if(checkRegister(curr->reg, "rbx", "ebx", "bx", "bl", "bh")){
				printDiff(curr->var, regsStart.rbx, regsEnd.rbx, getRegBytes(curr->reg));
			}
			else if(checkRegister(curr->reg, "rcx", "ecx", "cx", "cl", "ch")){
				printDiff(curr->var, regsStart.rcx, regsEnd.rcx, getRegBytes(curr->reg));
			}
			else if(checkRegister(curr->reg, "rdx", "edx", "dx", "dl", "dh")){
				printDiff(curr->var, regsStart.rdx, regsEnd.rdx, getRegBytes(curr->reg));
			}
			else if(checkRegister(curr->reg, "rsi", "esi", "si", "sil", "XXX")){
				printDiff(curr->var, regsStart.rsi, regsEnd.rsi, getRegBytes(curr->reg));
			}
			else{
				printf("No such register!\n");
				exit(1);
			}
		}
		
		
		// wait till next breakpoint (of start) or process dies
		CALL(ptrace(PTRACE_POKETEXT, childPid, (void*) startAddr, (void *) startTrap));
		CALL(ptrace(PTRACE_CONT, childPid, NULL, NULL));
		CALL(wait(&wait_status));
	}
}


int main(int argc, char** argv){
	unsigned long long int startAddr = strtoull(argv[1], NULL, 16);
	unsigned long long int endAddr = strtoull(argv[2], NULL, 16);
	
	char firstStr[MAX_STR];
	char secondStr[MAX_STR];
	RegNode* varList = NULL;
	while(scanf("%s%s", firstStr, secondStr)){
		if((strcmp(firstStr, "run") == 0) && (strcmp(secondStr, "profile") == 0)){
			break;
		}
		RegNode* newNode = createRegNode(firstStr, secondStr);
		insertSorted(&varList, newNode);
	}

	pid_t pid = CALL(fork());
	if(pid == 0){
		CALL(ptrace(PTRACE_TRACEME, 0, NULL, NULL));
		CALL(execv(argv[3], argv + 3));
	}
	else{
		run_profiler(pid, startAddr, endAddr, varList);
		destroyRegNode(varList);
	}
	
	return 0;
}
