
INCDIR =.
CC=gcc
CFLAGS=-I$(INCDIR)
OBJ=pscheduler.o list.o
DEPS=pscheduler.h list.h
SRC=pscheduler.c list.c
TARGET=scheduler

$(TARGET): main.c $(OBJ) 
	$(CC) -o $(TARGET) main.c $(OBJ) $(CFLAGS)

$(OBJ): $(SRC) $(DEPS)
	$(CC) -c $(SRC) $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o *~ core $(INCDIR)/*~ $(TARGET)
