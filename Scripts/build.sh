#!/bin/zsh

# A `realpath` alternative using the default C implementation.
filepath() {
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

OPENRENDERBOX_ROOT="$(dirname $(dirname $(filepath $0)))"

cd $OPENRENDERBOX_ROOT

# Set OPENRENDERBOX_LIB_SWIFT_PATH on Linux if swiftly is installed
if [[ "$(uname)" == "Linux" ]] && command -v swiftly &> /dev/null && [[ -z "$OPENRENDERBOX_LIB_SWIFT_PATH" ]]; then
  export OPENRENDERBOX_LIB_SWIFT_PATH="$(swiftly use --print-location)/usr/lib/swift"
  echo "Set OPENRENDERBOX_LIB_SWIFT_PATH=$OPENRENDERBOX_LIB_SWIFT_PATH"
fi

swift build
