#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>


int main() {
        int fd[2];

        //wywolujemy pipe, zwraca 0 = sukces, zwraca -1 = blad
        if (pipe(fd) == -1) {
                perror("pipe");
                return 1;
        }

        pid_t pid = fork(); // tworzymy proces dziecka


        if (pid < 0) { // jezeli wystapi blad, to fork() zwraca wartosci <0, najczesciej -1 
                perror("fork");
                exit(1);
        }

        if (pid > 0) { // dla procesu rodzica


                if (close(fd[0]) == -1 ) {
                        perror("rodzic close(fd[0])");
                        exit(1);
                }


                char msg[] = "Wiadomosc od procesu rodzica";

                ssize_t w = write(fd[1], msg, strlen(msg));
                // write zwraca -1, gdy wystapi blad
                //w przeciwnym wypadku, zwraca liczbe zapisanych bajtow
                if (w == -1) {
                        perror("write");
                        if (close(fd[1]) == -1){
                                perror("rodzic close(fd[1])");
                        }
                        exit(1);

                    
                }


                if (close(fd[1]) == -1) {
                        perror("rodzic close(fd[1])");
                        exit(1);
                }


                //rodzic czeka, az dzieci sie wykonaja
                // ostatnie pole to pole options - 0 oznacza, ze czekanie jest blokujace
                // nie zwracamy od razu, czekamy 
                // drugie pole to status, pozwala na wypisywanie zaawansowanych informacji
                // null wylacza te opcje
                if (waitpid(pid, NULL, 0) == -1){
                        //jezeli waitpid zwraca -1, wystapil blad
                        // jezeli nie ma problemu, zwraca pid dziecka
                        perror("waitpid");
                        exit(1);
                }
                printf("komunikacja zakonczona\n");
        }




        if (pid==0) { //dla dziecka

                // zamykam deskryptor do wysylania (1), bo dziecko go tutaj nie uzywa
                if (close(fd[1]) == -1) {
                        perror("child close(fd[1])");
                        _exit(1);
                        // _ powoduje, ze proces NATYCHMIASTOWO znika
                        // zalecane do stosowania w procesie dziecka
                        // dziecko wyplukuje odziedziczonego buforu po rodzicu, nie wypisze podwojnych komunikatow
                }


                char buf[100];


                ssize_t n  = read(fd[0], buf, sizeof(buf) - 1); //czytamy wiad z deskryptora 0
                // jezeli n jest >0, to wszystko jest ok - liczba odczytanych bajtow
                // -1, poniewaz zostawiamy jedno miejsce na pozniejsze dopisanie

                if (n == -1) {
                // read zwraca -1, jezeli wystapil blad
                        perror("child read");
                        // Zanim wyjdziemy, próbujemy zamknąć fd[0]
                        if (close(fd[0]) == -1) {
                                perror("child close fd[0]");
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



                buf[n]= '\0'; // upewniamy sie, ze bufor jest zakonczony '\0'
                //w ten sposob koncza sie stringi

                if (close(fd[0]) == -1) {
                         // zamykamy zero
                        // jezeli sie nie powiedzie:
                        perror("child close fd[0]");
                        _exit(1);
                }
                printf("Dziecko: odczytalem  %s \n", buf);

                _exit(0); 
                // dziecko wykonalo swoja robote, mozna go zamkac
        }


        return 0;
}
