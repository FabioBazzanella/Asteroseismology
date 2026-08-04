#!/bin/bash
CX=3
CY=8

# Pega o primeiro arquivo TESS_ global apenas para extrair os nomes dos eixos X e Y
PRIMEIRO_TESS_GLOBAL=$(ls Resultados/*/TESS_*.sal 2>/dev/null | head -n 1)

if [ -z "$PRIMEIRO_TESS_GLOBAL" ]; then
    echo "Erro: Nenhum arquivo Resultados/*/TESS_*.sal encontrado nas subpastas."
    exit 1
fi

NOME_X=$(awk -v col=$((CX+1)) 'NR==1 {print $col; exit}' "$PRIMEIRO_TESS_GLOBAL")
NOME_Y=$(awk -v col=$((CY+1)) 'NR==1 {print $col; exit}' "$PRIMEIRO_TESS_GLOBAL")

# Loop que itera sobre TODAS as pastas de estrelas DENTRO de Resultados/
for pasta in Resultados/*/; do
    [ -d "$pasta" ] || continue
    
    NOME_ESTRELA=$(basename "$pasta")
    
    # Verifica se a pasta tem arquivos TESS
    PRIMEIRO_TESS_ESTRELA=$(ls Resultados/"$NOME_ESTRELA"/TESS_*.sal 2>/dev/null | head -n 1)
    if [ -z "$PRIMEIRO_TESS_ESTRELA" ]; then
        continue 
    fi

    echo ""
    echo "__________________________________________________________________________"
    echo "\n"
    echo "Estrela: $NOME_ESTRELA"

    list=$(mktemp)
    lab=$(mktemp)
    range=$(mktemp)
    best=$(mktemp)
    
    # O AWK lê os arquivos e salva A LINHA INTEIRA do mínimo global
    awk -v cx="$CX" -v cy="$CY" -v list="$list" -v lab="$lab" -v range="$range" -v best="$best" '
    NF>=cx && NF>=cy && $cx+0==$cx && $cy+0==$cy { 
        if(!(FILENAME in seen)){
            seen[FILENAME]=1; files[++n]=FILENAME; 
            miny[FILENAME]=$cy+0; minx[FILENAME]=$cx+0
        } 
        x=$cx+0; y=$cy+0; 
        
        if(y<miny[FILENAME]){miny[FILENAME]=y; minx[FILENAME]=x} 
        
        if(!init){
            xmin=x; xmax=x; ymin=y; ymax=y; init=1;
            gmin_y=y; best_raw=$0;
        } 
        
        if(x<xmin)xmin=x; if(x>xmax)xmax=x; 
        if(y<ymin)ymin=y; if(y>ymax)ymax=y 
        
        if(y<gmin_y){
            gmin_y=y; best_raw=$0;
        }
    } 
    END{ 
        if(!init) exit 1; 
        for(i=1;i<=n;i++) ord[i]=i; 
        for(i=1;i<=n;i++) for(j=i+1;j<=n;j++) if(miny[files[ord[i]]] > miny[files[ord[j]]]) {t=ord[i];ord[i]=ord[j];ord[j]=t}; 
        dx=xmax-xmin; if(dx==0) dx=1; dy=ymax-ymin; if(dy==0) dy=1; 
        xlab=xmax+0.04*dx; xhi=xmax+0.35*dx; 
        for(k=1;k<=n;k++){ 
            f=files[ord[k]]; 
            print f >> list; 
            ylab=(n==1 ? miny[f] : ymin+(k-1)*dy/(n-1)); 
            
            num_parts = split(f, partes, "_");
            massa = partes[num_parts];
            sub(/\.sal$/, "", massa); 
            
            printf "%g %g %g %g %s\n", minx[f],miny[f],xlab,ylab,massa >> lab 
        } 
        print xhi > range
        
        # Salva a linha bruta do melhor modelo para ser desmontada no Bash
        print best_raw > best
    }' Resultados/"$NOME_ESTRELA"/TESS_*.sal 
    
    if [ ! -s "$best" ]; then
        echo "Erro: Nenhum dado válido encontrado para $NOME_ESTRELA."
        rm -f "$list" "$lab" "$range" "$best"
        continue
    fi

    # Lendo a linha inteira do melhor modelo
    BEST_LINE=$(cat "$best")
    
    # Extraindo as variáveis principais
    BEST_MASS=$(echo "$BEST_LINE" | awk '{printf "%.4f", $1}')
    BEST_MH=$(echo "$BEST_LINE" | awk '{printf "%.2E", $2}')
    BEST_X=$(echo "$BEST_LINE" | awk '{print $3}')
    BEST_MOD=$(echo "$BEST_LINE" | awk '{print $4}')
    BEST_Y=$(echo "$BEST_LINE" | awk '{print $8}')

    # ====================================================================
    # GERADOR DA TABELA DINÂMICA (EXTRAINDO l E k DINAMICAMENTE)
    # ====================================================================
    HEADER=$(head -n 1 "$PRIMEIRO_TESS_ESTRELA")
    
    echo "-> Melhor Modelo: Massa = $BEST_MASS | M_H = $BEST_MH | Modelo = $BEST_MOD | Teff = $BEST_X | Erro = $BEST_Y"
    echo ""
    echo "+-----------------+-----------------+-------+-------+-----------------+"
    printf "| %-15s | %-15s | %-5s | %-5s | %-15s |\n" "P_observado" "P_teorico (Modelo)" "l" "k" "Resíduo (s)"
    echo "+-----------------+-----------------+-------+-------+-----------------+"
    
    # O AWK calcula quantos períodos existem e onde os índices estão na linha
    awk -v hdr="$HEADER" -v best="$BEST_LINE" '
    BEGIN {
        n_obs = split(hdr, h, " ") - 9;
        split(best, b, " ");
        
        for(i=1; i<=n_obs; i++){
            p_obs = h[i+9];           # Períodos do TESS no cabeçalho
            p_teo = b[i+8];           # Períodos Teóricos
            l_val = b[i+8+n_obs];     # Índices l (salta N colunas)
            k_val = b[i+8+(2*n_obs)]; # Índices k (salta 2*N colunas)
            
            residuo = p_obs - p_teo;
            printf "| %-15s | %-18s | %-5s | %-5s | %+.4f        |\n", p_obs, p_teo, l_val, k_val, residuo;
        }
    }'
    echo "+-----------------+-----------------+-------+-------+-----------------+"
    echo ""

    # ====================================================================
    # GERAÇÃO DO GRÁFICO NO GNUPLOT
    # ====================================================================
    gnuplot <<EOF
    set term pngcairo size 1800,1000 font ',10' background '#ffffff'
    set output 'Resultados/$NOME_ESTRELA/plot_$NOME_ESTRELA.png'
    set title 'Estrela: $NOME_ESTRELA' font ',16' tc rgb 'blue'
    set xlabel '$NOME_X'
    set ylabel '$NOME_Y'
    set grid
    
    set key top left box opaque font ',12' spacing 1.2 textcolor rgb 'black'
    set style textbox opaque noborder margins 0.5,0.3

    files=system('cat $list')
    xhi=system('cat $range')+0
    set xrange [*:xhi]
    
    plot for [i=1:words(files)] sprintf('< sort -n -k$CX "%s"', word(files,i)) using $CX:$CY with lines lw 2 notitle, \
         '$lab' using 1:2:(\$3-\$1):(\$4-\$2) with vectors nohead lw 1 lc rgb 'gray' notitle, \
         '$lab' using 3:4:(sprintf('Massa: %s', stringcolumn(5))) with labels boxed left font ',10' offset 0.3,0 notitle, \
         '< echo "$BEST_X $BEST_Y"' using 1:2 with points pt 7 ps 2.5 lc rgb 'red' title "Melhor Ajuste: Massa=$BEST_MASS | M_H=$BEST_MH | Modelo=$BEST_MOD"
EOF

    rm -f "$list" "$lab" "$range" "$best"

    # Abre a imagem gerada em segundo plano
    xdg-open "Resultados/$NOME_ESTRELA/plot_$NOME_ESTRELA.png" &

done

echo "Todos os gráficos e tabelas foram gerados com sucesso!"
