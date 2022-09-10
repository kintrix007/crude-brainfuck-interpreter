#include <stdio.h>
#include <stdlib.h>

#define INC 0
#define DEC 1
#define RIGHT 2
#define LEFT 3
#define OPEN 4
#define CLOSE 5
#define WRITE 6
#define READ 7

unsigned char *mem;
long memSize;
long memIdx;
long *stack;
long stackSize;
long stackIdx;
int *ast;
long astSize;
long astIdx;

void buildAST(FILE *f);
void addAstElement(int elem);
void interpret();

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("One argument expected, got %d.\n", argc-1);
		exit(1);
	}

	char *filename = argv[1];
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		printf("Could not open file '%s'.\n", filename);
		exit(4);
	}

	buildAST(f);

	astIdx = 0;
	interpret();
}

void interpret() {
	stackIdx = -1;
	stackSize = 0;
	stack = NULL;
	memIdx = 0;
	memSize = 1;
	mem = malloc(1);
	mem[0] = 0;
	long jCloseIdx = -1;

	int astElem;
	do {
		astElem = ast[astIdx];
		switch (astElem) {
			case INC:
				if (jCloseIdx == -1) mem[memIdx]++;

				astIdx++;
				break;
			case DEC:
				if (jCloseIdx == -1) mem[memIdx]--;

				astIdx++;
				break;
			case RIGHT:
				if (jCloseIdx == -1) {
					memIdx++;
					if (memIdx >= memSize) {
						memSize *= 2;
						mem = (unsigned char*)realloc(mem, memSize);
						// Division by two is probably more efficient
						// than caching the previous value
						for (int i = memSize/2; i < memSize; i++) {
							mem[i] = 0;
						}
					}
				}

				astIdx++;
				break;
			case LEFT:
				if (jCloseIdx == -1) {
					memIdx--;
					if (memIdx < 0) {
						printf("Unexpected '<'. Memory pointer is already 0.\n");
						exit(5);
					}
				}

				astIdx++;
				break;
			case OPEN:
				stackIdx++;

				if (jCloseIdx == -1) {
					if (stackSize == 0) {
						stackSize = 1;
						stack = (long*)malloc(stackSize * sizeof(long));
						stackIdx = 0;
					} else if (stackIdx >= stackSize) {
						stackSize *= 2;
						stack = (long*)realloc(stack, stackSize * sizeof(long));
					}

					stack[stackIdx] = astIdx;

					if (mem[memIdx] == 0) {
						jCloseIdx = stackIdx;
					}
				}

				astIdx++;
				break;
			case CLOSE:
				if (jCloseIdx == -1) { // Reached without jumping
					if (stackIdx == -1) {
						printf("Unexpected ']'.\n");
						exit(6);
					}

					if (mem[memIdx] != 0) {
						astIdx = stack[stackIdx] + 1;
					} else {
						astIdx++;
						stackIdx--;
					}
				} else { // Reached while jumping
					if (jCloseIdx == stackIdx) {
						jCloseIdx = -1;
					}

					astIdx++;
					stackIdx--;
				}
				break;
			case WRITE:
				if (jCloseIdx == -1) putc(mem[memIdx], stdout);

				astIdx++;
				break;
			case READ:
				if (jCloseIdx == -1) mem[memIdx] = getc(stdin);

				astIdx++;
				break;
		}
	} while (astIdx < astSize);
}

inline void addAstElement(int elem) {
	astIdx++;

	if (astSize == 0) {
		astSize = 1;
		ast = (int*)malloc(astSize * sizeof(int));	
		astIdx = 0;
	} else if (astIdx >= astSize) {
		astSize *= 2;
		ast = (int*)realloc(ast, astSize * sizeof(int));
	}

	ast[astIdx] = elem;
}

inline void buildAST(FILE *f) {
	astIdx = -1;
	astSize = 0;
	ast = NULL;

	char ch;
	do {
		ch = getc(f);
		switch (ch) {
			case '+':
				addAstElement(INC);
				break;
			case '-':
				addAstElement(DEC);
				break;
			case '>':
				addAstElement(RIGHT);
				break;
			case '<':
				addAstElement(LEFT);
				break;
			case '[':
				addAstElement(OPEN);
				break;
			case ']':
				addAstElement(CLOSE);
				break;
                        case '.':
                                addAstElement(WRITE);
                                break;
                        case ',':
                                addAstElement(READ);
                                break;
		}
	} while (ch != EOF);
}
