#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_LINE 8192
#define INICIO   4

static const char *ARQUIVO_DE_ENTRADA = "estrelas.txt";
static const char *RAIZ = "massas";

/* ============================================================
 *  Estruturas auxiliares
 * ============================================================ */

typedef struct {
    char nome[512];
    double chi;
    double teff_min, teff_max;
    double massa_min, massa_max;
    double *periodo;
    double *amplitude;
    double *indice_sal;
    int qtd_obs;
} Estrela;

typedef struct {
    char nome_arquivo[2048];
    FILE *f;
} ArquivoAberto;

typedef struct {
    char **items;
    int count;
    int capacity;
} StrList;

static void strlist_init(StrList *l) {
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

static void strlist_add(StrList *l, const char *s) {
    if (l->count >= l->capacity) {
        l->capacity = (l->capacity == 0) ? 16 : l->capacity * 2;
        l->items = realloc(l->items, l->capacity * sizeof(char *));
    }
    l->items[l->count] = strdup(s);
    l->count++;
}

static void strlist_free(StrList *l) {
    for (int i = 0; i < l->count; i++) free(l->items[i]);
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* Busca recursiva por arquivos "parametros.dat" (equivalente a raiz.rglob) */
static void encontrar_parametros(const char *dir_path, StrList *out) {
    DIR *d = opendir(dir_path);
    if (!d) return;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            encontrar_parametros(full_path, out);
        } else if (S_ISREG(st.st_mode)) {
            if (strcmp(entry->d_name, "parametros.dat") == 0)
                strlist_add(out, full_path);
        }
    }
    closedir(d);
}

/* remove espaços/whitespace nas duas pontas, na própria string */
static char *strip(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' ||
                        s[len - 1] == ' '  || s[len - 1] == '\t')) {
        s[--len] = '\0';
    }
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

/* ============================================================
 *  1. LEITURA EM LOTE DO ARQUIVO DE ESTRELAS
 *     (equivalente à função geradora ler_estrelas_em_lote)
 * ============================================================ */

static int ler_proxima_estrela(FILE *f, Estrela *out) {
    char linha_bruta[MAX_LINE];
    int estado = 0; /* 0=NOME, 1=CHI, 2=TEFF, 3=MASSA, 4=DADOS */
    int tem_nome = 0;

    double *periodo = NULL, *amplitude = NULL, *indice_sal = NULL;
    int cap = 0, n = 0;

    memset(out, 0, sizeof(Estrela));

    while (fgets(linha_bruta, sizeof(linha_bruta), f)) {
        char *linha = strip(linha_bruta);

        if (linha[0] == '\0' || linha[0] == '#') {
            if (tem_nome && n > 0) {
                out->periodo = periodo;
                out->amplitude = amplitude;
                out->indice_sal = indice_sal;
                out->qtd_obs = n;
                return 1;
            }
            memset(out, 0, sizeof(Estrela));
            free(periodo); free(amplitude); free(indice_sal);
            periodo = amplitude = indice_sal = NULL;
            cap = 0; n = 0;
            tem_nome = 0;
            estado = 0;
            continue;
        }

        if (estado == 0) {
            strncpy(out->nome, linha, sizeof(out->nome) - 1);
            tem_nome = 1;
            estado = 1;
        } else if (estado == 1) {
            out->chi = atof(linha);
            estado = 2;
        } else if (estado == 2) {
            char tmp[MAX_LINE];
            strcpy(tmp, linha);
            char *t1 = strtok(tmp, " \t");
            char *t2 = t1 ? strtok(NULL, " \t") : NULL;
            char *t3 = t2 ? strtok(NULL, " \t") : NULL;
            if (t1 && t2 && !t3) {
                out->teff_min = atof(t1);
                out->teff_max = atof(t2);
            } else if (t1 && !t2) {
                out->teff_min = 0.0;
                out->teff_max = INFINITY;
            } else {
                printf("Erro -TEFF- em %s\n", ARQUIVO_DE_ENTRADA);
            }
            estado = 3;
        } else if (estado == 3) {
            char tmp[MAX_LINE];
            strcpy(tmp, linha);
            char *t1 = strtok(tmp, " \t");
            char *t2 = t1 ? strtok(NULL, " \t") : NULL;
            char *t3 = t2 ? strtok(NULL, " \t") : NULL;
            if (t1 && t2 && !t3) {
                out->massa_min = atof(t1);
                out->massa_max = atof(t2);
            } else if (t1 && !t2) {
                out->massa_min = 0.0;
                out->massa_max = INFINITY;
            } else {
                printf("Erro -MASSA- em %s\n", ARQUIVO_DE_ENTRADA);
            }
            estado = 4;
        } else if (estado == 4) {
            if (n >= cap) {
                cap = (cap == 0) ? 16 : cap * 2;
                periodo    = realloc(periodo,    cap * sizeof(double));
                amplitude  = realloc(amplitude,  cap * sizeof(double));
                indice_sal = realloc(indice_sal, cap * sizeof(double));
            }
            double vals[3] = {0.0, 0.0, 0.0};
            char tmp[MAX_LINE];
            strcpy(tmp, linha);
            char *tok = strtok(tmp, " \t");
            int idx = 0;
            while (tok && idx < 3) {
                vals[idx++] = atof(tok);
                tok = strtok(NULL, " \t");
            }
            periodo[n]    = vals[0];
            amplitude[n]  = vals[1];
            indice_sal[n] = vals[2];
            n++;
        }
    }

    if (tem_nome && n > 0) {
        out->periodo = periodo;
        out->amplitude = amplitude;
        out->indice_sal = indice_sal;
        out->qtd_obs = n;
        return 1;
    }

    free(periodo); free(amplitude); free(indice_sal);
    return 0;
}

