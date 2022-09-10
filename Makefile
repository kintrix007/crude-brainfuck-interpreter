app=bf

$(app): main.c
	gcc -O2 -o $(app) main.c && strip $(app)

clean:
	rm ./$(app)
