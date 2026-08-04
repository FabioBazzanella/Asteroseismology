# Ajuste de Modelos Estelares

> **Ferramenta para ajuste de modelos estelares pulsantes utilizando o método do χ² (Chi-Quadrado).**

Este projeto percorre uma grade de modelos estelares, compara os períodos teóricos com os períodos observados e determina os modelos que melhor reproduzem as observações.

Os resultados incluem, entre outros parâmetros:

- Temperatura efetiva (Teff)
- Massa estelar
- Massa da camada de hidrogênio
- Valor de χ²
- Modos sísmicos correspondentes

---

# Estrutura do Projeto

```
.
├── codigo.c
├── codigo.py
├── estrelas.txt
├── ordenarResultadoFinal.sh
└── massas/
└── Resultados/

```

## Importante

A pasta `massas/` **não está incluída neste repositório**, pois contém uma grande quantidade de modelos estelares.

Antes de executar o programa, copie sua própria pasta `massas/` para a raiz do projeto.

A estrutura esperada é:

```
massas/
└── massa_solar/
    └── massa_hidrogenio/
        ├── parametros.dat
        ├── *.sal
        └── *.sal2
```

---

# Compilação

```bash
gcc codigo.c -o calcular_estrelas -lm
```

---

# Execução

```bash
./calcular_estrelas
```

Ao final da execução será criada automaticamente a pasta `Resultados/`.

---

# Funcionamento

Para cada estrela presente em `estrelas.txt`, o programa:

1. Lê os dados observacionais.
2. Percorre toda a grade de modelos.
3. Aplica os filtros de χ², Teff e massa (quando definidos).
4. Calcula o χ² para cada modelo.
5. Ordena os modelos pelo melhor ajuste.
6. Salva automaticamente os resultados.

---

# Arquivo de Entrada (`estrelas.txt`)

O arquivo pode conter **uma ou várias estrelas**.

Cada estrela é descrita por um bloco.

Quando um bloco termina, basta iniciar imediatamente o bloco da próxima estrela.

Estrutura geral:

```text
Nome da estrela
Valor de χ
Intervalo de Teff
Intervalo de massa
Período  Amplitude  l
Período  Amplitude  l
...

Nome da próxima estrela
...
```

---

## Linha 1 — Nome da estrela

Exemplo

```text
G117-B15A
```

---

## Linha 2 — Valor de χ

Valor utilizado durante o ajuste.

Pode ser inteiro ou decimal.

Exemplos válidos

```text
0
1
1.5
2
2.5
3
3.5
4
```

---

## Linha 3 — Temperatura Efetiva (Teff)

Sem filtro

```text
0
```

Com intervalo

```text
10985 12985
```

O programa considera apenas modelos cujo Teff esteja dentro desse intervalo.

---

## Linha 4 — Massa

Sem filtro

```text
0
```

Com intervalo

```text
0.493 0.693
```

O programa considera apenas modelos cuja massa esteja dentro do intervalo informado.

---

## Linha 5 em diante — Modos observados

Formato:

```text
Período   Amplitude   l
```

Exemplo

```text
215.20 17.36 1
270.46  6.14 1
304.05  7.48 1
```

### Coluna l

| Valor | Significado |
|------:|-------------|
| 0 | O programa testa automaticamente l=1 e l=2 |
| 1 | Modo fixado em l=1 |
| 2 | Modo fixado em l=2 |

---

# Exemplo com duas estrelas

```text
G117-B15A
3
10985 12985
0.493 0.693
215.20 17.36 1
270.46  6.14 1
304.05  7.48 1

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

O programa processa primeiro **G117-B15A** e, em seguida, continua automaticamente com **L19-2**.

Não há limite para a quantidade de estrelas presentes no arquivo.

---

# Arquivos Gerados

Após a execução será criada automaticamente a pasta:

```text
Resultados/
```

contendo os arquivos de saída produzidos pelo programa.

O script `ordenarResultadoFinal.sh` organiza automaticamente os resultados finais.

---

# Observações

- A pasta `massas/` deve permanecer na raiz do projeto.
- O arquivo `parametros.dat` deve existir em cada diretório de modelos.
- Os arquivos `.sal` e `.sal2` devem estar presentes na estrutura da grade.
- Utilize sempre arquivos de texto simples (`UTF-8`).

