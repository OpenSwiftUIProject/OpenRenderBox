#!/bin/zsh

# A `realpath` alternative using the default C implementation.
filepath() {
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

gen_interface() {
  swift build -c release -Xswiftc -emit-module-interface -Xswiftc -enable-library-evolution -Xswiftc -no-verify-emitted-module-interface -Xswiftc -package-name -Xswiftc OpenRenderBox -Xswiftc -Osize

  mkdir -p .rb_template
}

gen_header() {
  mkdir -p .rb_template/Headers

  cp -r Sources/OpenRenderBox/include/OpenRenderBox/* .rb_template/Headers/

  # Rename files from ORBxx to RBxx and OpenRenderBoxxx to RenderBoxxx
  find .rb_template/Headers -name "ORB*" -type f | while read file; do
    new_name=$(echo "$file" | sed 's/ORB/RB/g')
    mv "$file" "$new_name"
  done

  find .rb_template/Headers -name "OpenRenderBox*" -type f | while read file; do
    new_name=$(echo "$file" | sed 's/OpenRenderBox/RenderBox/g')
    mv "$file" "$new_name"
  done

  # Update content in all header files
  find .rb_template/Headers -name "*.h" -type f | while read file; do
    sed -i '' 's/OpenRenderBox/RenderBox/g' "$file"
    sed -i '' 's/OPENRENDERBOX/RENDERBOX/g' "$file"
    sed -i '' 's/ORB/RB/g' "$file"
  done

  echo "Generated template headers successfully"
}

OPENRENDERBOX_ROOT="$(dirname $(dirname $(filepath $0)))"

cd $OPENRENDERBOX_ROOT

gen_interface
gen_header
