#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
NEWLINE=$'\n'

function log() {
    echo -e "[ SH ]\e[38;5;81m $1 \e[0m"
}

function gen-file() {
    log "| generating $1"
    echo $2 > $1
}

function gen-allimport-file() {
    gen-file $1 "# $3"

    echo "" >> $1

    for file in *.csp; do
        echo "import \"${file}\";" >> $1
    done
}

pushd $SCRIPT_DIR
    
log "start generating stdlib"

gen-allimport-file "std.csp" $SCRIPT_DIR "std.csp - File for quickly importing all stdlib files."

for dir in * ; do
    if [ -d "${dir}" ] && [ -f "${dir}/generate.sh" ]; then
        log "* $dir contains generate.sh"
        source "$dir/generate.sh"
    fi
done

log "stdlib generation finished"

popd
