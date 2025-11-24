#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
        int fd[2];

        if (pipe(fd) == -1) {
                perror("pipe");
                exit(1);
        }

        pid_t pid1 = fork();
        if (pid1 < 0) {
                perror("fork");
                exit(1);
        }

        if (pid1 == 0) {
                // Proces 1: wykonuje ls
                if (close(fd[0]) == -1) {  // zamykamy deskryptor do czytania
                        perror("child1 close(fd[0])");
                        _exit(1);
                }

                if (dup2(fd[1], STDOUT_FILENO) == -1) { // wymieniamy standardowe wyjscie na zapis do pipe
                        perror("child1 dup2");
                        _exit(1);
                }

                if (close(fd[1]) == -1) { // zamykamy deskryptor do pisania
                        perror("child1 close(fd[1])");
                        _exit(1);
                }

                execlp("ls", "ls", "-l", NULL); // odpalamy program ls
                // execlp WYMIENIA aktualny program na ls, wiec pozniejsze komendy sie nie wykonuja, jezeli dziala
                perror("execlp ls");
                _exit(1); 
        }

        pid_t pid2 = fork();
        if (pid2 < 0) {
                perror("fork");
                exit(1);
        }

        if (pid2 == 0) {
                // Proces 2: wykonuje sort
                if (close(fd[1]) == -1) { // zamykam deskryptor do pisania
                        perror("child2 close(fd[1])");
                        _exit(1);
                }

                if (dup2(fd[0], STDIN_FILENO) == -1) { // wymieniamy standardowe wejscie na czytanie z pipe
                        perror("child2 dup2");
                        _exit(1);
                }

                if (close(fd[0]) == -1) { // zamykamy deskryptor do czytania 
                        perror("child2 close(fd[0])");
                        _exit(1);
                } 

                execlp("sort", "sort", NULL);
                //wymieniamy obecny program na sort
                perror("execlp sort");
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

        printf("Komunikacja zakonczona\n");
        return 0;
}

