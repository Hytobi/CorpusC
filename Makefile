all: exec run clean

exec: main.o
	gcc -o exec main.c

run: exec
	./exec texte1.txt 4577 texte2.txt 4280 texte3.txt 1302 texte4.txt 3220

clean:
	rm -f *.o exec