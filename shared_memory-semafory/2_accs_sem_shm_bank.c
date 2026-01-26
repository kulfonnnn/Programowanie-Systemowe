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




//funkcja pobierajaca losowa liczbe nanosekund w przedziale 0-999999999 (0-1s) zmienilem na 10s dla testow!
long random_nsec() {
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1){perror("open"); exit(1);}
	
	unsigned int liczba; // unsigned int = 32-bitowa liczba bez znaku
	
	ssize_t n = read(fd, &liczba, sizeof(liczba));
	if (n != sizeof(liczba)){
		perror("read");
		if(close(fd)==-1){perror("close"); exit(1);}
		exit(1);
	}
	
	if(close(fd)==-1){perror("close"); exit(1);}
	
	// r może być 0 .. 2^32-1 (~4,29 mld)
    // chcemy nanosekundy w zakresie 0 .. max
	long long max = 10000000000; // realnie max nie moze byc wiekszy niz 4,29MLD, nic nie ogranicza
	long long ns = liczba % max;

	return ns;
}

	// funkcja deklarujaca i wywolujaca nanosleepa
void losowy_sleep(void){
	long long ns = random_nsec(); // wywolanie funkcji	
	struct timespec ts;
		ts.tv_sec  = ns / 1000000000;   // całe sekundy
		ts.tv_nsec = ns % 1000000000;   // nanosekundy < 1 s	
	//wywolanie nanosleep
	if(nanosleep(&ts, NULL) == -1){perror("nanosleep"); exit(1);}

}




//funkcja odlaczajaca SHM z tego procesu przy zakonczoniu programu
		//semafory i SHM dalej istnieja, inne wywolania progamu moga ich uzyc
		//w tym samym folderze jest program do ich usumiecia
struct Konta {int konto1;int konto2;};
struct Konta *konta = NULL;
void cleanup(void) {
	if(shmdt(konta) == -1) {
		perror("shmdt");
	} else {
		printf("Segment pamieci wspoldzielonej odlaczony\n");
	}
}


