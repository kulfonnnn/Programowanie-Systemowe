

#include <sys/stat.h>
#include <sys/types.h>
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

	

        //otwieramy fifo (read only)
        printf("Otwieram fifo....\n");
        int fd = open(argv[1], O_RDONLY);

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

	sleep(25);

	char buf[4096];
	int n;
	while (1){
		
		n = read(fd, buf, sizeof(buf));
			//czytamy wiad z deskryptora
			// jezeli n jest >0, to wszystko jest ok - liczba odczytanych bajtow
		if (n>0){
			printf("Odczytano %d bajtow\n", n);
		}	

		else if (n == -1) { // dla bledu read
			perror("read");
			if (close(fd) == -1) {
				perror("close");
			}
			exit(1);
		}

		else if (n == 0) {
			// To NIE jest błąd — po prostu EOF
			// EOF = End Of File
			printf("nic nie otrzymano (EOF)\n");
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

        printf("Program konczy dzialanie.\n");
        return 0;
}
	









