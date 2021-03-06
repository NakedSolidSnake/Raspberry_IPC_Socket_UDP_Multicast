# UDP Multicast

## Tópicos
* [Introdução](#introdução)
* [Endereço Multicast](#endereço-multicast)
* [Representação do Multicast na rede](#representação-do-multicast-na-rede)
* [Selecionando o endereço multicast](#selecionando-o-endereço-multicast)
* [Preparação do Ambiente](#preparação-do-ambiente)
* [netcat](#netcat)
* [tcpdump](#tcpdump)
* [Implementação](#implementação)
* [Biblioteca](#biblioteca)
* [udp_multicast_receiver.h](#udp_multicast_senderh)
* [udp_multicast_receiver.c](#udp_multicast_receiverc)
* [udp_multicast_sender.h](#udp_multicast_senderh)
* [udp_multicast_sender.c](#udp_multicast_senderc)
* [launch_processes](#launch_processes)
* [button_interface](#button_interface)
* [led_interface](#led_interface)
* [button_process](#button_process)
* [led_process](#led_process)
* [Compilando, Executando e Matando os processos](#compilando-executando-e-matando-os-processos)
* [Compilando](#compilando)
* [Clonando o projeto](#clonando-o-projeto)
* [Selecionando o modo](#selecionando-o-modo)
* [Modo PC](#modo-pc)
* [Modo RASPBERRY](#modo-raspberry)
* [Executando](#executando)
* [Interagindo com o exemplo](#interagindo-com-o-exemplo)
* [MODO PC](#modo-pc-1)
* [MODO RASPBERRY](#modo-raspberry-1)
* [Monitorando o tráfego usando o tcpdump](#monitorando-o-tráfego-usando-o-tcpdump)
* [Testando conexão com o servidor via netcat](#testando-conexão-com-o-servidor-via-netcat)
* [Matando os processos](#matando-os-processos)
* [Conclusão](#conclusão)
* [Referência](#referência)

## Introdução
O UDP no modo broadcast permite enviar mensagens para todas as máquinas conectadas na rede de uma única vez, porém essa forma de envio pode prejudicar o desempenho da rede dependendo do tamanho e da frequência da mensagem enviada, para contornar esses problemas existe um modo conhecido como _multicast_, que é parecido com o _broadcast_ mas envia a mensagem somente para as máquinas que estejam interessadas nesse conteúdo, dessa forma evita-se que haja um congestionamento na rede devido a replicação de mensagens para máquinas não interessadas. Para que as máquinas interessadas na mensagem transmitida, essas máquinas deverão se cadastrar em um grupo conhecido como IP _multicast_ para que possam a partir desse registro receberem as mensagens.

## Endereço Multicast
Para determinar o endereço _multicast_ é necessário conhecer as classes de IP que são separadas em classes A, B, C, D e E. Para modo _multicast_ foi reservado a classe D que é dedicado exclusivamente para esse propósito, possuindo um _range_ de 224.0.0.0 até 239.255.255.255, dessa forma o emissor pode enviar para qualquer um desses endereços.

## Representação do Multicast na rede
Quando uma mensagem _multicast_ é enviada as máquinas registradas irão receber essas mensagens. Para ilustrar, o exemplo representa a transmissão de uma mensagem _multicast_.

<p align="center">
<img src="https://thumbs.gfycat.com/AfraidUnitedHydatidtapeworm-small.gif">
</p>

Na imagem é possível notar que as mensagem chegam somente nas máquinas interessadas


## Selecionando o endereço multicast
Para selecionar o endereço de _multicast_ para aplicação podemos pesquisar no site [www.iana.org](http://www.iana.org/assignments/multicast-addresses/multicast-addresses.xhtml), que apresenta a finalidade de cada _range_.

De acordo com as recomendações, é selecionado o intervalo 232.192.0.0/24, sendo esse reservado para aplicações de uso privado, dentro desse _range_ é selecionado o 239.192.1.1 como endereço do grupo _multicast_ para a aplicação.

## Preparação do Ambiente
Antes de apresentar o exemplo, primeiro é necessário instalar algumas ferramentas para auxiliar na análise da comunicação. As ferramentas necessárias para esse artigo são o **tcpdump** e o **netcat(nc)**, para instalá-las basta executar os comandos abaixo:

```c
sudo apt-get update
```
```c
sudo apt-get install netcat
```
```c
sudo apt-get install tcpdump
```

## netcat 
O _netcat_ é uma ferramenta capaz de interagir com conexões UDP e TCP, podendo abrir conexões, ouvindo como um servidor, ou como cliente enviando mensagens para um servidor.

## tcpdump
O tcpdump é uma ferramenta capaz de monitorar o tráfego de dados em uma dada interface como por exemplo eth0, com ele é possível analisar os pacotes que são recebido e enviados.

## Implementação

Para demonstrar o uso desse IPC, é adotado o modelo Cliente/Servidor, onde o processo Cliente(_button_process_) vai enviar uma mensagem via multicast para o servidor(_led_process_) que vai ler a mensagem, e verificar se corresponde com os comandos cadastrados internamente e processar o comando caso seja válido. 

## Biblioteca
A biblioteca criada permite uma fácil criação do servidor, sendo o servidor orientado a eventos, ou seja, fica aguardando as mensagens chegarem.

### udp_multicast_receiver.h
Primeiramente é criado um _callback_ responsável pelo tratamento de eventos de recebimento, essa função será chamada quando houver esse evento.
```c
typedef void (*Event)(const char *buffer, size_t buffer_size, void *data);
```

É criado também um contexto que armazena os parâmetros utilizados pelo servidor, sendo o _socket_ para armazenar a instância criada, _port_ que recebe o número que corresponde onde o serviço será disponibilizado, _buffer_ que aponta para a memória alocada previamente pelo usuário, *buffer_size* o representa o tamanho do _buffer_, o _callback_ para recepção da mensagem e o endereço do grupo _multicast_

```c
typedef struct 
{
    int socket;
    int port;
    char *buffer;
    size_t buffer_size;
    Event on_receive_message;
    const char *multicast_group;
} UDP_Receiver;
```

Essa função inicializa o servidor com os parâmetros do contexto
```c
bool UDP_Multicast_Receiver_Init(UDP_Receiver *receiver);
```
Essa função aguarda uma mensagem publicada no grupo _multicast_ pelo cliente.
```c
bool UDP_Multicast_Receiver_Run(UDP_Receiver *receiver, void *user_data);
```

### udp_multicast_receiver.c

No *UDP_Multicast_Receiver_Init* é definido algumas variáveis para auxiliar na inicialização do servidor, sendo uma variável booleana que representa o estado da inicialização do servidor, uma variável do tipo inteiro para habilitar o reuso da porta caso o servidor precise reiniciar, uma estrutura sockaddr_in que é usada para configurar o servidor para se comunicar através da rede e uma estrutura utilizada para o registro do servidor no grupo _multicast_.

```c
bool status = false;
int yes = 1;
struct sockaddr_in server_addr;
struct ip_mreq multicast;
```

Para realizar a inicialização é criado um _dummy do while_, para que quando houver falha em qualquer uma das etapas, irá sair da função com status de erro, nesse ponto é verificado se o contexto, o _buffer_ e se o tamanho do _buffer_ foi inicializado, sendo sua inicialização de responsabilidade do usuário

```c
if(!receiver || !receiver->buffer || !receiver->buffer_size)
    break;
```
É criado um _endpoint_ com o perfil de se conectar via protocolo IPv4(AF_INET), do tipo _datagram_ que caracteriza o UDP(SOCK_DGRAM), o último parâmetro pode ser 0 nesse caso.
```c
receiver->socket = socket(AF_INET, SOCK_DGRAM, 0);
if(receiver->socket < 0)
    break;
```
A estrutura é preenchida com parâmetros fornecidos pelo usuário como em qual porta que o serviço vai rodar.
```c
memset(&server_addr, 0, sizeof(server_addr));

server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(receiver->port);
```

Aqui é habilitado o reuso do _socket_ caso necessite reiniciar o serviço
```c
if (setsockopt(receiver->socket, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0)
    break;
```
É aplicada as configurações ao _socket_ criado
```c
if (bind(receiver->socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    break;

```
O servidor é registrado no grupo multicast e é atribuído _true_ na variável _status_
```c
multicast.imr_multiaddr.s_addr = inet_addr(receiver->multicast_group);
multicast.imr_interface.s_addr = htonl(INADDR_ANY);

if(setsockopt(receiver->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicast, sizeof(multicast)) < 0)
    break;
status = true;
```

Na função *UDP_Multicast_Receiver_Run* é declarada algumas variáveis para receber as mensagens por meio do _multicast_

```c
bool status = false;
struct sockaddr_in client_addr;
socklen_t len = sizeof(client_addr);
size_t read_size;
```
É verificado se o _socket_ é válido e é aguardada uma mensagem no canal de _multicast_, a mensagem é repassada para o _callback_ para realizar o tratamento de acordo com a aplicação do cliente, e é retornado o _status_.
```c
if(receiver->socket > 0)
{
    read_size = recvfrom(receiver->socket, receiver->buffer, receiver->buffer_size, MSG_WAITALL,
                                (struct sockaddr *)&client_addr, &len); 
    receiver->buffer[read_size] = 0;
    receiver->on_receive_message(receiver->buffer, read_size, user_data);
    memset(receiver->buffer, 0, receiver->buffer_size);
    status = true;
}

return status;
```

### udp_multicast_sender.h
É criado também um contexto que armazena os parâmetros utilizados pelo cliente, sendo o _socket_ para armazenar a instância criada, _hostname_ é o _ip_ do canal _multicast_ que vai ser enviado as mensagens e o _port_ que recebe o número que corresponde onde o serviço vai ser disponibilizado

```c
typedef struct 
{
    int socket;
    const char *hostname;
    const char *port;
} UDP_Sender;
```

Inicializa o cliente com os parâmetros do descritor 
```c
bool UDP_Multicast_Sender_Init(UDP_Sender *sender);
```

Envia mensagem para o grupo _multicast_ baseado nos parâmetros do descritor.
```c
bool UDP_Multicast_Sender_Send(UDP_Sender *sender, const char *message, size_t message_size);
```
### udp_multicast_sender.c

Na função *UDP_Multicast_Sender_Init* é verificado se o contexto foi iniciado, o _socket_ é configurado como UDP e é habilitado o envio no modo _multicast_

```c
int multicast_enable;
bool status = false;
do 
{
    if(!sender)
        break;

    sender->socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sender->socket < 0)
        break;

    multicast_enable = 1;
    if(setsockopt(sender->socket, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&multicast_enable, sizeof(multicast_enable)) < 0)
        break;

    status = true;        
}while(false);

return status;
```

Na função *UDP_Multicast_Sender_Send* é declarada algumas variáveis para auxiliar na comunicação com o servidor, sendo uma variável booleana que representa o estado de envio para o servidor, uma estrutura sockaddr_in que é usada para configurar o servidor no qual será enviado as mensagens e uma variável de quantidade de dados enviados.

```c
bool status = false;
struct sockaddr_in server;
ssize_t send_len;
```
A estrutura é parametrizada com os dados do servidor
```c
memset(&server, 0, sizeof(server));
server.sin_family = AF_INET;
server.sin_addr.s_addr = inet_addr(sender->hostname);
server.sin_port = htons(atoi(sender->port));
```
Realiza o envio da mensagem para o canal _multicast_
```c
send_len = sendto(sender->socket, message, message_size, 0, (struct sockaddr *)&server, sizeof(server));
  if(send_len == message_size)
      status = true;

return status;
```
Aplicação é composta por três executáveis sendo eles:
* _launch_processes_ - é responsável por lançar os processos _button_process_ e _led_process_ através da combinação _fork_ e _exec_
* _button_interface_ - é responsável por ler o GPIO em modo de leitura da Raspberry Pi e escrever o estado interno no arquivo
* _led_interface_ - é responsável por ler do arquivo o estado interno do botão e aplicar em um GPIO configurado como saída

### *launch_processes*

No _main_ é criada duas variáveis para armazenar o PID do *button_process* e do *led_process*, e mais duas variáveis para armazenar o resultado caso o _exec_ venha a falhar.
```c
int pid_button, pid_led;
int button_status, led_status;
```

Em seguida é criado um processo clone, se processo clone for igual a 0, é criado um _array_ de *strings* com o nome do programa que será usado pelo _exec_, em caso o _exec_ retorne, o estado do retorno é capturado e será impresso no *stdout* e aborta a aplicação. Se o _exec_ for executado com sucesso o programa *button_process* será carregado. 
```c
pid_button = fork();

if(pid_button == 0)
{
    //start button process
    char *args[] = {"./button_process", NULL};
    button_status = execvp(args[0], args);
    printf("Error to start button process, status = %d\n", button_status);
    abort();
}   
```

O mesmo procedimento é repetido novamente, porém com a intenção de carregar o *led_process*.

```c
pid_led = fork();

if(pid_led == 0)
{
    //Start led process
    char *args[] = {"./led_process", NULL};
    led_status = execvp(args[0], args);
    printf("Error to start led process, status = %d\n", led_status);
    abort();
}
```

## *button_interface*
É definida uma lista de comandos que para o envio
```c
const char *led_commands[] = 
{
    "LED ON",
    "LED OFF"
};
```
A implementação do Button_Run ficou simples, onde é realizada a inicialização do interface de botão e fica em loop aguardando o pressionamento do botão para alterar o estado da variável e enviar a mensagem para o canal multicast
```c
bool Button_Run(UDP_Sender *sender, Button_Data *button)
{
    int state = 0;

    if(button->interface->Init(button->object) == false)
        return false;

    if(UDP_Multicast_Sender_Init(sender) == false)
        return false;

    while (true)
    {
        wait_press(button);
        state ^= 0x01;
        UDP_Multicast_Sender_Send(sender, led_commands[state], strlen(led_commands[state]));
    }

    return false;
}
```
## *led_interface*
A implementação do LED_Run ficou simplificada, é realizada a inicialização da interface de LED, do servidor e fica em loop aguardando o recebimento de uma mensagem.
```c
bool LED_Run(UDP_Receiver *receiver, LED_Data *led)
{

	if(led->interface->Init(led->object) == false)
		return false;

	if(UDP_Multicast_Receiver_Init(receiver) == false) 
		return false;


	while(true)
	{
		UDP_Multicast_Receiver_Run(receiver, led);
	}

	return false;	
}
```

## *button_process*
A parametrização do cliente fica por conta do processo de botão que inicializa o contexto com o endereço _multicast_, o serviço e assim os argumentos são passados para *Button_Run* iniciar o processo.

```c
UDP_Sender sender = 
{
    .hostname = "239.192.1.1",
    .port  = "1234"
};

Button_Run(&sender, &button);
```
## *led_process*
A parametrização do servidor fica por conta do processo de LED que inicializa o contexto com o _buffer_, seu tamanho, a porta do serviço que vai consumir e o _callback_ preenchido, e assim os argumentos são passados para *LED_Run* iniciar o serviço.
```c
UDP_Server receiver = 
{
    .buffer = server_buffer,
    .buffer_size = BUFFER_SIZE,
    .port = 1234,
    .on_receive_message = on_receive_message,
    .multicast_group = "239.192.1.1"
};

LED_Run(&receiver, &led);
```

A implementação no evento de recebimento da mensagem, compara a mensagem recebida com os comandos internos para o acionamento do LED, caso for igual executa a ação correspondente.
```c
void on_receive_message(const char *buffer, size_t buffer_size, void *data)
{
    LED_Data *led = (LED_Data *)data;

    if(strncmp("LED ON", buffer, strlen("LED ON")) == 0)
        led->interface->Set(led->object, 1);
    else if(strncmp("LED OFF", buffer, strlen("LED OFF")) == 0)
        led->interface->Set(led->object, 0);
}
```

## Compilando, Executando e Matando os processos
Para compilar e testar o projeto é necessário instalar a biblioteca de [hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware) necessária para resolver as dependências de configuração de GPIO da Raspberry Pi.

## Compilando
Para facilitar a execução do exemplo, o exemplo proposto foi criado baseado em uma interface, onde é possível selecionar se usará o hardware da Raspberry Pi 3, ou se a interação com o exemplo vai ser através de _input_ feito por FIFO e o _output_ visualizado através de LOG.

### Clonando o projeto
Pra obter uma cópia do projeto execute os comandos a seguir:

```bash
$ git clone https://github.com/NakedSolidSnake/Raspberry_IPC_Socket_UDP_Multicast
$ cd Raspberry_IPC_Socket_UDP_Multicast
$ mkdir build && cd build
```

### Selecionando o modo
Para selecionar o modo é necessário passar para o cmake uma variável de ambiente chamada de ARCH, e pode-se passar os seguintes valores, PC ou RASPBERRY, para o caso de PC o exemplo terá sua interface preenchida com os sources presentes na pasta src/platform/pc, que permite a interação com o exemplo através de FIFO e LOG, caso seja RASPBERRY usará os GPIO's descritos no [artigo](https://github.com/NakedSolidSnake/Raspberry_lib_hardware#testando-a-instala%C3%A7%C3%A3o-e-as-conex%C3%B5es-de-hardware).

#### Modo PC
```bash
$ cmake -DARCH=PC ..
$ make
```

#### Modo RASPBERRY
```bash
$ cmake -DARCH=RASPBERRY ..
$ make
```

## Executando
Para executar a aplicação execute o processo _*launch_processes*_ para lançar os processos *button_process* e *led_process* que foram determinados de acordo com o modo selecionado.

```bash
$ cd bin
$ ./launch_processes
```

Uma vez executado podemos verificar se os processos estão rodando atráves do comando 
```bash
$ ps -ef | grep _process
```

O output 
```bash
cssouza  31588  2298  0 08:15 pts/1    00:00:00 ./button_process
cssouza  31589  2298  0 08:15 pts/1    00:00:00 ./led_process
```
## Interagindo com o exemplo
Dependendo do modo de compilação selecionado a interação com o exemplo acontece de forma diferente

### MODO PC
Para o modo PC, precisamos abrir um terminal e monitorar os LOG's
```bash
$ sudo tail -f /var/log/syslog | grep LED
```

Dessa forma o terminal irá apresentar somente os LOG's referente ao exemplo.

Para simular o botão, o processo em modo PC cria uma FIFO para permitir enviar comandos para a aplicação, dessa forma todas as vezes que for enviado o número 0 irá logar no terminal onde foi configurado para o monitoramento, segue o exemplo
```bash
echo  "0" > /tmp/multicast_fifo
```

Output do LOG quando enviado o comando algumas vezes
```bash
May 23 08:16:19 dell-cssouza LED UDP[31589]: LED Status: On
May 23 08:16:20 dell-cssouza LED UDP[31589]: LED Status: Off
May 23 08:16:21 dell-cssouza LED UDP[31589]: LED Status: On
May 23 08:16:21 dell-cssouza LED UDP[31589]: LED Status: Off
May 23 08:16:22 dell-cssouza LED UDP[31589]: LED Status: On
May 23 08:16:23 dell-cssouza LED UDP[31589]: LED Status: Off
```

### MODO RASPBERRY
Para o modo RASPBERRY a cada vez que o botão for pressionado irá alternar o estado do LED.

## Monitorando o tráfego usando o tcpdump

Para monitorar as mensagens que trafegam, é necessário ler uma interface que corresponde ao endereço _multicast_, para saber quais interfaces que o computador possui é utilizado o comando:

```bash
$ netstat -ng
```
Output
```bash
IPv6/IPv4 Group Memberships
Interface       RefCnt Group
--------------- ------ ---------------------
lo              1      224.0.0.251
lo              1      224.0.0.1
enp0s31f6       1      239.192.1.1
enp0s31f6       1      224.0.0.251
enp0s31f6       1      224.0.0.1
docker0         1      224.0.0.251
docker0         1      224.0.0.1
docker0         1      224.0.0.106
vboxnet0        1      224.0.0.251
vboxnet0        1      224.0.0.1
lo              1      ff02::fb
lo              1      ff02::1
lo              1      ff01::1
enp0s31f6       1      ff02::1:ff08:a1fb
enp0s31f6       1      ff02::1:ff92:16a6
enp0s31f6       1      ff02::fb
enp0s31f6       1      ff02::1:ff1e:b93
enp0s31f6       1      ff02::1:ff1c:79de
enp0s31f6       1      ff02::1:ffda:d8cd
enp0s31f6       1      ff02::1
enp0s31f6       1      ff01::1
wlp2s0          1      ff02::1
wlp2s0          1      ff01::1
docker0         1      ff02::6a
docker0         1      ff02::1
docker0         1      ff01::1
vboxnet0        1      ff02::fb
vboxnet0        1      ff02::1:ff00:0
vboxnet0        1      ff02::1
vboxnet0        1      ff01::1
```
Como é possível ver existem diversos grupos _multicast_ disponíveis, no caso dessa máquina em questão podemos verificar que existe o endereço do grupo selecionado para a aplicação na inteface enp0s31f6 com o endereço 239.192.1.1

O tcpdump possui opções que permite a visualização dos dados, não irei explicar tudo, fica de estudo para quem quiser saber mais sobre a ferramenta. Executando o comando podemos ver todas as mensagens de multicast

```bash
sudo tcpdump -i enp0s31f6 -nnSX "multicast"
```

Após executar o comando o tcpdump ficará fazendo _sniffing_ da interface, tudo o que for trafegado nessa interface será apresentado, dessa forma enviando um comando e é possível ver a seguinte saída:
```bash
08:21:58.903735 IP 192.168.0.140.38455 > 239.192.1.1.1234: UDP, length 6
	0x0000:  4500 0022 e013 4000 0111 e7c1 c0a8 008c  E.."..@.........
	0x0010:  efc0 0101 9637 04d2 000e b215 4c45 4420  .....7......LED.
	0x0020:  4f4e                                     ON
```

* No instante **08:21:58.903735 IP 192.168.0.140.38455 > 239.192.1.1.1234** o cliente envia uma mensagem para o servidor via _multicast_

## Testando conexão com o servidor via netcat
A aplicação realiza a comunicação entre processos locais, para testar uma comunicação remota é usado o netcat que permite se conectar de forma prática ao servidor e enviar os comandos. Para se conectar basta usar o seguinte comando:

```bash
nc -u ip port
```

Como descrito no comando netstat é usado o ip de _multicast_ apresentado na interface enp0s31f6 que é o IP 239.192.1.1, então o comando fica

```bash
echo -e "LED ON" | nc -u 239.192.1.1 1234
```

É enviado o comando LED ON, se visualizar no log irá apresentar que o comando foi executado, para monitorar com o tcpdump basta mudar a interface

## Matando os processos
Para matar os processos criados execute o script kill_process.sh
```bash
$ cd bin
$ ./kill_process.sh
```

## Conclusão
O _multicast_ é uma boa solução para enviar mensagens de uma única vez para os interessados, diferente do _broadcast_ somente os registrados irão receber as mensagens, evitando assim o congestionamento do tráfego, essa forma de envio se assemelha ao padrão arquitetural _publish–subscribe_ como por exemplo o MQTT.


## Referência
* [Link do projeto completo](https://github.com/NakedSolidSnake/Raspberry_IPC_Socket_UDP_Multicast)
* [Mark Mitchell, Jeffrey Oldham, and Alex Samuel - Advanced Linux Programming](https://www.amazon.com.br/Advanced-Linux-Programming-CodeSourcery-LLC/dp/0735710430)
* [fork, exec e daemon](https://github.com/NakedSolidSnake/Raspberry_fork_exec_daemon)
* [biblioteca hardware](https://github.com/NakedSolidSnake/Raspberry_lib_hardware)

