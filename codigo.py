import subprocess
import numpy as np
from pathlib import Path

# Limpa resíduos antigos de todas as estrelas
#subprocess.run("rm -f saida_*.txt", shell=True)
arquivoDeEntrada = "estrelas.txt"
raiz = Path("massas")
inicio = 4 

# =========================================================
# 1. FUNÇÃO GERADORA (Lê o arquivo sem lotar a memória RAM)
# =========================================================
def ler_estrelas_em_lote(caminho_arquivo):
    estrela_atual = {}
    dados_temporarios = []
    estado = "NOME"

    with open(caminho_arquivo, "r") as f:
        for linha in f:
            linha = linha.strip()
            
            if not linha or linha.startswith("#"):
                if estrela_atual and dados_temporarios:
                    estrela_atual["dados"] = np.array(dados_temporarios)
                    yield estrela_atual 
                    estrela_atual = {}
                    dados_temporarios = []
                    estado = "NOME"
                continue

            if estado == "NOME":
                estrela_atual["nome"] = linha
                estado = "CHI"
            elif estado == "CHI":
                estrela_atual["chi"] = float(linha)
                estado = "TEFF"
            elif estado == "TEFF":
                valores = linha.split()
                if len(valores) == 2:
                    estrela_atual["teff_min"] = float(valores[0])
                    estrela_atual["teff_max"] = float(valores[1])
                elif len(valores) == 1:
                    estrela_atual["teff_min"] = float(0)
                    estrela_atual["teff_max"] = float("inf")
                else:
                    print(f"Erro -TEFF- em {arquivoDeEntrada}")
                estado = "MASSA"
            elif estado == "MASSA":
                valores = linha.split()
                if len(valores) == 2:
                    estrela_atual["massa_min"] = float(valores[0])
                    estrela_atual["massa_max"] = float(valores[1])
                elif len(valores) == 1:
                    estrela_atual["massa_min"] = float(0)
                    estrela_atual["massa_max"] = float("inf")
                else:
                    print(f"Erro -MASSA- em {arquivoDeEntrada}")
                estado = "DADOS"
            elif estado == "DADOS":
                dados_temporarios.append([float(x) for x in linha.split()])

        if estrela_atual and dados_temporarios:
            estrela_atual["dados"] = np.array(dados_temporarios)
            yield estrela_atual

# =========================================================
# 2. LOOP PRINCIPAL (Processa cada estrela entregue)
# =========================================================

# Checagem de segurança para garantir que a pasta massas existe
if not raiz.exists() or not raiz.is_dir():
    print(f"\n[ERRO CRÍTICO] A pasta raiz '{raiz}' não foi encontrada neste diretório!")
    exit()

