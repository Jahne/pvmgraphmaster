/*master.c*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pvm3.h>

#define TASK_NUMBER 4

#define MAX 7
#define INFINITE 3000

int cost[MAX][MAX] = { { INFINITE, 2, 4, 7, INFINITE, 5, INFINITE }, { 2,
		INFINITE, INFINITE, 6, 3, INFINITE, 8 }, { 4, INFINITE, INFINITE,
		INFINITE, INFINITE, 6, INFINITE }, { 7, 6, INFINITE, INFINITE, INFINITE,
		1, 6 }, { INFINITE, 3, INFINITE, INFINITE, INFINITE, INFINITE, 7 }, { 5,
		INFINITE, 6, 1, INFINITE, INFINITE, 6 }, { INFINITE, 8, INFINITE, 6, 7,
		6, INFINITE } };

double getTime() {
	struct timeval t;
	struct timezone tzp;
	gettimeofday(&t, &tzp);
	return t.tv_sec + t.tv_usec * 1e-6;
}

void sendMallocTab(int ip, int lenght) {

	int infinite = INFINITE;

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

int main() {
	int myId, parentId, taskNo, tIds[TASK_NUMBER];
	int activeTask[taskNo];
	memset(&activeTask, 0, sizeof(activeTask) * taskNo);


	myId = pvm_mytid();

	if (PvmNoParent == (parentId = pvm_parent())) {
		pvm_catchout(stdout);
		if (0
				>= (taskNo = pvm_spawn("Slave", NULL, PvmTaskDefault, "",
						TASK_NUMBER, tIds))) {
			printf("[%x] nie moge stworzyc dzieci\n", myId);
			pvm_perror("pvm_spawn");
			pvm_exit();
			return 1;
		}

		int i = 0;
		int suma[MAX], taskIds[MAX];
		double computingTime[MAX], totalTime[MAX];
		int currentVertex = MAX - 1;

		for (i = 0; i < taskNo; ++i) {
			sendMallocTab(tIds[i], MAX);
		}

		while (currentVertex) {
			if (i >= taskNo)
				i = 0;

			if(!activeTask[i]){
				sendStartVertex(tIds[i], currentVertex);
				activeTask[i] = 1;
			}

			if (activeTask[i]) {
				int arrive = pvm_nrecv(-1, 5);
				if(!(arrive == 0)) //results
				{
					pvm_upkint(&suma[currentVertex], 1, 1);
					pvm_upkdouble(&computingTime[currentVertex], 1, 1);
					pvm_upkdouble(&totalTime[currentVertex], 1, 1);
					taskIds[currentVertex] = tIds[i];
					activeTask[i] = 0;
					currentVertex--;
				}
			}


			i++;

		}

		for (i = 0; i < taskNo; ++i) {
			sendStartVertex(tIds[i], -1);
		}

		for (i = 0; i < MAX; i++) {
			printf(
					"task %d, suma sciezek %d, czas przetwarzania %f, laczny czas %f\n",
					taskIds[i], suma[i], computingTime[i], totalTime[i]);
		}

	} else {
		printf("Program ten powinien być uruchomiony jako master!");
	}
	pvm_exit();
	exit(0);
}
