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
		close(fd[0]);           // zamykamy czytanie potoku, bo piszemy
		dup2(fd[1], STDOUT_FILENO);  // przekierowujemy stdout do potoku
		close(fd[1]);           // oryginalne fd[1] zamykamy

		execlp(argv[1], argv[1], argv[2], NULL);  // uruchamiamy ls z jednym argumentem
		//jezeli execpl zadziala, to wymienia aktualny proces na ten z argumentu
		// nie w raca z powrotem - perror niezostanie wykonany, jezeli wszystko pojdzie dobrze
		perror("execlp");	
		exit(1);

	}

	// Proces macierzysty czeka chwilę, ale nie musi — od razu tworzymy drugie dziecko
	pid_t pid2 = fork();
	
	
	if (pid2 < 0) {
		perror("fork");
		exit(1);
	}	
	
	if (pid2 == 0) {
		// Proces 2: wykonuje sort
			close(fd[1]);           // zamykamy pisanie potoku, bo czytamy
		dup2(fd[0], STDIN_FILENO);   // przekierowujemy stdin z potoku
		close(fd[0]);           // oryginalne fd[0] zamykamy

		execlp(argv[3], argv[3], argv[4], NULL);  // uruchamiamy sort z jednym argumentem
		perror("execlp");
		exit(1);

	}

	// Proces rodzica zamyka oba końce potoku, bo nie używa pipe
	close(fd[0]);
	close(fd[1]);

	// Czeka na oba procesy dziecka
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);

	return 0;
}