/* ============================================================
 *  Lista dinâmica de arquivos TESS já abertos
 * ============================================================ */

typedef struct {
    ArquivoAberto *items;
    int count;
    int capacity;
} ArquivoList;

static void arqlist_init(ArquivoList *l) {
    l->items = NULL; l->count = 0; l->capacity = 0;
}

static FILE *arqlist_get_or_open(ArquivoList *l, const char *nome, const char *cabecalho) {
    for (int i = 0; i < l->count; i++) {
        if (strcmp(l->items[i].nome_arquivo, nome) == 0)
            return l->items[i].f;
    }
    if (l->count >= l->capacity) {
        l->capacity = (l->capacity == 0) ? 16 : l->capacity * 2;
        l->items = realloc(l->items, l->capacity * sizeof(ArquivoAberto));
    }
    FILE *novo = fopen(nome, "w");
    if (!novo) return NULL;
    fputs(cabecalho, novo);
    strncpy(l->items[l->count].nome_arquivo, nome, sizeof(l->items[l->count].nome_arquivo) - 1);
    l->items[l->count].nome_arquivo[sizeof(l->items[l->count].nome_arquivo) - 1] = '\0';
    l->items[l->count].f = novo;
    l->count++;
    return novo;
}

static void arqlist_close_all(ArquivoList *l) {
    for (int i = 0; i < l->count; i++) {
        if (l->items[i].f) fclose(l->items[i].f);
    }
    free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

/* ============================================================
 *  2. LOOP PRINCIPAL
 * ============================================================ */

int main(void) {
    struct stat st;

    /* Checagem de segurança para garantir que a pasta massas existe */
    if (stat(RAIZ, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("\n[ERRO CRITICO] A pasta raiz '%s' nao foi encontrada neste diretorio!\n", RAIZ);
        exit(0);
    }

    FILE *f_entrada = fopen(ARQUIVO_DE_ENTRADA, "r");
    if (!f_entrada) {
        fprintf(stderr, "Nao foi possivel abrir %s: %s\n", ARQUIVO_DE_ENTRADA, strerror(errno));
        return 1;
    }

    Estrela estrela;
    while (ler_proxima_estrela(f_entrada, &estrela)) {

        char nome_estrela[512];
        strncpy(nome_estrela, estrela.nome, sizeof(nome_estrela) - 1);
        nome_estrela[sizeof(nome_estrela) - 1] = '\0';
        for (char *p = nome_estrela; *p; p++) if (*p == ' ') *p = '_';

        printf("\n==================================================\n");
        printf("Iniciando calculos para: %s\n", nome_estrela);

        char cmd[1600];
        snprintf(cmd, sizeof(cmd), "rm -rf Resultados/%s", nome_estrela);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "mkdir -p Resultados/%s", nome_estrela);
        system(cmd);

        int qtd_obs = estrela.qtd_obs;
        double *periodo    = estrela.periodo;
        double *amplitude  = estrela.amplitude;
        double *indice_sal = estrela.indice_sal;

        const char *suffixos[2] = {"sal", "sal2"};

        char cabecalho_estatico[256];
        snprintf(cabecalho_estatico, sizeof(cabecalho_estatico),
                 "%21s%31s%21s%11s%21s%26s%26s%26s",
                 "# M", "M_H", "Teff", "modelo",
                 "chi^2", "chi_kepler", "SQRTchi_kepler", "chi_bk");

        char *cabecalho_periodos = malloc((size_t)qtd_obs * 40 + 16);
        cabecalho_periodos[0] = '\0';
        for (int i = 0; i < qtd_obs; i++) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%18.8f ", periodo[i]);
            strcat(cabecalho_periodos, buf);
        }

        char *cabecalho_completo = malloc(strlen(cabecalho_estatico) + strlen(cabecalho_periodos) + 2);
        sprintf(cabecalho_completo, "%s%s\n", cabecalho_estatico, cabecalho_periodos);

        ArquivoList arquivos_tess_abertos;
        arqlist_init(&arquivos_tess_abertos);

        /* rastreia parametros.dat de forma recursiva e ordena (equivalente a sorted(raiz.rglob(...))) */
        StrList params;
        strlist_init(&params);
        encontrar_parametros(RAIZ, &params);
        qsort(params.items, params.count, sizeof(char *), cmp_str);

        for (int pi = 0; pi < params.count; pi++) {
            char *arq_param = params.items[pi];

            char pasta[1600];
            strncpy(pasta, arq_param, sizeof(pasta) - 1);
            pasta[sizeof(pasta) - 1] = '\0';
            char *last_slash = strrchr(pasta, '/');
            if (last_slash) *last_slash = '\0';

            double massa = 0.0, massa_H = 0.0;
            {
                FILE *fp = fopen(arq_param, "r");
                if (!fp) {
                    printf("Erro ao ler %s: %s\n", arq_param, strerror(errno));
                    continue;
                }
                char linha1[MAX_LINE];
                if (!fgets(linha1, sizeof(linha1), fp)) {
                    fclose(fp);
                    printf("Erro ao ler %s: arquivo vazio\n", arq_param);
                    continue;
                }
                fclose(fp);
                char *l = strip(linha1);

                char tmp[MAX_LINE];
                strcpy(tmp, l);
                char *tok0 = strtok(tmp, " \t");
                char *tok1 = tok0 ? strtok(NULL, " \t") : NULL;
                if (!tok0 || !tok1) continue; /* len(valoresParam) < 2 */

                char valor_massa_H[256];
                strncpy(valor_massa_H, tok1, sizeof(valor_massa_H) - 1);
                valor_massa_H[sizeof(valor_massa_H) - 1] = '\0';

                if (!strchr(valor_massa_H, 'E') && !strchr(valor_massa_H, 'e')) {
                    int lv = (int)strlen(valor_massa_H);
                    for (int j = 1; j < lv; j++) {
                        if (valor_massa_H[j] == '+' || valor_massa_H[j] == '-') {
                            char novo[300];
                            memcpy(novo, valor_massa_H, j);
                            novo[j] = '\0';
                            strcat(novo, "E");
                            strcat(novo, valor_massa_H + j);
                            strcpy(valor_massa_H, novo);
                            break;
                        }
                    }
                }
                massa_H = atof(valor_massa_H);
                massa   = atof(tok0);
            }

            /* pasta.glob("*.sal") -- apenas arquivos diretamente dentro de "pasta" */
            StrList sal_files;
            strlist_init(&sal_files);
            DIR *d = opendir(pasta);
            if (d) {
                struct dirent *entry;
                while ((entry = readdir(d)) != NULL) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                        continue;
                    size_t nl = strlen(entry->d_name);
                    if (nl > 4 && strcmp(entry->d_name + nl - 4, ".sal") == 0) {
                        char fullp[1800];
                        snprintf(fullp, sizeof(fullp), "%s/%s", pasta, entry->d_name);
                        struct stat st2;
                        if (stat(fullp, &st2) == 0 && S_ISREG(st2.st_mode))
                            strlist_add(&sal_files, fullp);
                    }
                }
                closedir(d);
            }

            for (int si = 0; si < sal_files.count; si++) {
                char *arquivo_base = sal_files.items[si];

                /* diretorios = arquivo_base.parts */
                char partes_buf[1800];
                strncpy(partes_buf, arquivo_base, sizeof(partes_buf) - 1);
                partes_buf[sizeof(partes_buf) - 1] = '\0';
                char *partes[64];
                int n_partes = 0;
                char *tok = strtok(partes_buf, "/");
                while (tok && n_partes < 64) { partes[n_partes++] = tok; tok = strtok(NULL, "/"); }
                const char *diretorio_1 = (n_partes > 1) ? partes[1] : "";

                char *slash = strrchr(arquivo_base, '/');
                char *fname = slash ? slash + 1 : arquivo_base;
                char nome_stem[600];
                strncpy(nome_stem, fname, sizeof(nome_stem) - 1);
                nome_stem[sizeof(nome_stem) - 1] = '\0';
                char *dot = strrchr(nome_stem, '.');
                if (dot) *dot = '\0';

                char digitos[600];
                int di = 0;
                for (char *c = nome_stem; *c; c++)
                    if (isdigit((unsigned char)*c)) digitos[di++] = *c;
                digitos[di] = '\0';
                if (di == 0) continue; /* ValueError -> continue */
                int modelo = atoi(digitos);

                double val_valor[2][qtd_obs > 0 ? qtd_obs : 1];
                double val_indice[2][qtd_obs > 0 ? qtd_obs : 1];
                double val_teff[2][qtd_obs > 0 ? qtd_obs : 1];
                double val_k[2][qtd_obs > 0 ? qtd_obs : 1];
                memset(val_valor, 0, sizeof(val_valor));
                memset(val_indice, 0, sizeof(val_indice));
                memset(val_teff, 0, sizeof(val_teff));
                memset(val_k, 0, sizeof(val_k));

                double Teff_modelo = 0.0;

                for (int num = 0; num < 2; num++) {
                    char base_noext[1800];
                    strncpy(base_noext, arquivo_base, sizeof(base_noext) - 1);
                    base_noext[sizeof(base_noext) - 1] = '\0';
                    char *d2 = strrchr(base_noext, '.');
                    if (d2) *d2 = '\0';

                    char arquivo[1900];
                    snprintf(arquivo, sizeof(arquivo), "%s.%s", base_noext, suffixos[num]);

                    FILE *fa = fopen(arquivo, "r");
                    if (!fa) {
                        for (int x = 0; x < qtd_obs; x++) {
                            val_valor[num][x] = INFINITY;
                            val_teff[num][x]  = INFINITY;
                        }
                        continue;
                    }

                    char linha[MAX_LINE];
                    for (int sk = 0; sk < INICIO; sk++) {
                        if (!fgets(linha, sizeof(linha), fa)) break;
                    }

                    double *periodos_lidos = NULL, *k_lidos = NULL;
                    int cap2 = 0, n2 = 0;

                    while (fgets(linha, sizeof(linha), fa)) {
                        char tmp[MAX_LINE];
                        strcpy(tmp, linha);
                        char *campos[64];
                        int nc = 0;
                        char *t = strtok(tmp, " \t\r\n");
                        while (t && nc < 64) { campos[nc++] = t; t = strtok(NULL, " \t\r\n"); }

                        if (nc > 13) {
                            char *end4, *end1, *end13;
                            double v4  = strtod(campos[4], &end4);
                            double v1  = strtod(campos[1], &end1);
                            double v13 = strtod(campos[13], &end13);
                            if (end4 != campos[4] && end1 != campos[1] && end13 != campos[13]) {
                                if (n2 >= cap2) {
                                    cap2 = (cap2 == 0) ? 16 : cap2 * 2;
                                    periodos_lidos = realloc(periodos_lidos, cap2 * sizeof(double));
                                    k_lidos        = realloc(k_lidos, cap2 * sizeof(double));
                                }
                                periodos_lidos[n2] = v4;
                                k_lidos[n2] = v1;
                                Teff_modelo = v13;
                                n2++;
                            }
                        }
                    }
                    fclose(fa);

                    if (n2 == 0) {
                        for (int x = 0; x < qtd_obs; x++) val_valor[num][x] = INFINITY;
                        free(periodos_lidos); free(k_lidos);
                        continue;
                    }

                    for (int x = 0; x < qtd_obs; x++) {
                        double p_obs = periodo[x];
                        int idx_min = 0;
                        double melhor = fabs(periodos_lidos[0] - p_obs);
                        for (int k = 1; k < n2; k++) {
                            double diff = fabs(periodos_lidos[k] - p_obs);
                            if (diff < melhor) { melhor = diff; idx_min = k; }
                        }
                        val_valor[num][x]  = periodos_lidos[idx_min];
                        val_indice[num][x] = num + 1;
                        val_teff[num][x]   = Teff_modelo;
                        val_k[num][x]      = k_lidos[idx_min];
                    }

                    free(periodos_lidos);
                    free(k_lidos);
                }

                /* Seleção dos modos */
                double *resultado_valor  = malloc(qtd_obs * sizeof(double));
                double *resultado_indice = malloc(qtd_obs * sizeof(double));
                double *resultado_teff   = malloc(qtd_obs * sizeof(double));
                double *resultado_k      = malloc(qtd_obs * sizeof(double));

                for (int i = 0; i < qtd_obs; i++) {
                    double p_obs = periodo[i];
                    int escolha;
                    if (indice_sal[i] == 0) {
                        double diff1 = fabs(val_valor[0][i] - p_obs);
                        double diff2 = fabs(val_valor[1][i] - p_obs);
                        escolha = (diff1 <= diff2) ? 0 : 1;
                    } else if (indice_sal[i] == 1) {
                        escolha = 0;
                    } else if (indice_sal[i] == 2) {
                        escolha = 1;
                    } else {
                        escolha = 0;
                    }
                    resultado_valor[i]  = val_valor[escolha][i];
                    resultado_indice[i] = val_indice[escolha][i];
                    resultado_teff[i]   = val_teff[escolha][i];
                    resultado_k[i]      = val_k[escolha][i];
                }

                /* Cálculos Estatísticos */
                int N = qtd_obs;
                double soma_erros_quadraticos = 0.0;
                double soma_diferencas_absolutas = 0.0;
                double soma_amplitude = 0.0;
                double soma_amp_erro = 0.0;

                for (int i = 0; i < N; i++) {
                    double diff_abs = fabs(periodo[i] - resultado_valor[i]);
                    double erro_quad = diff_abs * diff_abs;
                    soma_diferencas_absolutas += diff_abs;
                    soma_erros_quadraticos += erro_quad;
                    soma_amplitude += amplitude[i];
                    soma_amp_erro += amplitude[i] * erro_quad;
                }

                double chi2_padrao = sqrt(soma_erros_quadraticos) / N;
                double chi_bk = soma_diferencas_absolutas / N;
                double chi_kepler;
                if (soma_amplitude != 0.0) {
                    chi_kepler = soma_amp_erro / soma_amplitude;
                } else {
                    chi_kepler = 0.0;
                }
                double sqrt_chi_kepler = sqrt(chi_kepler);

                /* Gravação Otimizada */
                char *str_valores  = malloc((size_t)N * 40 + 16);
                char *str_indices  = malloc((size_t)N * 16 + 16);
                char *str_k        = malloc((size_t)N * 16 + 16);
                str_valores[0] = str_indices[0] = str_k[0] = '\0';

                for (int i = 0; i < N; i++) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%18.8f ", resultado_valor[i]);
                    strcat(str_valores, buf);
                    snprintf(buf, sizeof(buf), "%4d ", (int)resultado_indice[i]);
                    strcat(str_indices, buf);
                    snprintf(buf, sizeof(buf), "%4d ", (int)resultado_k[i]);
                    strcat(str_k, buf);
                }

                size_t tam_linha = strlen(str_valores) + strlen(str_indices) + strlen(str_k) + 256;
                char *linha_saida = malloc(tam_linha);
                snprintf(linha_saida, tam_linha,
                         "%21.17f%31.16E%21.12f%11d%21.15f%26.14f%26.15f%26.15f%s%s%s\n",
                         massa, massa_H, Teff_modelo, modelo,
                         chi2_padrao, chi_kepler, sqrt_chi_kepler, chi_bk,
                         str_valores, str_indices, str_k);

                /* CORTE TESS (Utilizando os limites da estrela atual) */
                if ((chi2_padrao <= estrela.chi) &&
                    (estrela.teff_min <= Teff_modelo && Teff_modelo <= estrela.teff_max) &&
                    (estrela.massa_min <= massa && massa <= estrela.massa_max)) {

                    char nome_arq_tess[2048];
                    snprintf(nome_arq_tess, sizeof(nome_arq_tess),
                             "Resultados/%s/TESS_%s_%s.sal", nome_estrela, nome_estrela, diretorio_1);

                    FILE *ft = arqlist_get_or_open(&arquivos_tess_abertos, nome_arq_tess, cabecalho_completo);
                    if (ft) fputs(linha_saida, ft);
                }

                free(resultado_valor);
                free(resultado_indice);
                free(resultado_teff);
                free(resultado_k);
                free(str_valores);
                free(str_indices);
                free(str_k);
                free(linha_saida);
            }

            strlist_free(&sal_files);
        }

        strlist_free(&params);

        /* Fecha todos os arquivos TESS associados à estrela processada */
        arqlist_close_all(&arquivos_tess_abertos);

        free(cabecalho_periodos);
        free(cabecalho_completo);
        free(estrela.periodo);
        free(estrela.amplitude);
        free(estrela.indice_sal);
    }

    fclose(f_entrada);

    printf("\nProcessamento de todas as estrelas concluido com sucesso!\n\n");

    system("sh ordenarResultadoFinal.sh");

    return 0;
}
