CC = gcc
OBJ = dump.o main.o space.o inode.o

EXE = run
all: $(EXE)
.c.o: ; $(CC) -c $*.c

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ)

clean:
	del /F /Q $(EXE) *.o *.d core 2>NUL || true
