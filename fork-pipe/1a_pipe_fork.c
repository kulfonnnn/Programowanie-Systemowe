#include <stdio.h>
#include <unistd.h>
#include <string.h>



int main() {
	int fd[2];
	pipe(fd); // tworzymy pipe

	pid_t pid = fork(); // tworzymy proces dziecka
	
	if (pid > 0) { // dla procesu rodzica
		close(fd[0]); // zamykamy zero, bo nie czytamy
		
		char msg[] = "Wiadomosc od procesu rodzica";	
		write(fd[1], msg, strlen(msg)); //wysylamy wiad na jeden
		
		close(fd[1]); // zamykamy jeden
	}

	else if (pid==0) { //dla dziecka
		close(fd[1]); //zamykamy jeden, bo nie wysylamy
		
		char buf[100];
		int n = read(fd[0], buf, sizeof(buf)); //czytamy wiad na zero, n to liczba odczytanych bajtow
		buf[n]= '\0'; //zeby print dzialal, dodajemy koncowke stringa 

		close(fd[0]); // zamykamy zero
		
		printf("Odczytano %s \n", buf);
	}
	
	else {
		perror("fork");
	}

	return 0;
}
