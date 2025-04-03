CC = gcc
CFLAGS = -O2

mysh: shell.o interpreter.o shellmemory.o pcb.o queue.o schedule_policy.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -DFRAMESIZE=$(framesize) -DVARMEMSIZE=$(varmemsize) -c $<

clean:
	rm -f mysh *.o
