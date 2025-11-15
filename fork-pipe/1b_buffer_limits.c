#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>


// drugi terminal: sudo strace -p <pid>

void ts() { //funkcja timestamp, żeby było widać moment blokady
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("[%02d:%02d:%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
}


int main() {

	
	int fd[2];
	pipe(fd); // tworzymy pipe

	pid_t pid = fork(); // tworzymy proces dziecka
	
	printf("PID: %d  PPID: %d\n", getpid(), getppid()); // wypisanie pid i parent pid

	if (pid > 0) { // dla procesu rodzica
		close(fd[0]); // zamykamy zero, bo nie czytamy
		
		char msg[4096];                  // bufor 4KB
		memset(msg, 'A', sizeof(msg));  // wypełnij znakami 'A'
		long total = 0; // dotychczasowo przeslane bajty 


		for (int i = 0; i < 50; i++) {  // powtórz 50 razy 
			sleep(1);    
			ts(); // ← (5) log PRZED write (pozwala zobaczyć kiedy proces się blokuje)
            		printf("Rodzic: próbuję przeslac (porcja %d, total=%ld)...\n", i, total);
			

			ssize_t w = write(fd[1], msg, sizeof(msg));
			total += w; // dodajemy przeslane bajty
			 
	}


		close(fd[1]); // zamykamy jeden
	}

	else if (pid==0) { //dla dziecka
		close(fd[1]); //zamykamy jeden, bo nie wysylamy
		ts();
		printf("Dziecko idzie spac! \n");
		sleep(25);


		char buf[4096];
		int n;
		while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
			ts();
			printf("Dziecko: odczytano %d bajtow\n", n);
		}


		close(fd[0]); // zamykamy zero
	}
	
	else {
		perror("fork");
	}

	return 0;
}
