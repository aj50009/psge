CC=ccpsx
CX=cpe2x

all: resources obj\main.cpe
	$(CX) /CE obj\main.cpe

resources:
	

obj\main.cpe: src\main.c
	$(CC) -O3 -Xo$80010000 src\main.c src\psge.c src\shiftjis.c -oobj\main.cpe,obj\main.sym,obj\main.map
