#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>   // open, O_CREAT, O_RDWR
#include <unistd.h>  // close
#include <time.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>


int main(int argc, char *argv[]) {
	
	if (argc != 2){
		fprintf(stderr, "Bledna ilosc argumentow.\nUzycie: %s <wartosc_zmiany>", argv[0]);
		exit(1);	
	} else {
		printf("Poprawnie zainterpretowano wartosc argumentu: %s\n. Jezeli jest poprawny, stan konta zostanie zmieniony wielokrotnie\n", argv[1]);
	}
	
	bool stop=false;
	//zamiana argumentu na int
	int zmiana = atoi(argv[1]);
	if (zmiana == 0){
		fprintf(stderr, "Blad: Wartosc argumentu jest bledna lub wynosi 0\n");
		stop = true;
	}

	//deklaracja struktury do nanosleep
	struct timespec ts = {0, 50}; //5ns

	// tworzenie pliku lub weryfikacja istnienia (ftok nie pozwala na cos takiego)
		int tmpfd = open("/tmp/konto.ipc", O_CREAT | O_RDWR, 0666);
	if (tmpfd == -1) {
		perror("open");
		exit(1);
	} else {
	printf("Plik do generowania klucza IPC istnieje lub zostal utworzony.\n");
	}
	//zamykamy deskryptor, nie bedziemy go uzywac
	if (close(tmpfd) == -1) {
		perror("close");
		exit(1);
	}
	
	//utworzenie klucza
	key_t klucz = ftok("/tmp/konto.ipc", 'A'); //' zamiast " = char zamist *char, kod ASCII zamiast wskaznika
	if (klucz == -1){
		perror("ftok");
		exit(1);
	} else {
		printf("Klucz zostal pomyslnie utworzony\n");
	}

//SEMAFORY
		
	int semid = semget(klucz, 1, 0600 | IPC_CREAT | IPC_EXCL);
	//przy tych flagach, jezeli semafor istnieje, to zwraca -1 errno EEXIST
	if (semid == -1){
		if (errno==EEXIST){
			//nasz semafor juz istnieje - musimy jeszce raz zainicjowac, bo poprzednia komenda zwrocila -1
			semid = semget(klucz, 1, 0600);
			if(semid==-1){
				perror("semget");exit(1);
			}else{printf("Semafor o id %d zostal pobrany do uzycia\n", semid);}
		} else{perror("semget"); exit(1);}
	
	}else {
		//semafor zostal stworzony przez nas teraz - wiec mozemy ustalic wartosci poczatkowe
		semctl(semid, 0, SETVAL, 1); //semafor nr 0 (pierwszy) dostaje wartosc 1 na start
		printf("Semafor id=%d jest gotowy do uzycia przez pierwszy zainteresowany proces!", semid);
	} 
	


//SHARED MEMORY	
	//tworzymy segment pamieci
	int shmid = shmget(klucz, sizeof(int), 0600 | IPC_CREAT);
			// IPC_CREAT - jezeli nie istnieje, to go utworz
			// jezeli istnieje - ok, uzywam go i zwracam id	
	if (shmid == -1) {
		perror("shmget");
		exit(1);
	} else {
		printf("Segment o id=%d w pamieci istnieje - istnial, lub zostal utworzony\n", shmid);
	}
	
	//mapujemy adres segmentu do przestrzeni adresowej naszego procesu
		//shmat -> shm attach
	int *shared_val = (int*) shmat(shmid, NULL, 0);
	if (shared_val == (void*) -1){
		perror("shmat");
		exit(1);
	} else {
		printf("Wskaznik na adres segmentu zostal pomyslnie dodany do naszego procesu\n");
	}
		//wskaznika int* nie mozemy powrownac ze zwyklym int
		//wiec void rzutuje -1 na typ pod wskaznikiem		



//SEKCJA KRYTYCZNA
	
	//deklaracja operacji P
	struct sembuf sem_lock = {0, -1, 0};
		//semafor nr 0, zmniejsz wartosc (zajmij go), bez flag (domyslnie: blokujaco)
	
	printf("Probuje zajac semmafor id=%d\n", semid);
	//wykonanie operacji -> ZAJECIE SEMAFORA
	if (semop(semid, &sem_lock, 1)==-1){
		perror("semop");
		exit(1);
	}
		//semid, zadeklarowane wartosci, pierwsze {} w deklaracji
	
	//operacje na pamieci wspoldzielonej
	printf("Zajalem semafor id=%d\n",semid);
	printf("Pamiec wspoldzielona przed zmiana ma wartosc: %d\n", *shared_val);
	

	int i;
	
	if (stop==false){
		for(i=0; i<100000000; i++){
			//nanosleep(&ts, NULL);
			*shared_val = *shared_val + zmiana;
		}
	}
	printf("Wartosc po zmianie: %d\n", *shared_val);


	//deklaracja operacji V
	struct sembuf sem_unlock = {0, 1, 0};

	//wykonanie operacji - ODDANIE semafdora
	if(semop(semid, &sem_unlock, 1)==-1){
		perror("semop");
		exit(1);
	} 
	printf("Zwolnilem semafor id=%d\n", semid);

// KONIEC SEKCJI KRYTYCZNEJ


	//odlaczenie SHM od procesu
		//shm detach
	if(shmdt(shared_val) == -1){
		perror("shmdt");
		exit(1);
	}else {
		printf("Segment pamieci wspoldzielonej zostal odlaczony od tego procesu, ale dalej istnieje!\n");
	}

	//nie usuwam ani SHM, ani semafora - od tego jest skrypt sprzatajacy 

	return 0;

}






























	
