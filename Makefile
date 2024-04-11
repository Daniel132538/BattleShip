todos: fragmenta cliente servidor 
fragmenta: fragmenta.c
	gcc -c fragmenta.c
cliente: fragmenta
	gcc -o cliente1 cliente1.c fragmenta.o
	gcc -o cliente2 cliente2.c fragmenta.o
	gcc -o clienteExtra clienteExtra.c fragmenta.o
servidor: fragmenta
	gcc -o servidor1 servidor1.c fragmenta.o
	gcc -o servidor2 servidor2.c fragmenta.o
	gcc -o servidorExtra servidorExtra.c fragmenta.o
	
clean: 
	rm ./cliente1 ./cliente2 ./servidor1 ./servidor2 ./servidorExtra ./clienteExtra fragmenta.o
