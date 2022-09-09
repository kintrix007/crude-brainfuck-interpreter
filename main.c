#include <stdlib.h>
#include <stdio.h>

// 100 MB memory.
#define MEMORY_SIZE 104857600

long interpret(long idx);

struct Node {
	struct Node *next;
	int data;
};

unsigned char *memory;
struct Node* head;
char *buffer;
long addr;
int skipTillClose;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("One argument expected, got (%d).\n", argc-1);
		exit(1);
	}
	
	long length;
	char *filename = argv[1];
	FILE *f = fopen(filename, "r");

	if (filename == NULL) {
		printf("Cannot open file '%s'\n", filename);
		exit(4);
	}

	buffer = 0; // if malloc fails buffer is false
	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = malloc(length);
	if (buffer) {
		fread(buffer, 1, length, f);
	} else {
		printf("Couldn't allocate memory.");
		exit(3);
	}

	printf("--- ALLOCATING MEMORY");
	fflush(stdout);
	memory = malloc(MEMORY_SIZE);
	for (long i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
	printf(" DONE. ---\n\n");

	addr = 0;
	head = NULL;
	skipTillClose = 0;
	long strIdx = 0;
	do {
		strIdx = interpret(strIdx);
	} while (strIdx < length && strIdx != -1);

	printf("\n--- CODE FINISHED ---\nMemory Values:\n");

	long hasDataTill = 0;
	for (long i = 0; i < MEMORY_SIZE; i++) {
		if (memory[i] != 0) hasDataTill = i;
	}
	for (long i = 0; i < hasDataTill+1; i++) {
		printf("%d ", memory[i]);
	}
	printf("\n");
}

inline long interpret(long idx) {
	char ch = buffer[idx];

	switch (ch) {
		case '+':
			if (!skipTillClose) memory[addr]++;
			return ++idx;
		case '-':
			if (!skipTillClose) memory[addr]--;
			return ++idx;
		case '>':
			if (skipTillClose == 0) addr++;
			if (addr >= MEMORY_SIZE) {
				printf("Memory size exceeded.\n");
				exit(5);
			}
			return ++idx;
		case '<':
			if (skipTillClose == 0) addr--;
			if (addr < 0) addr = 0;
			return ++idx;
		case '[':
			if (skipTillClose > 0 || memory[addr] == 0) {
				skipTillClose++;
				return ++idx;
			} else {
				if (head == NULL) {
					head = (struct Node*)malloc(sizeof(struct Node));
					head->data = idx;
					head->next = NULL;
				} else {
					struct Node *node = (struct Node*)malloc(sizeof(struct Node));
					node->next = head;
					node->data = idx;
					head = node;
				}
				return ++idx;
			}
		case ']':
			if (skipTillClose > 0) {
				skipTillClose--;
				return ++idx;
			} else {
				if (memory[addr] != 0) {
					if (head == NULL) {
						printf("Syntax error at character #%d. Unexpected ']'", idx);
						exit(10);
					}
					idx = head->data;
					head = head->next;
					return idx;
				} else {
					return ++idx;
				}
			}
		case '.':
			putc(memory[addr], stdout);
			return ++idx;
		case ',':
			memory[addr] = getc(stdin);
			return ++idx;
		case EOF:
			return -1;
		default:
			return ++idx;

	}
}
