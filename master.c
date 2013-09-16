/*master.c*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pvm3.h>

#define MAX 7
#define INFINITE 3000

int costTmp[MAX][MAX] = { { INFINITE, 2, 4, 7, INFINITE, 5, INFINITE }, { 2,
		INFINITE, INFINITE, 6, 3, INFINITE, 8 }, { 4, INFINITE, INFINITE,
		INFINITE, INFINITE, 6, INFINITE }, { 7, 6, INFINITE, INFINITE, INFINITE,
		1, 6 }, { INFINITE, 3, INFINITE, INFINITE, INFINITE, INFINITE, 7 }, { 5,
		INFINITE, 6, 1, INFINITE, INFINITE, 6 }, { INFINITE, 8, INFINITE, 6, 7,
		6, INFINITE } };

int **cost; //graph
int infinite = 0; //brak polaczenia
int *graph; //one dimensional array of graph
int tasksNumber, vertexsNumber;

//struktura przechowujaca wyniki
struct _tResult {
	int suma;
	int tId;
	double computingTime;
	double totalTime;
	int vertex;
};

double getTime() {
	struct timeval t;
	struct timezone tzp;
	gettimeofday(&t, &tzp);
	return t.tv_sec + t.tv_usec * 1e-6;
}

void sendMallocTab(int ip, int lenght) {

	int infinite = INFINITE;

	printf("ip: %d\n", ip);
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&lenght, 1, 1);
	pvm_pkint(&infinite, 1, 1);
	pvm_pkint(graph, lenght * lenght, 1);
	pvm_send(ip, 10);

}

void saveGraph(int size) {
	FILE *fp; /* używamy metody wysokopoziomowej - musimy mieć zatem identyfikator pliku, uwaga na gwiazdkę! */
	if ((fp = fopen("graph.gph", "w+")) == NULL ) {
		printf("Nie mogę otworzyć pliku graph.gph do zapisu!\n");
		exit(1);
	}
	fprintf(fp, "%d %d\n", size, infinite); /* zapisz nasz łańcuch w pliku */
	int i, j = 0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			fprintf(fp, "%d ", cost[i][j]);
		}
//		fprintf(fp, "\n");
	}
	fclose(fp); /* zamknij plik */
	return;
}

void readGraph() {
	FILE *fp;
	if ((fp = fopen("graph.gph", "r")) == NULL ) {
		printf("Nie mogę otworzyć pliku graph.gph do odczytu!\n");
		exit(1);
	}

	fscanf(fp, "%d %d\n", &vertexsNumber, &infinite);
	printf("vertexNumber: %d, infinite: %d\n", vertexsNumber, infinite);

	graph = realloc(graph, sizeof(int) * vertexsNumber * vertexsNumber);

	int row = 0;
	int col = 0;
	int readValue = -1;
	while (1) {
		int ret = fscanf(fp, "%d", &readValue); //bez wczytywania /n
		if (ret == 1) {
			graph[row * (vertexsNumber - 1) + col] = readValue;
			printf("%d, ", graph[col]);
			col++;
			if (col >= vertexsNumber) {
				col = 0;
				row++;
			}
		} else if (ret == EOF) {
			break;
		} else {
			printf("No match.\n");
		}

	}

	puts("");
	fclose(fp); /* zamknij plik */
	return;
}

void sendStartVertex(int ip, int startVertex) {
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&startVertex, 1, 1);
	pvm_send(ip, 7);
}

//zwraca index wlasciwego wierzcholka
int selectShortestDistance(struct _tResult *tab, int length) {
	int shortestIndex = -1, shortestDistance = -1, i;
	for (i = 0; i < length; i++) {
		if ((shortestIndex == -1 && shortestDistance == -1)
				|| (shortestDistance > tab[i].suma)) {
			shortestDistance = tab[i].suma;
			shortestIndex = i;
		}
	}
	return shortestIndex;
}

int random_number(int min_num, int max_num) {
	int result = 0, low_num = 0, hi_num = 0;
	if (min_num < max_num) {
		low_num = min_num;
		hi_num = max_num + 1; // this is done to include max_num in output.
	} else {
		low_num = max_num + 1; // this is done to include max_num in output.
		hi_num = min_num;
	}
	result = (rand() % (hi_num - low_num)) + low_num;
	return result;
}

void generateGraph(int vertexNumber) {
	int number = 0;
	srand(time(NULL ));
	puts("tworze");

	cost = malloc(sizeof *cost * vertexNumber);
	if (cost) {
		int i;
		for (i = 0; i < vertexNumber; i++) {
			cost[i] = malloc(sizeof *cost[i] * vertexNumber);
		}
	}

	for (number = 0; number < vertexNumber; number++) {
		cost[number][number] = INFINITE;
		int next = 0;
		for (next = number + 1; next < vertexNumber; next++) {
			int distance = INFINITE;
			if (!(random_number(0, 3) == 2)) { //jesli istnieje polaczenie
				distance = random_number(1, 5);
				printf("distance: %d\n", distance);
			}

			cost[number][next] = distance;
			cost[next][number] = distance;

		}
	}

}

void printGraph(int max) {
	int i, j = 0;

	puts("graf graph");
	for (i = 0; i < max; i++) {
		for (j = 0; j < max; j++) {
			printf(" %d,", graph[i * (max - 1) + j]);
		}
		printf("\n");
	}
}

//void generateGraph(int max) {
//	int j, i = 0;
//
//	cost = malloc(sizeof *cost * max);
//	if (cost) {
//		int i;
//		for (i = 0; i < max; i++) {
//			cost[i] = malloc(sizeof *cost[i] * max);
//		}
//	}
//
//	for (i = 0; i < max; i++) {
//		for (j = 0; j < max; j++) {
//			cost[i][j] = costTmp[i][j];
//		}
//	}
//}

