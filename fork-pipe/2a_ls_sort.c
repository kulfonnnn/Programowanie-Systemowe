#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];

    pipe(fd);
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        perror("fork");
    }

    if (pid1 == 0) {
        // Proces 1: wykonuje ls
        close(fd[0]);           // zamykamy czytanie potoku, bo piszemy
        dup2(fd[1], STDOUT_FILENO);  // przekierowujemy stdout do potoku
        close(fd[1]);           // oryginalne fd[1] zamykamy

        execlp("ls", "ls", NULL);  // uruchamiamy ls
    	exit(1);
    }

    // Tworzymy drugie dziecko
    pid_t pid2 = fork();

    if (pid2 == 0) {
        // Proces 2: wykonuje sort
        close(fd[1]);           // zamykamy pisanie potoku, bo czytamy
        dup2(fd[0], STDIN_FILENO);   // przekierowujemy stdin z potoku
        close(fd[0]);           // oryginalne fd[0] zamykamy

        execlp("sort", "sort", NULL);  // uruchamiamy sort
    }

    // Proces rodzica zamyka oba końce potoku, bo nie używa pipe
    close(fd[0]);
    close(fd[1]);

        // Czeka na oba procesy dziecka
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}

