#!/bin/zsh

# A `realpath` alternative using the default C implementation.
filepath() {
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

OPENBOX_ROOT="$(dirname $(dirname $(filepath $0)))"

cd $OPENBOX_ROOT

OPENBOX_LIBRARY_EVOLUTION=0 swift build
