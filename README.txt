Alunos: Emanuelly Parreira 
	Lucas Bomfim
	Vinicius Matheus

** Trabalho: Simulador de um Sistema Detector de Incêndios Florestais **

** Para compilar e executar: 

	Vá ao diretório onde se encontra o arquivo main.c e, no terminal, insira o comando <gcc main.c -o main>. Isso irá criar o executável, daí basta digitar ./main para executar o código.

** Sobre o funcionamento do programa:

	O programa foi desenvolvido usando uma técnica multithreading, ou seja, usamos várias threads que executam simultaneamente as rotinas determinadas. Sendo assim, para simular um sistema detector de incêndios usamos 4 rotinas de thread diferentes:

		-Thread de Incêndio: responsável por gerar uma posição aleatória na matriz (que representa o mapa florestal) e na posição definida gerar um incêndio (plotar um '@'); também é responsável por imprimir o mapa enquanto o programa estiver executando.
		-Thread Sensor: é responsável por ficar monitorando nas posições vizinhas de cada thread sensor se existe algum '@'(que indica que há incêndio). Se o incêndio é identificado informações sobre o incêndio e sobre o censor são guardados em uma struct que representa cada sensor; além disso, quando o incêndio é identificado é chamado uma função auxiliar que é responsável por propagar essa informação pelos outros sensores vizinhos, até que se chegue nos sensores que se encontram na borda de cima, de baixo, da esquerda e da direita.
		-Thread Central: é responsável por ficar monitorando se os sensores da borda do mapa receberam alguma informação (mensagem de incêndio), quando sim, essa rotina irá gravar essa mensagem em um arquivo chamado "incendio.log", e também vai chamar a thread bombeiro para apagar o incendio (transforma '@' em '-')
		-Thread Bombeiro: é responsável por apagar o incendio identificado pelo sensor.
	