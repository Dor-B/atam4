#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX_STR 257

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
int main(int argc, char** argv){
	long int startAdrr = strtol(argv[1], NULL, 16);
	long int endAdrr = strtol(argv[2], NULL, 16);
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
	/*printf("start: %p\n", startAdrr);
	printf("end: %p\n", endAdrr);
	printf("rax %s of size %d\n", regsNames.rax, regsNames.raxSize);
	printf("rbx %s of size %d\n", regsNames.rbx, regsNames.rbxSize);
	printf("rcx %s of size %d\n", regsNames.rcx, regsNames.rcxSize);
	printf("rdx %s of size %d\n", regsNames.rdx, regsNames.rdxSize);
	printf("rsi %s of size %d\n", regsNames.rsi, regsNames.rsiSize);
	*/
	return 0;
}
