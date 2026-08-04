# Ajuste de Modelos Estelares

Ferramenta para ajuste de modelos estelares pulsantes através da comparação entre períodos observados e períodos teóricos utilizando o método do **chi-quadrado**.

O projeto percorre uma grade de modelos estelares, calcula o valor de χ² para cada modelo e retorna os melhores candidatos, juntamente com seus parâmetros físicos (Teff, massa, espessura da camada de hidrogênio e demais parâmetros sísmicos).

---

# Estrutura do projeto

```
.
├── codigo.c
├── codigo.py
├── estrelas.txt
├── ordenarResultadoFinal.sh
└── massas/
```

A pasta **`massas/`** **não acompanha este repositório**, pois contém vários gigabytes de modelos estelares.

Você deve copiar sua própria grade de modelos para essa pasta.

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

# Instalação

Clone ou baixe este repositório.

Depois copie sua pasta `massas/` para a raiz do projeto.

A estrutura final deve ficar assim:

```
projeto/
├── codigo.c
├── codigo.py
├── estrelas.txt
├── ordenarResultadoFinal.sh
└── massas/
```

---

# Compilação

Compile o código em C:

```bash
gcc codigo.c -o calcular_estrelas -lm
```

O parâmetro `-lm` é necessário para utilizar a biblioteca matemática.

---

# Execução

Execute o programa com:

```bash
./calcular_estrelas
```

Ao final da execução será criada automaticamente a pasta

```
Resultados/
```

contendo todos os arquivos gerados.

O script

```
ordenarResultadoFinal.sh
```

é executado automaticamente para organizar os resultados finais.

---

# Arquivo de entrada (`estrelas.txt`)

O arquivo `estrelas.txt` define os parâmetros da estrela observada e os períodos utilizados no ajuste.

Sua estrutura deve seguir exatamente a ordem abaixo.

| Linha | Conteúdo |
|-------|----------|
| 1 | Nome da estrela |
| 2 | Valor do χ |
| 3 | Intervalo de Teff |
| 4 | Intervalo de massa |
| 5 em diante | Períodos observados |

---

## Linha 1

Nome da estrela.

Exemplo:

```
G117-B15A
```

---

## Linha 2

Valor do χ utilizado no ajuste.

Exemplo:

```
3
```

---

## Linha 3 – Temperatura efetiva (Teff)

Pode assumir dois formatos.

Sem restrição:

```
0
```

Intervalo:

```
11000 12000
```

---

## Linha 4 – Massa

Pode assumir dois formatos.

Sem restrição:

```
0
```

Intervalo:

```
0.50 0.70
```

---

## Linha 5 em diante

Cada linha representa um modo observado.

Formato:

```
Período    Amplitude    l
```

Exemplo:

```
215.20   17.36   1
270.46    6.14   1
304.05    7.48   1
```

### Coluna **l**

| Valor | Significado |
|--------|-------------|
| 0 | O programa testa automaticamente entre l=1 e l=2 |
| 1 | Modo fixado em l=1 |
| 2 | Modo fixado em l=2 |

---

# Exemplo completo

```
G117-B15A
3
0
0.50 0.70
215.20 17.36 1
270.46  6.14 1
304.05  7.48 1
```

Nesse exemplo:

- estrela: **G117-B15A**
- χ = **3**
- Teff sem restrição
- massa entre **0.50 e 0.70 M☉**
- três modos observados, todos fixados com **l = 1**

---

# Saída

Ao término da execução, o programa gera automaticamente:

- resultados do cálculo de χ²;
- melhores modelos encontrados;
- parâmetros físicos correspondentes;
- arquivos organizados na pasta `Resultados/`.
