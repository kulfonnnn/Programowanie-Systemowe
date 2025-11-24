#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>



// Funkcja dzieli argv na tablicę tablic argumentów programów
// argc - liczba argumentów, argv - tablica argumentów
// *prog_count - wskaźnik gdzie zapiszemy ile znaleźliśmy programów
// Zwraca: tablica programów, gdzie każdy program to tablica argumentów zakończona NULL

char ***split_argv(int argc, char *argv[], int *prog_count) {
        int max_programs = argc / 2 + 1; // maksymalna liczba programów (oszacowanie)
        //konieczne, aby nie bylo dynamicznego rozszerzania tablicy    

        // Alokujemy miejsce na wskaźniki do tablic argumentów programów
        char ***programs = malloc(sizeof(char**) * max_programs);
        if (programs == NULL) { // jezeli nie udalo sie przydzielic miejsca -> error
                perror("malloc");
                exit(1);
        }

        int start = 1;          // indeks początku aktualnego programu w argv (pomijamy argv[0])
        int count = 0;          // licznik programów znalezionych

        // Pętla od 1 do argc włącznie (żeby obsłużyć ostatni program bez kropki)
        for (int i = 1; i <= argc; i++) {
                // Sprawdzamy czy doszliśmy do końca argumentów lub do separatora "."
                if (i == argc || strcmp(argv[i], ".") == 0) {
                        int length = i - start; // długość segmentu (ilość argumentów programu)

                        // Tworzymy nową tablicę argumentów dla programu (+1 na NULL na końcu)
                        char **args = malloc(sizeof(char*) * (length + 1));
                        if (args == NULL) {
                                perror("malloc");
                                exit(1);
                        }

                        // Kopiujemy wskaźniki do argumentów z argv[start..i-1]
                        for (int j = 0; j < length; j++) {
                                args[j] = argv[start + j];
                        }
                        args[length] = NULL; // kończymy listę argumentów NULLem

                        // Zapisujemy wskaźnik do tablicy argumentów do tablicy programów
                        programs[count++] = args;

                        start = i + 1; // ustawiamy start na następny program po kropce
                }
        }

        // Przekazujemy liczbę programów przez wskaźnik
        *prog_count = count;

        return programs; // zwracamy tablicę programów (każdy to tablica argumentów)
}



int main(int argc, char *argv[]) {

        if (argc < 3) { // jezeli podano mniej argumentow niz 2
                fprintf(stderr, "Usage: %s prog1 arg1 arg2 . prog2 arg1 . prog3\n", argv[0]); //wyprintuj instrukcje
                fprintf(stderr, "Separator .  /works for any prog / arg count\n");
                exit(1);
        }

        int prog_count;
        char ***programs = split_argv(argc, argv, &prog_count);
        //*** oznacza ze to tablica tablic
        //& daje funkcji adres od prog_count, wiec moze ja edytowac

        //wyprintowanie programow wraz z ich argumentami
        for (int i = 0; i < prog_count; i++) {
                printf("Program %d arguments:\n", i + 1);

                // programs[i] to tablica argumentów jednego programu, zakończona NULL
                for (int j = 0; programs[i][j] != NULL; j++) {
                        printf("  argv[%d] = %s\n", j, programs[i][j]);
                }
        }

        int pipes[prog_count - 1][2]; //tablica dwuwymiarowa na polaczenia miedzy procesami

        // dla kazdego polaczenia tworzymy pipe
        for (int i = 0; i < prog_count - 1; i++) {
                if (pipe(pipes[i]) == -1) { //pipes[i] -> jedno polaczenie, na kazdym 0 i 1
                        perror("pipe");
                        exit(1);
                }
        }

        //glowna petla
        for (int i = 0; i < prog_count; i++) {

                pid_t pid = fork();

                if (pid < 0) {
                        perror("fork");
                        exit(1);
                }

                if (pid == 0) {
                        //dla procesow dzieci

                        // jezeli to nie pierwsza iteracja (pierwszy program)
                        if (i > 0) { // to ustawiamy czytanie z pipe zamiast ze stdin
                                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) { //czytamy z poprzedniego
                                        perror("dup2 stdin");
                                        exit(1);
                                } // if daje obsluge bledow, dup2 bez ifa odpaliloby sie tak samo
                        }

                        // jezeli to nie ostatnia iteracja (ostatni program)
                        if (i < prog_count - 1) { // to ustawiamy wysylanie na pipe zamiast stdout
                                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                                        perror("dup2 stdout");
                                        exit(1);
                                }
                        }

                        // zamykamy wszystkie konce potokow
                        for (int j = 0; j < prog_count - 1; j++) {
                                if (close(pipes[j][0]) == -1) {
                                        perror("child close pipe read end");
                                        exit(1);
                                }
                                if (close(pipes[j][1]) == -1) {
                                        perror("child close pipe write end");
                                        exit(1);
                                }
                        }

                        execvp(programs[i][0], programs[i]);
                        //jezeli execvp zadziala, to wymienia aktualny proces na ten z argumentu
                        // nie w raca z powrotem - perror niezostanie wykonany, jezeli wszystko pojdzie dobrze
                        perror("execvp");
                        exit(1);
                }


        }

        // Rodzic zamyka wszystkie końce potoków, bo ich nie używa
        for (int i = 0; i < prog_count - 1; i++) {
                if (close(pipes[i][0]) == -1) {
                        perror("parent close pipe read end");
                        exit(1);
                }
                if (close(pipes[i][1]) == -1) {
                        perror("parent close pipe write end");
                        exit(1);
                }
        }

        // I czeka na wszystkie procesy dzieci
        for (int i = 0; i < prog_count; i++) {
                if (wait(NULL) == -1) { //czeka na proces potomny, NULL ignoruje wynik (PID dziecka)
                        perror("wait");
                        exit(1);
                }
        }
        printf("\n Program konczy sie...\n");

        return 0;
}
