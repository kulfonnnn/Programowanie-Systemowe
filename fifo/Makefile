all: 1a 1b 2a 2b 3a 3b mplayer


# $@ = nazwa celu, $< = pierwszy plik zalezny, $^ = wszystkie pliki zalezne
CC = gcc
CFLAGS = -Wall

#-----------
1a: 1a_spam_sending.o
	$(CC) $(CFLAGS) -o $@ $^

# $< - pierwsza zaleznosc (tutaj: plik .c)
1a_spam_sending.o: 1a_spam_sending.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
1b: 1b_slow_reading.o
	$(CC) $(CFLAGS) -o $@ $^

1b_slow_reading.o: 1b_slow_reading.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
2a: 2_only_writing.o
	$(CC) $(CFLAGS) -o $@ $^

2_only_writing.o: 2_only_writing.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
2b: 2b_only_writing.o
	$(CC) $(CFLAGS) -o $@ $^

2b_only_writing.o: 2b_only_writing.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
3a: 3a_writing.o
	$(CC) $(CFLAGS) -o $@ $^

3a_writing.o: 3a_writing.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
3b: 3b_reading.o
	$(CC) $(CFLAGS) -o $@ $^

3b_reading.o: 3b_reading.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
mplayer: mplayer_fifo.o
	$(CC) $(CFLAGS) -o $@ $^

mplayer_fifo.o: mplayer_fifo.c
	$(CC) $(CFLAGS) -c -o $@ $<

#-----------
clean:
	rm -f *.o 1a 1b 2a 2b 3a 3b mplayer
#dzieki fladze f nie dostaniemy bledow, nawet jezeli pliki nie istnieja itp
