#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/ipc.h>

int main() {

    // generowanie klucza
    key_t klucz = ftok("/tmp/konto.ipc", 'A');
    if (klucz == -1) {
        perror("ftok");
        exit(1);
    }

    // pobranie ID segmentu pamięci współdzielonej
    int shmid = shmget(klucz, sizeof(int), 0600);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // usunięcie segmentu
		//realnie zostanie usuniety dopiero, gdy zaden proces nie bedzie podlaczony
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    } else {
		printf("Segment pamieci wspoldzielonej o shmid=%d zostal usuniety\n", shmid);
	}

	//SEMAFOR
		//pobranie ID semafora (1 semafor w zestawie)
    int semid = semget(klucz, 1, 0600);
    if (semid == -1) {
        perror("semget (semafor)");
    } else {
        // usunięcie semafora nr 0 (czyli pierwszy)
        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl(IPC_RMID)");
        } else {
            printf("Semafor o semid=%d zostal usuniety\n", semid);
        }
    }
    return 0;
}

