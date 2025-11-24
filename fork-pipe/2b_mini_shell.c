#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {


        if (argc != 5) { // jezeli podano inna liczbe argumentow niz 4
                fprintf(stderr, "Usage: %s prog1 arg1 prog2 arg 2\n", argv[0]); //wyprintuj instrukcje
                exit(1);
        }


        int fd[2];
        if (pipe(fd) == -1) { //wywoluje pipe, jezeli cos pojdzie nie tak
                perror("pipe"); // na stand wyjscie diagnostyczne komunikat
                exit(1); // koniec dzialania procesu
        }

        pid_t pid1 = fork();

        if (pid1 < 0) {
                perror("fork");
                exit(1);
        }

        if (pid1 == 0) {
                // Proces 1: wykonuje ls
                if (close(fd[0]) == -1) {  // zamykamy czytanie potoku, bo piszemy
                        perror("child1 close(fd[0])");
                        _exit(1);
                }
                if (dup2(fd[1], STDOUT_FILENO) == -1) {  // przekierowujemy stdout do potoku
                        perror("child1 dup2");
                        _exit(1);
                }
                if (close(fd[1]) == -1) {  // oryginalne fd[1] zamykamy
                        perror("child1 close(fd[1])");
                        _exit(1);
                }

                execlp(argv[1], argv[1], argv[2], NULL);  // uruchamiamy ls z jednym argumentem
                //jezeli execpl zadziala, to wymienia aktualny proces na ten z argumentu
                // nie w raca z powrotem - perror nie zostanie wykonany, jezeli wszystko pojdzie dobrze
                perror("execlp");
                _exit(1);

        }

        // Proces macierzysty czeka chwilę, ale nie musi — od razu tworzymy drugie dziecko
        pid_t pid2 = fork();


        if (pid2 < 0) {
                perror("fork");
                exit(1);
        }

        if (pid2 == 0) {
                // Proces 2: wykonuje sort
                if (close(fd[1]) == -1) {  // zamykamy pisanie potoku, bo czytamy
                        perror("child2 close(fd[1])");
                        _exit(1);
                }
                if (dup2(fd[0], STDIN_FILENO) == -1) {  // przekierowujemy stdin z potoku
                        perror("child2 dup2");
                        _exit(1);
                }
                if (close(fd[0]) == -1) {  // oryginalne fd[0] zamykamy
                        perror("child2 close(fd[0])");
                        _exit(1);
                }

                execlp(argv[3], argv[3], argv[4], NULL);  // uruchamiamy sort z jednym argumentem
                perror("execlp");
                _exit(1);

        }

        // Proces rodzica zamyka oba końce potoku, bo nie używa pipe
        if (close(fd[0]) == -1) {
                perror("parent close(fd[0])");
                exit(1);
        }
        if (close(fd[1]) == -1) {
                perror("parent close(fd[1])");
                exit(1);
        }

        // Czeka na oba procesy dziecka
        if (waitpid(pid1, NULL, 0) == -1) {
                perror("waitpid pid1");
                exit(1);
        }
        if (waitpid(pid2, NULL, 0) == -1) {
                perror("waitpid pid2");
                exit(1);
        }

        return 0;
}

