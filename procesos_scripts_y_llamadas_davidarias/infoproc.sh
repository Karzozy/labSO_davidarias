#!/bin/bash


pid="${1:-$$}"

if [ ! -d "/proc/$pid" ]; then
    echo "Error: el proceso $pid no existe" >&2
    exit 1
fi

mostrar_info() {
    local p="$1"
    local status="/proc/$p/status"
    local nombre estado ppid hilos

    while IFS= read -r linea; do
        case "$linea" in
            Name:*)
                nombre="${linea#Name:}"
                nombre="${nombre#"${nombre%%[![:space:]]*}"}"
                ;;
            State:*)
                estado="${linea#State:}"
                estado="${estado#"${estado%%[![:space:]]*}"}"
                ;;
            PPid:*)
                ppid="${linea#PPid:}"
                ppid="${ppid#"${ppid%%[![:space:]]*}"}"
                ;;
            Threads:*)
                hilos="${linea#Threads:}"
                hilos="${hilos#"${hilos%%[![:space:]]*}"}"
                ;;
        esac
    done < "$status"

    echo "PID: $p | Name: $nombre | State: $estado | PPid: $ppid | Threads: $hilos"
}

mostrar_info "$pid"

actual="$pid"
while [ "$actual" -ne 1 ]; do
    ppid_actual=""
    while IFS= read -r linea; do
        case "$linea" in
            PPid:*)
                ppid_actual="${linea#PPid:}"
                ppid_actual="${ppid_actual#"${ppid_actual%%[![:space:]]*}"}"
                ;;
        esac
    done < "/proc/$actual/status"

    if [ -z "$ppid_actual" ] || [ "$ppid_actual" -eq 0 ]; then
        break
    fi

    mostrar_info "$ppid_actual"
    actual="$ppid_actual"
done
exit 0