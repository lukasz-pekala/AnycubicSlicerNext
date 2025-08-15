#!/bin/bash

set -e
set -o pipefail

while getopts ":dpa:snt:xbc:hug:" opt; do
  case "${opt}" in
    d )
        export BUILD_TARGET="deps"
        ;;
    p )
        export PACK_DEPS="1"
        ;;
    a )
        export ARCH="$OPTARG"
        ;;
    s )
        export BUILD_TARGET="slicer"
        ;;
    n )
        export NIGHTLY_BUILD="1"
        ;;
    t )
        export OSX_DEPLOYMENT_TARGET="$OPTARG"
        ;;
    x )
        export SLICER_CMAKE_GENERATOR="Ninja"
        export SLICER_BUILD_TARGET="all"
        export DEPS_CMAKE_GENERATOR="Ninja"
        ;;
    b )
        export BUILD_ONLY="1"
        ;;
    c )
        export BUILD_CONFIG="$OPTARG"
        ;;
    1 )
        export CMAKE_BUILD_PARALLEL_LEVEL=1
        ;;
    u )
        export BUILD_UNIVERSAL="1"
        ;;
    g )
        export GHPROXY="$OPTARG"
        ;;
    h ) echo "Usage: ./build_release_macos.sh [-d]"
        echo "   -d: Build deps only"
        echo "   -a: Set ARCHITECTURE (arm64 or x86_64)"
        echo "   -s: Build slicer only"
        echo "   -n: Nightly build"
        echo "   -t: Specify minimum version of the target platform, default is 11.3"
        echo "   -x: Use Ninja CMake generator, default is Xcode"
        echo "   -b: Build without reconfiguring CMake"
        echo "   -c: Set CMake build configuration, default is Release"
        echo "   -u: Build universal binary (both arm64 and x86_64)"
        echo "   -1: Use single job for building"
        echo "   -g: Set GitHub proxy. default is null"
        exit 0
        ;;
    * )
        ;;
  esac
done

# Set defaults

if [ -z "$ARCH" ]; then
  if [ "1." == "$BUILD_UNIVERSAL". ]; then
    ARCH="universal"
  else
    ARCH="$(uname -m)"
  fi
  export ARCH
fi

if [ "1." == "$BUILD_UNIVERSAL". ]; then
  echo "Universal build enabled - will create a combined arm64/x86_64 binary"
fi

if [ -z "$BUILD_CONFIG" ]; then
  export BUILD_CONFIG="Release"
fi

if [ -z "$BUILD_TARGET" ]; then
  export BUILD_TARGET="all"
fi

if [ -z "$SLICER_CMAKE_GENERATOR" ]; then
  export SLICER_CMAKE_GENERATOR="Xcode"
fi

if [ -z "$SLICER_BUILD_TARGET" ]; then
  export SLICER_BUILD_TARGET="ALL_BUILD"
fi

if [ -z "$DEPS_CMAKE_GENERATOR" ]; then
  export DEPS_CMAKE_GENERATOR="Unix Makefiles"
fi

if [ -z "$OSX_DEPLOYMENT_TARGET" ]; then
  export OSX_DEPLOYMENT_TARGET="11.3"
fi

echo "Build params:"
echo " - ARCH: $ARCH"
echo " - BUILD_CONFIG: $BUILD_CONFIG"
echo " - BUILD_TARGET: $BUILD_TARGET"
echo " - CMAKE_GENERATOR: $SLICER_CMAKE_GENERATOR for Slicer, $DEPS_CMAKE_GENERATOR for deps"
echo " - OSX_DEPLOYMENT_TARGET: $OSX_DEPLOYMENT_TARGET"
echo

# if which -s brew; then
# 	brew --prefix libiconv
# 	brew --prefix zstd
# 	export LIBRARY_PATH=$LIBRARY_PATH:$(brew --prefix zstd)/lib/
# elif which -s port; then
# 	port install libiconv
# 	port install zstd
# 	export LIBRARY_PATH=$LIBRARY_PATH:/opt/local/lib
# else
# 	echo "Need either brew or macports to successfully build deps"
# 	exit 1
# fi

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# PROJECT_BUILD_DIR="$PROJECT_DIR/build_$ARCH"
DEPS_DIR="$PROJECT_DIR/deps"


