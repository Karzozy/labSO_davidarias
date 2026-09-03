#!/bin/bash

dir="${1:-.}"

if [ ! -d "$dir" ]; then
    echo "Error: '$dir' no existe o no es un directorio" >&2
    exit 1
fi

convertidos=0
saltados=0

for archivo in "$dir"/*.mkv; do
    [ -e "$archivo" ] || continue

    salida="${archivo%.mkv}.mp4"

    if [ -e "$salida" ]; then
        echo "Saltando: '$salida' ya existe"
        saltados=$((saltados + 1))
        continue
    fi

    echo "Convirtiendo: '$archivo' -> '$salida'"
    ffmpeg -y -i "$archivo" "$salida" > /dev/null 2>&1

    if [ $? -eq 0 ]; then
        convertidos=$((convertidos + 1))
    else
        echo "Error al convertir '$archivo'" >&2
    fi
done

echo "Convertidos: $convertidos"
echo "Saltados: $saltados"

exit 0
