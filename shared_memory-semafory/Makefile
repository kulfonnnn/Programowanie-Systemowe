# Makefile dla plikow w katalogu
CC = gcc
CFLAGS = -Wall

# cele all
all: shared_memory_bank_acc 2_accs_sem_shm_bank rm_shm_sem sem_spam_shm_bank spam_shm_bank_acc

#---------------------------------
2_accs_sem_shm_bank: 2_accs_sem_shm_bank.o
	$(CC) $(CFLAGS) -o $@ $^

2_accs_sem_shm_bank.o: 2_accs_sem_shm_bank.c
	$(CC) $(CFLAGS) -c -o $@ $<

#---------------------------------
rm_shm_sem: rm_shm_sem.o
	$(CC) $(CFLAGS) -o $@ $^

rm_shm_sem.o: rm_shm_sem.c
	$(CC) $(CFLAGS) -c -o $@ $<

#---------------------------------
sem_spam_shm_bank: sem_spam_shm_bank.o
	$(CC) $(CFLAGS) -o $@ $^

sem_spam_shm_bank.o: sem_spam_shm_bank.c
	$(CC) $(CFLAGS) -c -o $@ $<

#---------------------------------
spam_shm_bank_acc: spam_shm_bank_acc.o
	$(CC) $(CFLAGS) -o $@ $^

spam_shm_bank_acc.o: spam_shm_bank_acc.c
	$(CC) $(CFLAGS) -c -o $@ $<

#---------------------------------


#---------------------------------
shared_memory_bank_acc: shared_memory_bank_acc.o
	$(CC) $(CFLAGS) -o $@ $^

shared_memory_bank_acc.o: shared_memory_bank_acc.c
	$(CC) $(CFLAGS) -c -o $@ $<

#---------------------------------




clean:
	rm -f *.o shared_memory_bank_acc 2_accs_sem_shm_bank rm_shm_sem sem_spam_shm_bank spam_shm_bank_acc