# Fix for Multi-config generators
if [ "$SLICER_CMAKE_GENERATOR" == "Xcode" ]; then
    export BUILD_DIR_CONFIG_SUBDIR="/$BUILD_CONFIG"
else
    export BUILD_DIR_CONFIG_SUBDIR=""
fi

function build_deps() {
    local DEPS="$1"
    local DEPS_BUILD_DIR="$2"
    local ARCH="$3"

    if [ -f "$DEPS/.finished" ]; then
        echo "Deps already built, skip building"
        return 0
    fi
    echo "Building deps..."
    (
        set -x
        mkdir -p "$DEPS"
        mkdir -p "$DEPS_BUILD_DIR"
        if [ "1." != "$BUILD_ONLY". ]; then
            local ghproxy_arg=""
            [ -n "$GHPROXY" ] && ghproxy_arg="-DGHPROXY=${GHPROXY}"
            cmake -S "${DEPS_DIR}" -B "${DEPS_BUILD_DIR}" \
                -G "${DEPS_CMAKE_GENERATOR}" \
                -DDESTDIR="$DEPS" -DDEP_DOWNLOAD_DIR="${DEPS_DIR}/DL_CACHE" \
                $ghproxy_arg \
                -DOPENSSL_ARCH="darwin64-${ARCH}-cc" \
                -DCMAKE_BUILD_TYPE="$BUILD_CONFIG" \
                -DCMAKE_OSX_ARCHITECTURES:STRING="${ARCH}" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET="${OSX_DEPLOYMENT_TARGET}"
        fi
        cmake --build "${DEPS_BUILD_DIR}" --config "$BUILD_CONFIG" && touch $DEPS/.finished
    )
}

function pack_deps() {
    echo "Packing deps..."
    (
        set -x
        cd "$DEPS"
        tar -zcvf "AnycubicSlicerNext_dep_mac_${BUILD_CONFIG}_${ARCH}_$(date +"%Y%m%d").tar.gz" .
        cd -
    )
}

function build_slicer() {
    local DEPS="$1"
    local PROJECT_BUILD_DIR="$2"
    local ARCH="$3"
    echo "Building slicer..."
    (
        set -x
        if [ "1." != "$BUILD_ONLY". ]; then
            cmake -S "${PROJECT_DIR}"  -B ${PROJECT_BUILD_DIR}\
                -G "${SLICER_CMAKE_GENERATOR}" \
                -DBBL_RELEASE_TO_PUBLIC=1 \
                -DCMAKE_PREFIX_PATH="$DEPS/usr/local" \
                -DCMAKE_INSTALL_PREFIX="$PWD/AnycubicSlicerNext" \
                -DCMAKE_BUILD_TYPE="$BUILD_CONFIG" \
                -DCMAKE_MACOSX_RPATH=ON \
                -DCMAKE_INSTALL_RPATH="${DEPS}/usr/local" \
                -DCMAKE_MACOSX_BUNDLE=ON \
                -DCMAKE_OSX_ARCHITECTURES="${ARCH}" \
                -DCMAKE_OSX_DEPLOYMENT_TARGET="${OSX_DEPLOYMENT_TARGET}"
        fi
        cmake --build "${PROJECT_BUILD_DIR}" --config "$BUILD_CONFIG" --target "$SLICER_BUILD_TARGET"
    )

    echo "Verify localization with gettext..."
    (
        cd "$PROJECT_DIR"
        sh "$PROJECT_DIR/run_gettext.sh"
    )



    # extract version
    # export ver=$(grep '^#define SoftFever_VERSION' ../src/libslic3r/libslic3r_version.h | cut -d ' ' -f3)
    # ver="_V${ver//\"}"
    # echo $PWD
    # if [ "1." != "$NIGHTLY_BUILD". ];
    # then
    #     ver=${ver}_dev
    # fi

    # zip -FSr OrcaSlicer${ver}_Mac_${ARCH}.zip OrcaSlicer.app
}

