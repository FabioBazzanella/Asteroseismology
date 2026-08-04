# Arquivo de entrada (`estrelas.txt`)

O arquivo `estrelas.txt` contém uma ou mais estrelas que serão processadas em sequência.

Cada estrela é descrita por um **bloco** de informações. Após o último período observado de uma estrela, basta iniciar imediatamente o bloco da próxima estrela.

A estrutura geral é:

```
Nome da estrela
Valor de χ
Intervalo de Teff
Intervalo de massa
Período  Amplitude  l
Período  Amplitude  l
Período  Amplitude  l
...

Nome da próxima estrela
Valor de χ
...
```

Cada bloco possui a seguinte estrutura.

| Linha | Conteúdo |
|------:|----------|
| 1 | Nome da estrela |
| 2 | Valor de χ |
| 3 | Intervalo de temperatura efetiva (Teff) |
| 4 | Intervalo de massa |
| 5 em diante | Modos observados |

---

## Nome da estrela

Pode ser qualquer identificação.

Exemplo:

```
G117-B15A
```

---

## Valor de χ

Valor utilizado no cálculo do ajuste.

Pode ser inteiro ou decimal.

Exemplos válidos:

```
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

## Intervalo de Teff

Existem duas formas de informar a temperatura efetiva.

### Sem restrição

```
0
```

O programa considera toda a grade de temperaturas.

### Com restrição

```
10985 12985
```

O programa analisará apenas modelos com

```
10985 ≤ Teff ≤ 12985
```

---

## Intervalo de massa

Também pode ser informado de duas maneiras.

### Sem restrição

```
0
```

### Com restrição

```
0.493 0.693
```

O programa analisará apenas modelos cuja massa esteja dentro desse intervalo.

---

## Modos observados

Cada linha representa um modo observado e possui três colunas:

```
Período    Amplitude    l
```

Exemplo:

```
215.20   17.36   1
270.46    6.14   1
304.05    7.48   1
```

### Coluna `l`

| Valor | Significado |
|------:|-------------|
| 0 | O programa testa automaticamente entre l = 1 e l = 2 |
| 1 | Modo fixado em l = 1 |
| 2 | Modo fixado em l = 2 |

---

# Exemplo com duas estrelas

```
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

No exemplo acima, o programa processará primeiro **G117-B15A** e, após finalizar esse bloco, continuará automaticamente com **L19-2**.

É possível adicionar quantas estrelas forem desejadas seguindo exatamente esse mesmo formato.
