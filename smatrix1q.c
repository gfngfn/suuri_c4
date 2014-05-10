#include <stdio.h>
#include <stdlib.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define TRU 1
#define FLS 0

#define END_OF_LIST NULL

#define mymalloc(p, type, n) {\
	p = (type*)malloc(sizeof(type) * n);\
	if(p == NULL) {\
		printf("[mymalloc] not enough memories\n");\
		exit(EXIT_FAILURE);\
	}\
}

#define myfopen(fp, fn, m) {\
	fp = fopen(fn, m);\
	if(fp == NULL) {\
		printf("[myfopen] cannot open file '%s'\n", fn);\
		exit(EXIT_FAILURE);\
	}\
}

typedef int BOOL;

typedef double data;

typedef struct slobj_ {
	struct slobj_ *next;
	int col;
	data v;
} slobj;

typedef struct {
	int row, col;
	slobj **root;
} smatrix;

smatrix *smatrix_new(int, int);
void smatrix_insert(smatrix*, int, int, data);
data smatrix_access(smatrix*, int, int);
void smatrix_print(smatrix*);

smatrix *smatrix_read(char*);
void smatrix_write(char*, smatrix*);
smatrix *product(smatrix*, smatrix*);
void smatrix_free(smatrix*);

int main(int argc, char *argv[]) {
	smatrix *a, *b, *c;

	if(argc != 4) {
		printf("[main] wrong number of arguments\n");
		return EXIT_FAILURE;
	} else {
		a = smatrix_read(argv[1]);
		b = smatrix_read(argv[2]);
		c = product(a, b);

		printf("A:\n"); smatrix_print(a);
		printf("B:\n"); smatrix_print(b);	
		printf("C(=A*B):\n"); smatrix_print(c);

		smatrix_write(argv[3], c);
		printf("results saved in '%s'\n", argv[3]);

		smatrix_free(a);
		smatrix_free(b);
		smatrix_free(c);
		return EXIT_SUCCESS;
	}
}

/*---- begin middle level problems ----*/

smatrix *smatrix_new(int n, int m) {
	smatrix *smt;
	slobj **ppo;
	int i;

	mymalloc(smt, smatrix, 1);
	mymalloc(ppo, slobj*, n);
	for(i = 0; i < n; i++) {
		mymalloc(ppo[i], slobj, 1);
		ppo[i] = END_OF_LIST;
	}

	smt->row = n;
	smt->col = m;
	smt->root = ppo;

	return smt;
}

void smatrix_insert(smatrix *smt, int n, int m, data x) {
	slobj *po, *pof, *ponew;
	BOOL ismiddle;

	mymalloc(ponew, slobj, 1);
	ponew->col = m;
	ponew->v = x;

	po = (smt->root)[n];
	pof = NULL;
	ismiddle = FLS;
	while(po != END_OF_LIST) {
		if(m < po->col) {
			ismiddle = TRU;
			break;
		}
		pof = po;
		po = po->next;
	}

	if(ismiddle) {
		ponew->next = po;
	} else {
		ponew->next = END_OF_LIST;
	}
	if(pof == NULL) {
		/* if the n-th row has no non-zero element */ 
		(smt->root)[n] = ponew;
	} else {
		pof->next = ponew;
	}
	return;
}

data smatrix_access(smatrix *smt, int n, int m) {
	slobj *po;

	po = (smt->root)[n];

	while(po != END_OF_LIST) {
		if(po->col == m) {
			return po->v;
		} else {
			if(po->col > m) { break; }
		}
		po = po->next;
	}
	
	return 0.0;
}

/*---- end middle level problems ----*/

void smatrix_print(smatrix *smt) {
	slobj *po;
	int i, j, k;

	i = 0;
	for(i = 0; i < smt->row; i++) {
		po = (smt->root)[i];
		printf("|");
		j = 0;
		while(po != END_OF_LIST) {
			for(k = (j == 0 ? 0 : j+1); k < po->col; k++) { printf(" %2.5e", 0.0); }
				/* fill zero between non-zero elements
					(j == 0 is the exceptional case of beginning a row) */
			printf(" %2.5e", po->v);

			j = (po->col == 0 ? 1 : po->col);
				/* po->col == 0 is the case where (i,0) element is non-zero */

			po = po->next;
		}
		for(k = (j == 0 ? 0 : j+1); k < smt->col; k++) { printf(" %2.5e", 0.0); }
			/* fill zero after the end of list
				(j == 0 is the exceptional case of the nonexistence of non-zero element in i-th row) */
		printf(" |\n");
	}
	return;
}

/*---- begin high level problems ----*/

smatrix *smatrix_read(char *filename) {
	smatrix *smt;
	FILE *fp;
	int n, m, i, sci;
	data scf;

	myfopen(fp, filename, "r");

	fscanf(fp, "%d %d", &n, &m);
	smt = smatrix_new(n, m);

	for(i = 0; i < n; i++) {
		while(TRU) {
			fscanf(fp, "%d", &sci);
			if(sci == -1) {
				break;
			} else {
				if(sci < 1 || m < sci) {
					printf("[smatrix_read] file '%s' contains illegal matrix data\n", filename);
					exit(EXIT_FAILURE);
				} else {
					fscanf(fp, "%lf", &scf);
					smatrix_insert(smt, i, sci-1, scf);
				}
			}
		}
	}

	fclose(fp);
	return smt;
}

void smatrix_write(char *filename, smatrix *smt) {
	FILE *fp;
	slobj *po;
	int i;

	myfopen(fp, filename, "w");

	fprintf(fp, "%d %d\n", smt->row, smt->col);

	for(i = 0; i < smt->row; i++) {
		po = (smt->root)[i];
		while(po != END_OF_LIST) {
			fprintf(fp, "%d %lf ", (po->col)+1, po->v);
			po = po->next;
		}
		fprintf(fp, "%d\n", -1);
	}

	fclose(fp);
	return;
}

smatrix *product(smatrix *smta, smatrix *smtb) {
	smatrix *smtc;
	slobj *poa;
	int i, j, k;
	data sum, dta, dtb;
	
	if(smta->col != smtb->row) {
		printf("[product] product undefined between (%d, %d) and (%d, %d)\n", smta->row, smta->col, smtb->row, smtb->col);
		return NULL;
	} else {

		smtc = smatrix_new(smta->row, smtb->col);

		for(i = 0; i < smta->row; i++) {
			for(j = 0; j < smtb->col; j++) {

				sum = 0.0;
				poa = (smta->root)[i];
				while(poa != END_OF_LIST) {
					k = poa->col;
					dta = poa->v;
					dtb = smatrix_access(smtb, k, j);
						/* dta contains a_(i, k)
							make dtb b_(k, j) i.e. the counterpart of a_(i, k). */

					if(dtb != 0.0) { sum += dta * dtb; }
						/* c(i, j) = sum(a_(i, k) * b_(k, j)) */

					poa = poa->next;
				}

				if(sum != 0.0) { smatrix_insert(smtc, i, j, sum); }
			}
		}

		return smtc;
	}
}

void smatrix_free(smatrix *smt) {
	int i;
	slobj *po, *ponx;

	for(i = 0; i < smt->row; i++) {
		po = (smt->root)[i];
		while(po != END_OF_LIST) {
			ponx = po->next;
			free(po);
			po = ponx;
		}
	}
	free(smt->root);
	free(smt);

	return;
}

/*---- end hard level problems ----*/

