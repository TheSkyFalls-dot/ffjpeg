CC=gcc

CFLAGS=-O3 -fopenmp -Isrc -Iinclude

SRC=src/main.c
OBJ=build/main.o

EXE=build/ffjpeg.exe
DLL=build/ffjpeg.dll

all: $(EXE) $(DLL)

build:
	mkdir -p build

$(OBJ): $(SRC) | build
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -fopenmp -o $(EXE)

$(DLL): $(SRC)
	$(CC) $(SRC) -shared $(CFLAGS) -o $(DLL)

clean:
	rm -rf build
