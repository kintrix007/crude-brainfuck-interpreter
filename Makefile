app=bf

$(app): main.c
	gcc -o $(app) main.c && strip $(app)

clean:
	rm ./$(app)