void rewrideArrays(int length) {
	int lengthFor2D = length * length;
	int i, j = 0;
	graph = realloc(graph, lengthFor2D * sizeof(int));
	for (i = 0; i < length; i++) {
		for (j = 0; j < length; j++) {

			graph[i * (length - 1) + j] = cost[i][j];
		}
	}

	free(cost);
}

int main() {
	infinite = INFINITE;
	int nhost, narch, hostsNumber;
	struct pvmhostinfo *hosts;
	pvm_config(&nhost, &narch, &hosts);

	printf("Na ilu hostach odpalić? (max %d)\n", nhost);
	scanf("%d", &hostsNumber);
	if (hostsNumber < 0 || hostsNumber > nhost) {
		puts(
				"Liczba hostow poza dostepnym zakresem, wiec zostanie ustawiona na 1 hosta");
		hostsNumber = 1;
	}
	puts("Ile dzieci chcesz stworzyc na kazdym z hostow?");
	scanf("%d", &tasksNumber);
	printf("Na %d hostach uruchamiasz po %d dzieci.\n", hostsNumber,
			tasksNumber);

	puts("Wybierz co chcesz zrobic!");
	int option = 0;
	do {
		puts("1. Wczytac graf.");
		puts("2. Wygenerowac nowy graf.");
		scanf("%d", &option);
	} while (option != 1 && option != 2);
	if (option == 1) {
		readGraph();
		printGraph(vertexsNumber);
	} else if (option == 2) {
		puts("Ilu wierzcholkowy graf wygenerowac? (min 5)");
		scanf("%d", &vertexsNumber);
		if (5 > vertexsNumber) {
			puts("Ustawiono 5 wierzcholkow");
			vertexsNumber = 5;
		}
		generateGraph(vertexsNumber);
		saveGraph(vertexsNumber);
		rewrideArrays(vertexsNumber);
	}

	int *tIds = (int *) malloc(sizeof(int) * tasksNumber * hostsNumber);
	int *tIdsTmp = (int *) malloc(sizeof(int) * tasksNumber);
	int myId, parentId, taskNo, i = 0, tIdsMax = 0;
	myId = pvm_mytid();
	struct _tResult * results = (struct _tResult*) malloc(
			sizeof(struct _tResult) * vertexsNumber);

	if (PvmNoParent == (parentId = pvm_parent())) {
//		pvm_catchout(stdout);
		for (i = 0; i < hostsNumber; i++) {

			if (0
					>= (taskNo = pvm_spawn("Slave", NULL, PvmTaskHost,
							hosts[i].hi_name, tasksNumber, tIdsTmp))) {
				printf("%s [%x] nie moge stworzyc dzieci\n", hosts[i].hi_name,
						myId);
				pvm_perror("pvm_spawn");
			} else {
				tIds = realloc(tIds, sizeof(int) * (tIdsMax + taskNo));

				for (i = 0; i < taskNo; i++) {
					tIds[i + tIdsMax] = tIdsTmp[i];
				}

				tIdsMax += taskNo;
			}
		}

		int activeTask[tIdsMax];
		memset(activeTask, 0, sizeof(int) * tIdsMax);
		int i = 0;
		int vertexNumberCondition = vertexsNumber - 1;

		for (i = 0; i < tIdsMax; i++) {
			sendMallocTab(tIds[i], vertexsNumber);
		}

		puts("rozdzielam zadania i zbieram wyniki");

		int lastSendedVertex = vertexNumberCondition;

		while (vertexNumberCondition + 1) {
			if (i >= tIdsMax)
				i = 0;

			if (!activeTask[i] && lastSendedVertex >= 0) {
				printf("stId: %d, tId: %d \n", i, tIds[i]);
				sendStartVertex(tIds[i], lastSendedVertex--);
				activeTask[i] = tIds[i];
			}

			int arrive = pvm_nrecv(-1, 5);
			if (!(arrive == 0)) //results
			{
				int tmp1, tmp2, tid = 0;
				pvm_bufinfo(arrive, &tmp1, &tmp2, &tid);
				pvm_upkint(&results[vertexNumberCondition].vertex, 1, 1);
				pvm_upkint(&results[vertexNumberCondition].suma, 1, 1);
				pvm_upkdouble(&results[vertexNumberCondition].computingTime, 1,
						1);
				pvm_upkdouble(&results[vertexNumberCondition].totalTime, 1, 1);
				results[vertexNumberCondition].tId = tid;
				int idx = 0;
				for (idx = 0; idx < tIdsMax; idx++) {
					if (activeTask[idx] == tid) {
						activeTask[idx] = 0;
						break;
					}
				}
				vertexNumberCondition--;
			}

			i++;

		}

		for (i = 0; i < tIdsMax; ++i) { // rozsylany komunikat o koncu pracy
			sendStartVertex(tIds[i], -1);
		}

		for (i = 0; i < vertexsNumber; i++) {
			printf(
					"task %d, suma sciezek %d, czas przetwarzania %f, laczny czas %f\n",
					results[i].tId, results[i].suma, results[i].computingTime,
					results[i].totalTime);
		}

		int resultVertex = selectShortestDistance(results, vertexsNumber);
		printf("wierzcholek index: %d, vertex %d, jego wartosc: %d\n",
				resultVertex, results[resultVertex].vertex,
				results[resultVertex].suma);
	} else {
		printf("Program ten powinien być uruchomiony jako master!");
	}
	pvm_exit();
	exit(0);
}
