

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
	
	char msg[] = "Wiadomosc";
	

	int w = write(fd, msg, sizeof(msg));
	
	if (w == -1) {
		perror("write");
		if (close(fd) == -1) {
			perror("close(fd)");
		}
		exit(1);
	}

        if (close(fd) == -1) {
                perror("close");
        }

	
	printf("Halo, halo, czy ktos mnie zobaczy?");
        

	printf("Program konczy dzialnie...\n");


	return 0;
}