int main(int argc, char *argv[]) {

	

	if (argc != 4){
		fprintf(stderr, "Bledna ilosc argumentow.\nUzycie: %s <nr operacji> <nr_konta> <wartosc_zmiany> (1=wplata, 2=wyplata, 3=przelew do)", argv[0]);
		exit(1);	
	} else {
		printf("Poprawnie zainterpretowano wartosci nr operacji: %s, nr konta: %s, wartosc zmiany: %s\n", argv[1], argv[2], argv[3]);
	}
	
	//zamiana argumentu na int
	int nr_op = atoi(argv[1]);
	int nr_konta = atoi(argv[2]);
	int zmiana = atoi(argv[3]); // jezeli wartosc zmiany jest nieprawidlowa - program zmieni ja na 0 i sie wykona
	if (nr_op == 0 || nr_konta == 0){
		fprintf(stderr, "Blad: Wartosc argumentu jest bledna\n");
		exit(1);
	}


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


//SPANIE 1 
	losowy_sleep();


//SEMAFORY
		
	int semid = semget(klucz, 2, 0600 | IPC_CREAT | IPC_EXCL);
	//przy tych flagach, jezeli semafor(y) istnieje, to zwraca -1 errno EEXIST
	if (semid == -1){
		if (errno==EEXIST){
			//nasz zestaw semaforow o tym id juz istnieje - ale nie wiemy ile ma semagorow
			semid = semget(klucz, 0, 0600); //podpinamy sie pod zestaw (nic nie tworzymy/weryfikujemy)
			if(semid==-1){perror("semget");exit(1);}
			
			//teraz sprawdzamy ile ten zestaw ma semaforow
			struct semid_ds info;
			if(semctl(semid, 0, IPC_STAT, &info)==-1){perror("semctl"); exit(1);}
				//IPC_STAT -> wypelnia nasza strukture informacjami o tym zestawie
			if(info.sem_nsems != 2) {
				//jezeli pole numer semaforow ma inna wartosc niz 2
				fprintf(stderr, "Blad: zestaw semaforow juz istnieje, ale zamiast dwoch ma %lu semaforow\n", info.sem_nsems);
				exit(1);
			}
			//jezeli istnieje zestaw z 2 semaforami - prawdopodobnie nasz program go stworzyl, mozna uzywac
			printf("Zestaw z dwoma semaforami juz istnieje, uzywam go (semid=%d)\n",semid);			
 			}
	}else {

		//Where multiple peers do not know who will be the first to initialize the set, checking for a nonzero sem_otime in the associated data structure retrieved by a semctl(2) IPC_STAT operation can be used to avoid races.
		// sem_otime  Time of last semop(2) system call.
		

		struct semid_ds info;
		if (semctl(semid, 0, IPC_STAT, &info) == -1) {perror("semctl"); exit(1);}
			//IPC_STAT wypelnia nasza strukture informacjami o tym zestawie

		// jeśli sem_otime == 0, nikt jeszcze nie ustawił wartości semafora
		if (info.sem_otime == 0) {
			if(semctl(semid, 0, SETVAL, 1)==-1){perror("semctl");exit(1);}
			if(semctl(semid, 1, SETVAL, 1)==-1){perror("semctl");exit(1);}
			printf("Zestaw semaforow id=%d jest gotowy do uzycia przez pierwszy zainteresowany proces!\n", semid);
		} else {
			// ktoś inny już ustawił wartości, nic nie robimy
			printf("Zestaw semaforow teoretycznie nie istnieje, ale praktycznie ktoś wygrał wyścig i id=%d ma juz zmienione wartosci\n", semid);
		}
	} 
	


//SHARED MEMORY
	
	//tworzymy zmienna, ktora intepretuje przydzielone bajty jako 2 inty
	//struct Konta {int konto1; int konto2;}; (mamy to globalnie!)

	//tworzymy segment pamieci
	int shmid = shmget(klucz, sizeof(struct Konta), 0600 | IPC_CREAT);
			// IPC_CREAT - jezeli nie istnieje, to go utworz
			// jezeli istnieje - ok, uzywam go i zwracam id	
	if (shmid == -1) {
		perror("shmget");
		exit(1);
	} else {
		printf("Segment o id=%d w pamieci istnieje - istnial, lub zostal utworzony\n", shmid);
	}
	
	//mapujemy adres segmentu do przestrzeni adresowej naszego procesu
		//shmat -> shm attachi
	konta = shmat(shmid, NULL, 0);
	if (konta == (void*) -1){
		perror("shmat");
		exit(1);
	} else {
		printf("Wskaznik na adres segmentu zostal pomyslnie dodany do naszego procesu\n");
	}
		//wskaznika int* nie mozemy powrownac ze zwyklym int
		//wiec void rzutuje -1 na typ pod wskaznikiem		
	
	
	atexit(cleanup);




//SEKCJE KRYTYCZNE

	//OPERACJE P/V
		//{nr semaforu, zmiana wartosci (zajmij/odblokuj), flagi (domyslnie=blokujaco)}
	// konto 1
	struct sembuf P_konto1 = {0, -1, 0}; // semafor 0
	struct sembuf V_konto1 = {0,  1, 0};
	//konto 2
	struct sembuf P_konto2 = {1, -1, 0}; // semafor 1
	struct sembuf V_konto2 = {1,  1, 0};
	// przelew
	struct sembuf P_przelew[2] = {
    	{0, -1, 0}, // blokuj konto 1
    	{1, -1, 0}  // blokuj konto 2
	};
	struct sembuf V_przelew[2] = {
		{0, 1, 0},  // odblokuj konto 1
		{1, 1, 0}   // odblokuj konto 2
	};	

//KONTO 1 (SEMAFOR 0)

	if (nr_konta == 1 && nr_op != 3){

		printf("Probuje zajac semafor nr 0...\n");
		if(semop(semid, &P_konto1, 1)==-1){perror("semop"); exit(1);}
			//(id, struct operacji, ilosc {} do uzycia)
		printf("Zajalem semafor nr 0!\n");
		
		losowy_sleep();

		printf("Saldo konta 1 przed zmiana = %d\n", konta->konto1);

		if(nr_op == 1){
			konta->konto1 += zmiana;
		} else if (nr_op == 2){
			konta->konto1 -= zmiana;
		}
		

		printf("Saldo konta 1 po zmianach = %d\n", konta->konto1);

		if(semop(semid, &V_konto1, 1) == -1){ perror("semop"); exit(1);}
		printf("Zwolnilem semafor nr 0\n");

	}	

//KONTO 2 (SEMAFOR 1)

	if (nr_konta == 2 && nr_op != 3){

		printf("Probuje zajac semafor nr 1...\n");
		if(semop(semid, &P_konto2, 1)==-1){perror("semop"); exit(1);}
			//(id, struct operacji, ilosc {} do uzycia)
		printf("Zajalem semafor nr 1!\n");
		
		losowy_sleep();

		printf("Saldo konta 2 przed zmiana = %d\n", konta->konto2);

		if(nr_op == 1){
			konta->konto2 += zmiana;
		} else if (nr_op == 2){
			konta->konto2 -= zmiana;
		}


		printf("Saldo konta 2 po zmianach = %d\n", konta->konto2);

		if(semop(semid, &V_konto2, 1) == -1){ perror("semop"); exit(1);}
		printf("Zwolnilem semafor nr 1\n");

	}	

//PRZELEW (OBA SEMAFOR)

	if (nr_op == 3){

		printf("Probuje zajac oba semafory...\n");
		if(semop(semid, P_przelew, 2)==-1){perror("semop"); exit(1);}
			//(id, struct operacji, ilosc {} do uzycia)
		printf("Zajalem oba semafory!\n");
		losowy_sleep();

		if (nr_konta == 1){
			printf("Saldo konta 1 przed zmiana = %d\n", konta->konto1);

			konta->konto1 -= zmiana;
			konta->konto2 += zmiana;

			printf("Saldo konta 1 po zmianach = %d\n", konta->konto1);

		}else if (nr_konta == 2){
			printf("Saldo konta 2 przed zmiana = %d\n", konta->konto2);

			konta->konto2 -= zmiana;
			konta->konto1 += zmiana;

			printf("Saldo konta 2 po zmianach = %d\n", konta->konto2);
		}

		if(semop(semid, V_przelew, 2) == -1){ perror("semop"); exit(1);}
		printf("Zwolnilem oba semafory\n");
	}
		

// KONIEC SEKCJI KRYTYCZNYCH

	return 0;
}






























	
