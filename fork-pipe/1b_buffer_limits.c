#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>  // dodane dla waitpid()

void ts() { //funkcja timestamp, żeby było widać moment blokady
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        printf("[%02d:%02d:%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

int main() {

        int fd[2];
        if (pipe(fd) == -1) {   // obsługa błędu pipe
                perror("pipe");
                exit(1);
        }

        pid_t pid = fork(); // tworzymy proces dziecka
        if (pid < 0) {        // obsługa błędu fork
                perror("fork");
                exit(1);
        }

        printf("PID: %d  PPID: %d\n", getpid(), getppid()); // wypisanie pid i parent pid

        if (pid > 0) { // dla procesu rodzica
                if (close(fd[0]) == -1) {   // zamykamy deskryptor do czytania
                        perror("rodzic close(fd[0])");
                        exit(1);
                }

                char msg[4096];                  // bufor 4KB
                memset(msg, 'A', sizeof(msg));  // wypełnij znakami 'A'
                long total = 0; // dotychczasowo przeslane bajty 

                for (int i = 0; i < 30; i++) {  // powtórz 50 razy 
                        sleep(1);    
                        ts(); // log PRZED write (pozwala zobaczyć kiedy proces się blokuje)
                        printf("Rodzic: próbuję przeslac (porcja %d, total=%ld)...\n", i, total);

                        ssize_t w = write(fd[1], msg, sizeof(msg));
						// write zwraca -1, gdy wystapi blad
                		//w przeciwnym wypadku, zwraca liczbe zapisanych bajtow
                        if (w == -1) {
                                perror("rodzic write");
                                if (close(fd[1]) == -1) {
                                        perror("rodzic close(fd[1])");
                                }
                                exit(1);
                        }
                        total += w; // dodajemy przeslane bajty
                }

                if (close(fd[1]) == -1) { // zamykamy deskryptor do pisania
                        perror("rodzic close(fd[1])");
                        exit(1);
                }

                
                //rodzic czeka, az dziecko sie wykona
                 // ostatnie pole to pole options - 0 oznacza, ze czekanie jest blokujace
                 // nie zwracamy od razu, czekamy
                 // drugie pole to status, pozwala na wypisywanie zaawansowanych informacji
                 // null wylacza te opcje
                if (waitpid(pid, NULL, 0) == -1) {
                        perror("waitpid");
                        exit(1);
                }
                printf("komunikacja zakonczona\n");
        }

        else if (pid == 0) { //dla dziecka
                if (close(fd[1]) == -1) {  // zamkniecie deskryptora do pisanai
                        perror("child close(fd[1])");
                        _exit(1);
						// _ powoduje, ze proces NATYCHMIASTOWO znika
                        // zalecane do stosowania w procesie dziecka
                        // dziecko wyplukuje odziedziczonego buforu po rodzicu, nie wypisze podwojnych komunikatow


                }
                ts();
                printf("Dziecko idzie spac! \n");
                sleep(25);

                char buf[4096];
                int n;
                while ((n = read(fd[0], buf, sizeof(buf))) > 0) {
						//czytamy wiad z deskryptora 0
                		// jezeli n jest >0, to wszystko jest ok - liczba odczytanych bajtow
             
                        ts();
                        printf("Dziecko: odczytano %d bajtow\n", n);
                }

                if (n == -1) { // dla bledu read
                        perror("child read");
                        if (close(fd[0]) == -1) {
                                perror("child close(fd[0])");
                        }
                        _exit(1);
                }

				else if (n == 0) {
                        // To NIE jest błąd — po prostu EOF
                        // EOF = End Of File
                        printf("child: nic nie otrzymano (EOF)\n");
                        if (close(fd[0]) == -1) {
                                perror("child close fd[0]");
                        }
                        _exit(0);
                }


                if (close(fd[0]) == -1) { //zamkniecie deskryptora do czytania
                        perror("child close(fd[0])");
                        _exit(1);
                }
                _exit(0);
        }

        return 0;
}

