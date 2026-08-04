# Ajuste de Modelos Estelares

> Documentação baseada no código-fonte `codigo.c`.

## Visão Geral

Este programa realiza o ajuste de modelos estelares pulsantes comparando períodos observados com períodos teóricos provenientes de uma grade de modelos.

Para cada estrela presente em `estrelas.txt`, o programa:

1. Lê os parâmetros da estrela.
2. Procura recursivamente todos os arquivos `parametros.dat` dentro da pasta `massas/`.
3. Localiza automaticamente os arquivos `.sal` e `.sal2` correspondentes.
4. Associa cada período observado ao período teórico mais próximo.
5. Calcula as métricas `chi²`, `chi_kepler`, `sqrt(chi_kepler)` e `chi_bk`.
6. Filtra os modelos de acordo com os limites definidos pelo usuário.
7. Gera automaticamente os arquivos de saída em `Resultados/`.

---

# Estrutura do projeto

```text
.
├── codigo.c
├── codigo.py
├── estrelas.txt
├── ordenarResultadoFinal.sh
└── massas/
```

A pasta `massas/` **não faz parte do repositório**. Ela deve ser fornecida pelo usuário.

Estrutura esperada:

```text
massas/
└── massa_solar/
    └── massa_hidrogenio/
        ├── parametros.dat
        ├── modelo001.sal
        ├── modelo001.sal2
        └── ...
```

---

# Compilação

```bash
gcc codigo.c -o calcular_estrelas -lm
```

# Execução

```bash
./calcular_estrelas
```

Ao terminar, o script `ordenarResultadoFinal.sh` é executado automaticamente.

---

# Arquivo estrelas.txt

O arquivo pode conter **quantas estrelas forem desejadas**.

Cada estrela ocupa um bloco independente.

Os blocos são separados por **uma linha em branco** (ou comentário iniciado por `#`).

Formato:

```text
Nome
Chi
Teff
Massa
Periodo Amplitude l
Periodo Amplitude l
...
```

## Linha 1

Nome da estrela.

## Linha 2

Valor limite de χ utilizado como filtro dos modelos.

Aceita valores inteiros ou decimais:

```text
0
1
1.5
2
2.5
3
3.5
```

## Linha 3

Filtro de temperatura.

Sem filtro:

```text
0
```

Com intervalo:

```text
10985 12985
```

## Linha 4

Filtro de massa.

Sem filtro:

```text
0
```

Com intervalo:

```text
0.493 0.693
```

## Demais linhas

```text
Periodo Amplitude l
```

onde

- período observado
- amplitude
- índice l

Valores possíveis para l:

| l | Significado |
|---|-------------|
|0|Escolha automática entre .sal e .sal2|
|1|Força utilizar .sal|
|2|Força utilizar .sal2|

---

# Exemplo

```text
G117-B15A
3
10985 12985
0.493 0.693
215.20 17.36 1
270.46 6.14 1
304.05 7.48 1

L19-2
1
11105 13105
0.605 0.805
113.80 2.48 0
118.70 1.20 0
143.60 0.60 0
192.60 6.50 0
350.10 1.10 0
```

O programa processa cada bloco sequencialmente até o final do arquivo.

---

# Saídas

Para cada estrela é criada uma pasta:

```text
Resultados/
└── Nome_da_Estrela/
```

Os arquivos TESS contendo os modelos aprovados são gravados automaticamente nessa pasta.

---

# Observações

- A pasta `massas/` deve existir na raiz do projeto.
- Cada diretório de modelos deve conter `parametros.dat`.
- O programa procura esses arquivos automaticamente de forma recursiva.
- Comentários iniciados por `#` são ignorados.
- Linhas em branco encerram um bloco de estrela.