for estrela in ler_estrelas_em_lote(arquivoDeEntrada):
    nome_estrela = estrela["nome"].replace(" ", "_")
    print(f"\n==================================================")
    print(f"Iniciando cálculos para: {nome_estrela}")

    subprocess.run(f"rm -rf Resultados/{nome_estrela}" , shell=True)
    Path(f"Resultados/{nome_estrela}").mkdir(parents=True, exist_ok=True)
    dados = estrela["dados"]
    periodo = dados[:, 0]
    amplitude = dados[:, 1]
    indice_sal = dados[:, 2]
    
    qtd_obs = len(periodo)
    suffixos = ["sal", "sal2"]

    cabecalho_estatico = (
        f"{'# M':>21}{'M_H':>31}{'Teff':>21}{'modelo':>11}"
        f"{'chi^2':>21}{'chi_kepler':>26}{'SQRTchi_kepler':>26}{'chi_bk':>26}"
    )
    cabecalho_periodos = "".join([f"{p:>18.8f} " for p in periodo])
    cabecalho_completo = cabecalho_estatico + cabecalho_periodos + "\n"

    arquivos_tess_abertos = {}
    nome_saida = f"saida_{nome_estrela}.txt"
    


    #with open(nome_saida, "w") as f_saida:
        #f_saida.write(cabecalho_completo)

    # CORREÇÃO CRÍTICA AQUI: Rastreia o arquivo e extrai a pasta "pai"
    for arq_param in sorted(raiz.rglob("parametros.dat")):
        pasta = arq_param.parent 

        try:
            # Lê diretamente o arquivo que o rglob encontrou
            with open(arq_param) as f:

                linha = f.readlines()

                linhaUm = linha[0]
                valoresParam = linhaUm.split() 
                # Evita erros se a linha estiver incompleta
                if len(valoresParam) < 2: 
                    continue
    
                valor_massa_H = valoresParam[1]
                if "E" not in valor_massa_H and "e" not in valor_massa_H:
                    for j in range(1, len(valor_massa_H)):
                        if valor_massa_H[j] in "+-":
                            valor_massa_H = valor_massa_H[:j] + "E" + valor_massa_H[j:]
                            break
                massa_H = float(valor_massa_H)
                massa = float(valoresParam[0])
        except Exception as e:
            print(f"Erro ao ler {arq_param}: {e}")
            continue
            

        for arquivo_base in pasta.glob("*.sal"):
            # Ignora se por acaso houver uma subpasta terminada em .sal
            if not arquivo_base.is_file():
                continue
                
            diretorios = arquivo_base.parts
            nome = arquivo_base.stem
            
            try:
                modelo = int(''.join(filter(str.isdigit, nome)))
            except ValueError:
                continue
            
            
            valoresMinimosFinal = [
                {"valor": np.zeros(qtd_obs), "indice": np.zeros(qtd_obs), "Teff": np.zeros(qtd_obs), "k": np.zeros(qtd_obs)},
                {"valor": np.zeros(qtd_obs), "indice": np.zeros(qtd_obs), "Teff": np.zeros(qtd_obs), "k": np.zeros(qtd_obs)}
            ]
            
            Teff_modelo = 0.0 
            k_modelo = 0
            for num, ext in enumerate(suffixos):
                arquivo = arquivo_base.with_suffix(f".{ext}")

                if not arquivo.exists():
                    valoresMinimosFinal[num]["valor"][:] = np.inf 
                    valoresMinimosFinal[num]["Teff"][:] = np.inf 
                    continue
                    
                periodos_lidos = []
                k_lidos = []
                with open(arquivo) as f:
                    for _ in range(inicio):
                        next(f, None)
                        
                    for linha in f:
                        valores = linha.split()
                        if len(valores) > 13:
                            try:
                                periodos_lidos.append(float(valores[4]))
                                k_lidos.append(float(valores[1]))
                                Teff_modelo = float(valores[13])

                            except ValueError:
                                pass
                if not periodos_lidos:
                    valoresMinimosFinal[num]["valor"][:] = np.inf
                    continue

                periodos_lidos = np.array(periodos_lidos)
                k_lidos = np.array(k_lidos)

                # Busca eficiente
                for x, p_obs in enumerate(periodo):
                    idx_min = np.abs(periodos_lidos - p_obs).argmin()
                    valoresMinimosFinal[num]["valor"][x] = periodos_lidos[idx_min]
                    valoresMinimosFinal[num]["indice"][x] = num + 1
                    valoresMinimosFinal[num]["Teff"][x] = Teff_modelo
                    valoresMinimosFinal[num]["k"][x] = k_lidos[idx_min]

            # Seleção dos modos
            resultado = {
                "valor": np.zeros(qtd_obs),
                "indice": np.zeros(qtd_obs),
                "Teff": np.zeros(qtd_obs),
                "k": np.zeros(qtd_obs)
            }
            
            for i, p_obs in enumerate(periodo):
                if indice_sal[i] == 0:
                    diff1 = np.abs(valoresMinimosFinal[0]["valor"][i] - p_obs)
                    diff2 = np.abs(valoresMinimosFinal[1]["valor"][i] - p_obs)
                    escolha = 0 if diff1 <= diff2 else 1
                elif indice_sal[i] == 1:
                    escolha = 0
                elif indice_sal[i] == 2:
                    escolha = 1
                else:
                    escolha = 0 

                resultado["valor"][i] = valoresMinimosFinal[escolha]["valor"][i]
                resultado["indice"][i] = valoresMinimosFinal[escolha]["indice"][i]
                resultado["Teff"][i] = valoresMinimosFinal[escolha]["Teff"][i]
                resultado["k"][i] = valoresMinimosFinal[escolha]["k"][i]
            # Cálculos Estatísticos
            esperados = resultado["valor"]
            N = qtd_obs

            diferencas_absolutas = np.abs(periodo - esperados)
            erros_quadraticos = diferencas_absolutas**2

            chi2_padrao = np.sqrt(np.sum(erros_quadraticos)) / N
            chi_bk = np.sum(diferencas_absolutas) / N
            
            # Evita divisão por zero caso a amplitude esteja vazia/zerada
            soma_amplitude = np.sum(amplitude)
            if soma_amplitude != 0:
                chi_kepler = np.sum(amplitude * erros_quadraticos) / soma_amplitude
            else:
                chi_kepler = 0.0
                
            sqrt_chi_kepler = np.sqrt(chi_kepler)

            # Gravação Otimizada
            str_valores = "".join([f"{v:>18.8f} " for v in resultado["valor"]])
            str_indices = "".join([f"{int(ind):>4} " for ind in resultado["indice"]])
            str_k = "".join([f"{int(ind):>4} " for ind in resultado["k"]])
            
            linha_saida = (
                f"{massa:>21.17f}{massa_H:>31.16E}{Teff_modelo:>21.12f}{modelo:>11d}"
                f"{chi2_padrao:>21.15f}{chi_kepler:>26.14f}{sqrt_chi_kepler:>26.15f}{chi_bk:>26.15f}"
                f"{str_valores}{str_indices}{str_k}\n"
            )

            #f_saida.write(linha_saida)
            
            # =========================================================
            # CORTE TESS (Utilizando os limites da estrela atual)
            # =========================================================
            if (chi2_padrao <= estrela["chi"]) and (estrela["teff_min"] <= Teff_modelo <= estrela["teff_max"]) and (estrela["massa_min"] <= massa <= estrela["massa_max"]):
                # Inclui o nome da estrela no arquivo para não misturar os dados
                nome_arq_tess = f"Resultados/{nome_estrela}/TESS_{nome_estrela}_{diretorios[1]}.sal"
                
                if nome_arq_tess not in arquivos_tess_abertos:
                    arquivos_tess_abertos[nome_arq_tess] = open(nome_arq_tess, "w")
                    arquivos_tess_abertos[nome_arq_tess].write(cabecalho_completo)
                    
                arquivos_tess_abertos[nome_arq_tess].write(linha_saida)

    # Fecha todos os arquivos TESS associados à estrela processada
    for f in arquivos_tess_abertos.values():
        f.close()
        

print("\nProcessamento de todas as estrelas concluído com sucesso!\n")

#LEMBRAR DE NO FINAL MUDAR ISSO PARA QUE GERE O GRAFICO PARA CADA ESTRELA SEM ESPERAR O CODIGO TERMINAR DE RODAR!!
subprocess.run("sh ordenarResultadoFinal.sh", shell=True);
