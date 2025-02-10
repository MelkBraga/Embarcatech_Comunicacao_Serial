# Embarcatech_Comunicacao_Serial
Repositório criado para a Tarefa 1 - Aula Síncrona 03/02 - Comunicação Serial com RP2040

Aluno: Melk Silva Braga

# Instruções de compilação

Para compilar o código, são necessárias as seguintes extensões: 

*Raspberry Pi Pico*

*Cmake*

*Cmake Tools*

Para rodar o código na placa BitDogLab, é necessário conectá-la via cabo usb em modo bootsel 
e ter instalado o driver WinUSB (v6.1.7600.16385) na interface 1. Só então clicar em "Run Project",
na extensão Raspberry Pi Pico.

Em seguida, abrir o Serial Monitor do VSCode, escolher a posta COM adequada e iniciar o monitoramento,
para que informações possam ser visualizadas e caracteres possam ser digitados. O program aceita caracteres 
de a-z, A-Z, 0-9 e os especiais "@ # $ % &", e ao digitar e enviar qualquer um deles pelo Serial Monitor, 
vai ser exibido no display da placa. Caso sejam números, a matriz de LED também exibirá um desenho correspondente.

Ao clicar no botão A, o LED verde é ligado ou desligado, e uma informação é exibida no display e no serial monitor.
Ao clicar no botão B, o LED azul é ligado ou desligado, e uma informação é exibida no display e no serial monitor.

# Vídeo demonstrativo
https://youtu.be/2CgC29dd6aY

* No vídeo, o display aparece oscilando por conta do efeito estroboscópico na câmera
