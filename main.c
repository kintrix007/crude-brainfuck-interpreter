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

void buildAST(FILE *f, int **ast);
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

	buildAST(f, &ast);

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
				if (jCloseIdx == -1) putc(mem[memIdx], stdout);	// Could use a syscall instead in assembly

				astIdx++;
				break;
			case READ:
				if (jCloseIdx == -1) mem[memIdx] = getc(stdin);	// Could use a syscall instead in assembly

				astIdx++;
				break;
		}
	} while (astIdx < astSize);
}

inline void buildAST(FILE *f, int **pAst) {
	astIdx = -1;
	astSize = 0;
	*pAst = NULL;

	int astElem;
	char ch;
	do {
		ch = getc(f);
		switch (ch) {
			case '+':
				astElem = INC;
				break;
			case '-':
				astElem = DEC;
				break;
			case '>':
				astElem = RIGHT;
				break;
			case '<':
				astElem = LEFT;
				break;
			case '[':
				astElem = OPEN;
				break;
			case ']':
				astElem = CLOSE;
				break;
                        case '.':
				astElem = WRITE;
                                break;
                        case ',':
				astElem = READ;
                                break;
			default:
				continue;
		}

		// Add AST element
		astIdx++;

		if (astSize == 0) {
			astSize = 1;
			*pAst = (int*)malloc(astSize * sizeof(int));	
			astIdx = 0;
		} else if (astIdx >= astSize) {
			astSize *= 2;
			*pAst = (int*)realloc(*pAst, astSize * sizeof(int));
		}

		(*pAst)[astIdx] = astElem;
	} while (ch != EOF);
}
