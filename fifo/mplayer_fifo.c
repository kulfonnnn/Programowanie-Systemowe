#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>  
#include <string.h>
#include <errno.h>
#include <signal.h>

int main(int argc, char *argv[]){

	signal(SIGPIPE, SIG_IGN);
	//jezeli mplayer sie zakonczy w trakcie dzialania programu, a my wyslemy write
	// otrzymamy sigpipe ktory zamknie nasz program bez obslugi bledow
	// ignorujemy go	


	// sprawdzamy, czy zostal podany argument
	if (argc != 2){
		fprintf(stderr, "Usage: %s /sciezka/fo/fifo \n", argv[0]);
		exit(1);
	}

	

        // próbujemy stworzyć FIFO
        if (mkfifo(argv[1], 0666) == -1) {
        	if (errno == EEXIST) {
                	// jezeli fifo już istnieje, korzystamy dalej
                	printf("FIFO %s już istnieje – uzywam go.\n", argv[1]);
        	} else {
                // inny błąd
                	perror("mkfifo");
               	 	exit(1);
            }
        } else {
        	printf("FIFO %s utworzone.\n", argv[1]);
        }



	//otwieramy fifo (wirte only)
	printf("Otwieram fifo....\n");
	int fd = open(argv[1], O_WRONLY);
	
	//jezeli wystapil blad, open zwraca -1 i errno
	if (fd == -1) {
		// czytelne komunikaty dla najpopularniejszych errno
		if (errno == ENOENT){
			fprintf(stderr, "Plik fifo %s nie istnieje\n", argv[1]);
			exit(1);	
		}else {
			perror("open");
			
			// usuwamy FIFO
			if (unlink(argv[1]) == -1) {
				perror("unlink");
				exit(1);
			} else {
				printf("FIFO %s usuniete\n", argv[1]);
			}
			exit(1);
		}
	} else {
		printf("Otwarcie sie powiodlo\n");
	}




	char command[16];
	char time[16];
	bool quitting = false;


	printf("Panel sterowania rozpoczyna dzialanie. Wpisuj polecenia zgodnie z instrukcja\n");
	printf("p = pauza/wznow, s = przewin/cofnij, m = wycisz, q = quit (mplayer+program), l = leave (program)\n");
	while(true){
	
		// czyscimy zmienna time 
		strcpy(time, "");
		//command nie musimy, bo ona jest zawsze nadpisywana

	
		printf("Podaj litere\n");
		
		//tworzymy bufor do wpisania komendy
		char buf[3];

		//wczytanie litery od uzytkownika
		if (fgets(buf, sizeof(buf), stdin) == NULL){
			// fgets zwraca null w przypadku bledu
			perror("fgets");
			if(close(fd)==-1){
				perror("close");
			}
			// usuwamy FIFO
			if (unlink(argv[1]) == -1) {
				perror("unlink");
				exit(1);
			} else {
				printf("FIFO %s usuniete\n", argv[1]);
			}
			
			exit(1);
		
	        //buf musi mieć postać "X\n"
	        } else if (buf[0] == '\n') {
			printf("Nie wpisano żadnej litery, spróbuj ponownie.\n");
			continue;

	        // jezeli drugi znak to nie '\n', to znaczy że wpisano więcej niż 1 znak
	        } else if (buf[1] != '\n') {
			printf("Wpisz tylko jedną literę!\n");

			//musimy wyczyscic stdin - w przeciwnym wypadku
			//nastepna iteracja odczyta jego aktualna zawarosc
			int c;
			while ((c = getchar()) != '\n' && c != EOF);
				//dopoki getchar nie zwroci konca lini / bledu
				//odczytujemy znaki z stdin
			continue;
	    		
		} else{
		
			//buf moze zawierac rowniez znaki konca lini lub stringa
			//dlatego porownujemy pierwsze pole [0]
			
			if (buf[0] == 'p'){
				strcpy(command, "pause");
			} else if(buf[0] == 'm'){
				strcpy(command, "mute");
			} else if(buf[0] == 'q'){
				strcpy(command, "quit");
				//nie dajemy break tutaj, bo nie zostanie wyslany sygnal quit
				quitting = true;
			} else if(buf[0] == 'l'){
				break;	
	
			} else if(buf[0] == 's'){
				strcpy(command, "seek ");
				printf("Podaj, o ile sekund chcesz przewinac. Ujemna liczba = w tyl\n");
				//wczytanie liczby i obsluga bledu
				if(fgets(time, sizeof(time), stdin) == NULL){
					perror("fgets");
					if(close(fd)==-1){
						perror("close");
					}
					// usuwamy FIFO
					if (unlink(argv[1]) == -1) {
						perror("unlink");
						exit(1);
					} else {
						printf("FIFO %s usuniete\n", argv[1]);
					}
					exit(1);

				} else if (time[0] == '\n'){
					printf("Nic nie podales, spróbuj ponownie\n");
				
				} else if (time[strlen(time)-1] != '\n') {
    					//jezeli na drugim od koncu miejscu w time nie ma \n
					//to znaczy, ze wiadomosc>buifor
					//czyscimy stdin dla nastepnej iteracji
    					int c; while ((c = getchar()) != '\n' && c != EOF);
    					printf("Za długa liczba, spróbuj ponownie.\n");
    					continue;
				}
			
			} else {
				printf("Wpisales nieparawidlowa komende!\n");
				continue;
			}
		}
		
		//tworzymy komende do wyslania do fifo
		char full_command[50];
		
		//jezeli nasza komenda nie zawiera czasu
		if (time[0] == '\0'){
			//wysylamy command + znak nowej lini
			int n = snprintf(full_command, sizeof(full_command), "%s\n", command);
			// obsluga bledow
			if (n < 0) {
 				perror("snprintf error");
			} else if (n >= sizeof(full_command)) {
				//n zwraca ilosc bajtow, ktore chcemy zapisac
				//jezeli chcemy zapisac > bufor, to n>bufor
    				fprintf(stderr, "Bufor za mały, string został obcięty!\n");
			}
		} else {
			//wysylamy command + time + znak nowej lini
			int n = snprintf(full_command, sizeof(full_command), "%s%s\n", command, time);
			// obsluga bledow
			if (n < 0) {
 				perror("snprintf error");
			} else if (n >= sizeof(full_command)) {
    				fprintf(stderr, "Bufor za mały, string został obcięty!\n");
			}

		}

		//wysylamy komende do fifo
		if (write(fd, full_command, strlen(full_command)) == -1) {
			perror("write");
			if(close(fd) == -1){
				perror("close");
			}

			// usuwamy FIFO
			if (unlink(argv[1]) == -1) {
				perror("unlink");
				exit(1);
			} else {
				printf("FIFO %s usuniete\n", argv[1]);
			}
			 
			exit(1);
		}
		if(quitting == true){
			break;
		}


		
	}

	if (close(fd) == -1) {
		perror("close");
	}


	// usuwamy FIFO
	if (unlink(argv[1]) == -1) {
		perror("unlink");
		exit(1);
	} else {
    		printf("FIFO %s usuniete\n", argv[1]);
	}

	printf("Frontend zakonczyl dzialanie.\n");
	return 0;
}

				






