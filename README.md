=========================================================
      GUIA DO ARQUIVO DE ENTRADA (INPUT FILE)
=========================================================

Este arquivo de entrada é utilizado para configurar os
parâmetros da estrela e os dados dos períodos observados 
para a rotina de ajuste. O arquivo deve ser formatado em 
texto puro, respeitando estritamente a ordem das linhas.

---------------------------------------------------------
ESTRUTURA DO ARQUIVO (Linha por Linha)
---------------------------------------------------------
Linha 1: Nome da Estrela
         Exemplo: G117-B15A

Linha 2: Valor de Chi
         Exemplo: 0, 1, 1.5, 2, 3...

Linha 3: Temperatura Efetiva (Teff)
         Define o limite de temperatura a ser analisado.
         - Digite "0" : Sem filtro (range de 0 a infinito).
         - Digite "x y" : Para definir um intervalo específico
           separado por espaço (ex: "11000 12000"):

         [Limite Inferior da Teff]   [Limite Superior da Teff]

Linha 4: Massa
         Define o limite de massa solar a ser analisado.
         - Digite "0" : Sem filtro (range de 0 a infinito).
         - Digite "x y" : Para definir um intervalo específico
           separado por espaço (ex: "0.5 0.7"):

         [Limite Inferior da massa]   [Limite Superior da massa]

Linha 5 em diante: Modos de Pulsação (Períodos)
         Cada linha a partir daqui representa um período
         observado, contendo 3 colunas separadas por espaços
         ou tabulações:
         
         [Período]   [Amplitude]   [Grau Esférico (l)]

         Regras exclusivas para a coluna do Grau Esférico (l):
         - 0 : Parâmetro livre. O programa testará e escolherá 
               automaticamente entre l=1 ou l=2.
         - 1 : Modo fixado manualmente pelo pesquisador em l=1.
         - 2 : Modo fixado manualmente pelo pesquisador em l=2.

---------------------------------------------------------
EXEMPLO PRÁTICO DE ARQUIVO
---------------------------------------------------------
G117-B15A
3
0
0.5 0.7
215.20      17.36      1
270.46      06.14      1
304.05      07.48      1

---------------------------------------------------------
EXPLICAÇÃO DO EXEMPLO ACIMA
---------------------------------------------------------
- G117-B15A : Estrela analisada.
- 3         : Valor do chi configurado como 3.
- 0         : Teff sem filtro (livre de 0 a infinito).
- 0.5 0.7   : Massa com filtro (entre 0.5 a 0.7 massas solares).
- Linhas finais: Foram inseridos 3 períodos (215.20, 270.46 
 e 304.05) com suas respectivas amplitudes. O pesquisador 
  fixou todos os modos em l=1 (terceira coluna).
