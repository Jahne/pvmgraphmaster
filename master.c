/*master.c*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pvm3.h>

#define TASK_NUMBER 4

#define MAX 7
#define INFINITE 6000

//int cost[MAX][MAX] = { { INFINITE, 2, 4, 7, INFINITE, 5, INFINITE }, { 2,
//		INFINITE, INFINITE, 6, 3, INFINITE, 8 }, { 4, INFINITE, INFINITE,
//		INFINITE, INFINITE, 6, INFINITE }, { 7, 6, INFINITE, INFINITE, INFINITE,
//		1, 6 }, { INFINITE, 3, INFINITE, INFINITE, INFINITE, INFINITE, 7 }, { 5,
//		INFINITE, 6, 1, INFINITE, INFINITE, 6 }, { INFINITE, 8, INFINITE, 6, 7,
//		6, INFINITE } };

int **cost; //graph

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
	pvm_pkint(*cost, lenght * lenght, 1);
	pvm_send(ip, 10);

}

void sendStartVertex(int ip, int startVertex) {
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&startVertex, 1, 1);
	pvm_send(ip, 7);
}

//zwraca index wlasciwego wierzcholka
int selectShortestDistance(struct _tResult *tab, int length) {
	int shortestIndex = -1, shortestDistance = -1, i;
	for (i = 0; i < length; ++i) {
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

	puts("stworzylem");
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

	puts("wygenerowalem graf");
}

void printGraph(int max) {
	int i, j;
	for (i = 0; i < max; i++) {
		for (j = 0; j < max; j++) {
			printf(" %d,", cost[i][j]);
		}
		printf("\n");
	}
}

int taskNumber, vertexNumber;

int main() {
	puts("Ile dzieci chcesz tworzyc?");
	scanf("%d", &taskNumber);
	puts("Ilu wierzcholkowy graf wygenerowac?");
	scanf("%d", &vertexNumber);

	printf("Tworzysz %d dzieci i graf z %d wierzcholkow\n", taskNumber,
			vertexNumber);
	int myId, parentId, taskNo;
	int *tIds = (int *) malloc(sizeof(int) * vertexNumber);
	myId = pvm_mytid();
	struct _tResult * results = (struct _tResult*) malloc(
			sizeof(struct _tResult) * vertexNumber);

	if (PvmNoParent == (parentId = pvm_parent())) {
		pvm_catchout(stdout);
		if (0
				>= (taskNo = pvm_spawn("Slave", NULL, PvmTaskDefault, "",
						taskNumber, tIds))) {
			printf("[%x] nie moge stworzyc dzieci\n", myId);
			pvm_perror("pvm_spawn");
			pvm_exit();
			return 1;
		}

		int activeTask[taskNo];
		memset(activeTask, 0, sizeof(activeTask) * taskNo);
		int i = 0;
		int currentVertex = vertexNumber - 1;

		printf("Tworzysz %d dzieci i graf z %d wierzcholkow\n", taskNumber,
				vertexNumber);

		generateGraph(vertexNumber);

		puts("graf");
		printf("Tworzysz %d dzieci i graf z %d wierzcholkow\n", taskNumber,
				vertexNumber);

		printGraph(vertexNumber);

		puts("wysylam graf");
		printf("Tworzysz %d dzieci i graf z %d wierzcholkow\n", taskNumber,
				vertexNumber);

		for (i = 0; i < taskNo; ++i) {
			sendMallocTab(tIds[i], vertexNumber);
		}

		puts("rozdzielam zadania i zbieram wyniki");
		i = 0;
		printf("Tworzysz %d dzieci i graf z %d wierzcholkow\n", taskNumber,
				vertexNumber);

		int lastSendVertex = -1;
		while (currentVertex + 1) {
//			puts("while");
			if (i >= taskNo)
				i = 0;

			if (!activeTask[i] && lastSendVertex != currentVertex) {
				printf("stId: %d\n", i);
				sendStartVertex(tIds[i], currentVertex);
				activeTask[i] = tIds[i];
				lastSendVertex = currentVertex;
			}

			int arrive = pvm_nrecv(-1, 5);
			if (!(arrive == 0)) //results
			{
				int tmp1, tmp2, tid = 0;
				pvm_bufinfo(arrive, &tmp1, &tmp2, &tid);
				printf("tmp1: %d, tmp2: %d, tid: %d\n", tmp1, tmp2, tid);
				pvm_upkint(&results[currentVertex].vertex, 1, 1);
				pvm_upkint(&results[currentVertex].suma, 1, 1);
				pvm_upkdouble(&results[currentVertex].computingTime, 1, 1);
				pvm_upkdouble(&results[currentVertex].totalTime, 1, 1);
				results[currentVertex].tId = tid;
				int idx = 0;
				for (idx = 0; idx < taskNo; idx++) {
					if (activeTask[idx] == tid) {
//						printf("recv i: %d\n", idx);
						activeTask[idx] = 0;
						break;
					}
				}
				currentVertex--;
			}

			i++;

		}

		for (i = 0; i < taskNo; ++i) { // rozsylany komunikat o koncu pracy
			sendStartVertex(tIds[i], -1);
		}

		for (i = 0; i < vertexNumber; i++) {
			printf(
					"task %d, suma sciezek %d, czas przetwarzania %f, laczny czas %f\n",
					results[i].tId, results[i].suma, results[i].computingTime,
					results[i].totalTime);
		}

		int resultVertex = selectShortestDistance(results, vertexNumber);
		printf("wierzcholek index: %d, vertex %d, jego wartosc: %d\n",
				resultVertex, results[resultVertex].vertex,
				results[resultVertex].suma);
	} else {
		printf("Program ten powinien być uruchomiony jako master!");
	}
	pvm_exit();
	exit(0);
}
