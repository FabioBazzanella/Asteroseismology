# 🌟 Ajuste de Modelos Estelares Pulsantes

> Ferramenta otimizada em C para análise e ajuste de modelos teóricos de estrelas pulsantes, comparando períodos observados com grades de modelos estelares.

## 🔭 Visão Geral

Este programa realiza o ajuste de modelos estelares comparando os períodos observacionais de pulsação com os períodos teóricos provenientes de uma extensa grade de simulações. 

Para cada estrela definida no arquivo de entrada `estrelas.txt`, o algoritmo executa o seguinte fluxo:
1. Lê os parâmetros observacionais e os limites de filtragem definidos pelo usuário.
2. Realiza uma busca recursiva por arquivos de parâmetros (`parametros.dat`) na estrutura de diretórios `massas/`.
3. Identifica automaticamente os arquivos de dados dos modelos teóricos correspondentes (`.sal` e `.sal2`).
4. Associa cada período observado ao período teórico mais próximo calculado pelo modelo.
5. Calcula métricas estatísticas de qualidade de ajuste: $\chi^2$, $\chi_{\text{Kepler}}$, $\sqrt{\chi_{\text{Kepler}}}$ e $\chi_{\text{bk}}$.
6. Filtra os modelos que satisfazem os critérios restritivos de temperatura efetiva ($T_{\text{eff}}$), massa e o limite da função de mérito.
7. Gera e estrutura automaticamente os arquivos de saída na pasta `Resultados/` e aciona um script de ordenação.

## 📂 Estrutura do Projeto

```text
.
├── codigo.c                 # Código-fonte principal em C (transformado em C, por uma IA, a partir do codigo escrito em python)
├── codigo.py                # Script original
├── estrelas.txt             # Arquivo de configuração de entrada
├── ordenarResultadoFinal.sh # Script de pós-processamento (ordenação)
└── massas/                  # Diretório com a grade de modelos (Não incluso no repositório)
```

⚠️ **Importante:** A pasta `massas/` deve ser fornecida na raiz do projeto. O código espera a seguinte hierarquia interna:
```text
massas/
└── massa_solar/
    └── massa_hidrogenio/
        ├── parametros.dat
        ├── modelo001.sal
        └── modelo001.sal2
```

## ⚙️ Compilação e Execução

O código foi escrito em C (padrão POSIX 2008) e requer o compilador `gcc`. 

**1. Compilação:**
```bash
gcc codigo.c -o calcular_estrelas -lm
```

**2. Execução:**
```bash
./calcular_estrelas
```
*Nota: Após o término da rotina em C, o script bash `ordenarResultadoFinal.sh` será invocado automaticamente.*

## 📝 Arquivo de Entrada (`estrelas.txt`)

O arquivo suporta a entrada de múltiplas estrelas simultaneamente. Cada estrela ocupa um bloco de texto independente, que deve ser separado por **uma linha em branco** (linhas iniciadas com `#` são ignoradas e servem como comentários).

### Estrutura do Bloco
* **Linha 1:** Nome da estrela
* **Linha 2:** Valor limite (corte) da métrica de ajuste ($\chi^2$)
* **Linha 3:** Filtro de Temperatura Efetiva ($T_{\text{eff}}$). Use `0` para desativar ou forneça um intervalo (ex: `10985 12985`).
* **Linha 4:** Filtro de Massa Estelar. Use `0` para desativar ou forneça um intervalo (ex: `0.493 0.693`).
* **Linhas seguintes:** Dados dos modos de pulsação no formato: `Período Amplitude l`

#### Tabela de Índices `l` (Modos)
| Valor de `l` | Comportamento do Algoritmo |
|:---:|---|
| `0` | Automático: utiliza o modelo com a menor diferença absoluta entre `.sal` e `.sal2` |
| `1` | Força a utilização exclusiva dos dados do arquivo `.sal` |
| `2` | Força a utilização exclusiva dos dados do arquivo `.sal2` |

### Exemplo de Configuração
```text
# Exemplo Estrela 1
G117-B15A
3
10985 12985
0.493 0.693
215.20 17.36 1
270.46 6.14  1
304.05 7.48  1

# Exemplo Estrela 2
L19-2
1
11105 13105
0.605 0.805
113.80 2.48  0
118.70 1.20  0
143.60 0.60  0
192.60 6.50  0
350.10 1.10  0
```

## 📊 Saídas e Resultados

O sistema estrutura os resultados aprovados de forma categorizada. Para cada estrela processada, é criado um subdiretório específico:
```text
Resultados/
└── Nome_da_Estrela/
    ├── TESS_Nome_da_Estrela_massa1.sal
    └── ...
```
Modelos que passarem nos critérios de $T_{\text{eff}}$, massa e no limite de $\chi^2$ terão seus parâmetros físicos e estatísticos exportados nestes arquivos.