function build_universal() {
    echo "Building universal binary..."
    local UNIVERSAL_BUILD_DIR="$PROJECT_DIR/universal"
    for ARCH in "x86_64" "arm64"; do
        echo "Building for $ARCH"
        local DEPS_BUILD_DIR="$DEPS_DIR/build_${BUILD_CONFIG}_$ARCH"
        local DEPS="$PROJECT_DIR/build/${BUILD_CONFIG}_$ARCH"
        local PROJECT_BUILD_DIR="$PROJECT_DIR/build_${BUILD_CONFIG}_$ARCH"
        build_deps "$DEPS" "$DEPS_BUILD_DIR" "$ARCH"
        build_slicer "$DEPS" "$PROJECT_BUILD_DIR" "$ARCH"
        cp -R "$PROJECT_BUILD_DIR/${BUILD_CONFIG}/AnycubicSlicerNext.app" "$UNIVERSAL_BUILD_DIR/AnycubicSlicerNext_$ARCH.app"
    done

    
    # Create universal binary
    echo "Creating universal binary..."
    (
        set -x

        ls "$UNIVERSAL_BUILD_DIR" 2>&1  > /dev/null || mkdir -p "$UNIVERSAL_BUILD_DIR"
        UNIVERSAL_APP="$UNIVERSAL_BUILD_DIR/AnycubicSlicerNext.app"
        ls "$UNIVERSAL_APP" 2>&1  > /dev/null  && rm -rf "$UNIVERSAL_APP"

        # Create universal binary using lipo
        for BINARY_PATH in "Contents/MacOS/AnycubicSlicerNext" \
            "Contents/Frameworks/libusb-1.0.0.dylib" \
            "Contents/Frameworks/libftdi1.2.dylib" \
            "Contents/Frameworks/libhidapi.0.dylib" \
            "Contents/Frameworks/libusb-1.0.0.dylib" \
            "Contents/Frameworks/libftdi1.2.dylib" \
            "Contents/Frameworks/libhidapi.0.dylib" ; do
            lipo -create \
                    "$UNIVERSAL_BUILD_DIR/AnycubicSlicerNext_x86_64.app/$BINARY_PATH" \
                    "$UNIVERSAL_BUILD_DIR/AnycubicSlicerNext_arm64.app/$BINARY_PATH" \
                    -output "$UNIVERSAL_APP/$BINARY_PATH"
        done
        
        echo "Universal binary created at $UNIVERSAL_APP"
    )
}

case "${BUILD_TARGET}" in
    all)
        if [ "1." == "$BUILD_UNIVERSAL". ]; then
            build_universal
        else
            local DEPS_BUILD_DIR="$DEPS_DIR/build_${BUILD_CONFIG}_$ARCH"
            local DEPS="$PROJECT_DIR/build/${BUILD_CONFIG}_$ARCH"
            build_deps "$DEPS" "$DEPS_BUILD_DIR" "$ARCH"
            build_slicer "$DEPS" "$PROJECT_BUILD_DIR" "$ARCH"
        fi
        ;;
    deps)
        local DEPS_BUILD_DIR="$DEPS_DIR/build_${BUILD_CONFIG}_$ARCH"
        local DEPS="$PROJECT_DIR/build/${BUILD_CONFIG}_$ARCH"
        build_deps "$DEPS" "$DEPS_BUILD_DIR" "$ARCH"
        ;;
    slicer)
        if [ "1." == "$BUILD_UNIVERSAL". ]; then
            build_universal
        else
            local DEPS="$PROJECT_DIR/build/${BUILD_CONFIG}_$ARCH"
            build_slicer "$DEPS" "$PROJECT_BUILD_DIR" "$ARCH"
        fi
        ;;
    *)
        echo "Unknown target: $BUILD_TARGET. Available targets: deps, slicer, all."
        exit 1
        ;;
esac

if [ "1." == "$PACK_DEPS". ]; then
    pack_deps
fi
