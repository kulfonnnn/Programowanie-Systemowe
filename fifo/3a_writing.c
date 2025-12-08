




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>  
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[]){


        // sprawdzamy, czy zostal podany argument
        if (argc != 2){
                fprintf(stderr, "Usage: %s /sciezka/fo/fifo \n", argv[0]);
                exit(1);
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
                        exit(1);
                }
        } else {
                printf("Otwarcie sie powiodlo\n");
        }


	
	char msg[4096];                  // bufor 4KB
	memset(msg, 'A', sizeof(msg));  // wypełnij znakami 'A'
		//memset zwraca msg, nie zwraca bledow
	long total = 0; // dotychczasowo przeslane bajty 

	for (int i = 0; i < 30; i++) {  // powtórz 50 razy 
		printf("Próbuję przeslac (porcja %d, total=%ld)...\n", i, total);


		if(i==10){
			sleep(10);
		}
		
		ssize_t w = write(fd, msg, sizeof(msg));
			// write zwraca -1, gdy wystapi blad
			//w przeciwnym wypadku, zwraca liczbe zapisanych bajtow
		if (w == -1) {
			perror("write");
			if (close(fd) == -1) {
				perror("close(fd)");
			}
			exit(1);
		}
		if (w != sizeof(msg)) {
 			printf("UWAGA: write zapisalo tylko %zd bajtow!\n", w);
		}



		total += w; // dodajemy przeslane bajty
	}		
		


	
        if (close(fd) == -1) {
                perror("close");
        }


        // nie usuwamy FIFO - zrobi to proces czytajacy
        printf("Program konczy dzialnie...\n");
        return 0;
}

	








