# cli-lib
Command Line Interface library, for developing CLI applications and games in C. It has functions to access keyboard, screen and manage timing tasks.

Nosso jogo se trata se chama Pong, um clássico jogo de tênis eletrônico em que dois jogadores (ou um jogador contra o computador) controlam raquetes verticais e tentam rebater uma bola que se move pelo campo. O objetivo é evitar que a bola ultrapasse a raquete do jogador, enquanto tenta fazer com que a bola passe pela raquete do adversário, fazendo-o perder vidas.

## Detalhes principais do jogo:
* Campo de Jogo: O campo tem um tamanho definido de 80x24 (largura x altura), com bordas e um fundo onde a bola e as raquetes se movem.
* Raquetes: Cada jogador tem uma raquete representada por um retângulo vertical que pode ser movido para cima e para baixo. A raquete do Jogador 1 se move com as teclas 'w' e 's', enquanto a do Jogador 2 se move com as teclas 'o' e 'l'. No modo single-player, a raquete do computador se move automaticamente.
* Bola: A bola se move com uma velocidade inicial e rebate nas bordas do campo e nas raquetes. Quando a bola atinge a borda esquerda ou direita do campo, o jogador perde uma vida e o outro ganha um ponto.
* Vidas e Pontuação: Cada jogador começa com 4 vidas. O jogo termina quando um jogador perde todas as suas vidas. As pontuações e as vidas restantes são exibidas no final de cada rodada.
* Modo single-player: Nesse modo, o jogador pode treinar suas habilidades jogando contra a inteligência artificial (IA), sem precisar de um segundo jogador.

## Regras:
O jogador deve controlar sua raquete para impedir que a bola ultrapasse sua linha de base.
O adversário (ou o computador) tenta fazer o mesmo. No modo single-player, o computador move a raquete automaticamente.
Quando a bola atinge o limite da tela à esquerda ou à direita, o jogador perde uma vida e o outro ganha um ponto.
O jogo termina quando um dos jogadores perde todas as suas vidas.

## Requerimentos
- A biblioteca funciona nos sistemas operacionais:
   - Linux based (Ubuntu, etc)
   - MacOS
- É necessario o gcc instalado.

## Como realizar a compilação
<ul>
    <li>Passo 1: Acesso ao Diretório - 
Abra um terminal e navegue até o diretório onde os arquivos do Pong estão localizados usando o comando
cd:
cd seu/caminho/para/Pong/</li>
    <li>Passo 2: Compilação -  
Por possuir um arquivo makefile automatizamos o processo de compilação e construção do jogo, para compilar baste ir no terminal e usar o comando 
make</a></li>
    <li>Passo 3: Execução - 
Por conta do makefile para o proximo processo basta colocar o comando 
make run
no terminal
</li>
</ul>



