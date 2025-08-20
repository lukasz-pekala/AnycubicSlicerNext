#!/bin/bash

PROJECT_DIR=$(dirname $(readlink -f ${0}))
DEPS_DIR=${PROJECT_DIR}/deps

cd ${PROJECT_DIR}
set -e # exit on first error

function usage() {
    echo "Usage: ./BuildLinux.sh [-1][-b][-c][-d][-i][-r][-s][-u]"
    echo "   -d: Build deps only"
    echo "   -s: Build slicer only"
    echo "   -c: Set CMake build configuration, default is Release"
    echo "   -j: Use single job for building"
    echo "   -g: Set GitHub proxy. default is null"
    echo "   -u: install dependencies (optional and need sudo)"
    echo "   -i: Generate appimage (optional)"
    echo "   -h: Show this help message"
    echo "For a first use, you want to 'sudo ./BuildLinux.sh -u'"
    echo "   and then './BuildLinux.sh'"
}

function build_deps(){
    type=$1 # CMAKE_BUILD_TYPE
    install_dir=$2
    build_dir=${DEPS_DIR}/build_${type}
    
    BUILD_ARGS="-DDEP_WX_GTK3=ON"


    echo "Checking: ${install_dir}/done"
    if [[ ! -f "${install_dir}/done" ]]; then
        echo "Configuring dependencies..."
        if [[ -n "${GITHUB_PROXY}" ]]; then
            BUILD_ARGS="${BUILD_ARGS} -DGHPROXY=${GITHUB_PROXY}"
        fi
        
        BUILD_ARGS="${BUILD_ARGS} -DCMAKE_BUILD_TYPE=${type}"
        
        echo "cmake -S ${DEPS_DIR} -B ${build_dir} ${BUILD_ARGS}  -DDESTDIR=\"$install_dir\" -DDEP_DOWNLOAD_DIR=\"${DEPS_DIR}/DL_CACHE\""
        cmake -S ${DEPS_DIR} -B ${build_dir} ${BUILD_ARGS}  -DDESTDIR="$install_dir" -DDEP_DOWNLOAD_DIR="${DEPS_DIR}/DL_CACHE"
        cmake --build ${build_dir} --config ${type} && touch ${install_dir}/done
    else
        echo "Dependencies already built, skip building..."
    fi
}


function build_slicer(){
    type=$1 # CMAKE_BUILD_TYPE
    deps_install_dir=$2/usr/local
    echo "Configuring AnycubicSlicer..."
    build_dir=$3
    BUILD_ARGS=""
    if [[ -n "${FOUND_GTK3_DEV}" ]]
    then
        BUILD_ARGS="-DSLIC3R_GTK=3"
    fi

    BUILD_ARGS="${BUILD_ARGS} -DCMAKE_BUILD_TYPE=${type}"

    echo -e "cmake -S ${PROJECT_DIR} -B ${build_dir}  -DCMAKE_PREFIX_PATH="${deps_install_dir}" ${BUILD_ARGS}"
    cmake -S ${PROJECT_DIR} -B ${build_dir}  \
        -DCMAKE_PREFIX_PATH="${deps_install_dir}" \
        -DANYCUBICTOOLS=ON  ${BUILD_ARGS} -DSLIC3R_STATIC=ON
    echo "done"
    echo "Building AnycubicSlicer ..."
    cmake --build ${build_dir} --target AnycubicSlicer -j$(nproc)
    echo "Building AnycubicSlicer_profile_validator .."
    cmake --build ${build_dir} --target AnycubicSlicer_profile_validator -j$(nproc)
    sh "$PROJECT_DIR/run_gettext.sh"
    echo "done"
}

BUILD_TARGET="all"
unset name
while getopts ":jg:sduihc:" opt; do
  case ${opt} in
    j )
        export CMAKE_BUILD_PARALLEL_LEVEL=1
        ;;
    c )
        CONFIG=${OPTARG}
        ;;
    d )
       BUILD_TARGET="deps"
        ;;
    h ) usage
        exit 0
        ;;
    i )
        BUILD_IMAGE="1"
        ;;
    s )
        BUILD_TARGET="slicer"
        ;;
    u )
        UPDATE_LIB="1"
        ;;
    g )
        GITHUB_PROXY=${OPTARG}
        ;;
  esac
done
echo "PROJECT_DIR=${PROJECT_DIR}"
echo "DEPS_DIR=${DEPS_DIR}"
echo "BUILD_TARGET=${BUILD_TARGET}"
echo "CONFIG=${CONFIG}"
echo "UPDATE_LIB=${UPDATE_LIB}"
echo "BUILD_IMAGE=${BUILD_IMAGE}"
echo "GITHUB_PROXY=${GITHUB_PROXY}"


if [ ${OPTIND} -eq 1 ]
then
    usage
    exit 0
fi

DISTRIBUTION=$(grep -e ^ID= /etc/os-release | cut -d= -f 2)
DISTRIBUTION_LIKE=$(grep -e ^ID_LIKE= /etc/os-release | cut   -d= -f 2)
# Check for direct distribution match to Ubuntu/Debian
if [ "${DISTRIBUTION}" == "ubuntu" ] || [ "${DISTRIBUTION}" == "linuxmint" ]; then
    DISTRIBUTION="debian"
# Check if distribution is Debian/Ubuntu-like based on ID_LIKE
elif [[ "${DISTRIBUTION_LIKE}" == *"debian"* ]] || [[ "${DISTRIBUTION_LIKE}" == *"ubuntu"* ]]; then
    DISTRIBUTION="debian"
fi
if [ ! -f ./linux.d/${DISTRIBUTION} ]
then
    echo "Your distribution does not appear to be currently supported by these build scripts"
    exit 1
fi
source ./linux.d/${DISTRIBUTION}

echo "FOUND_GTK3=${FOUND_GTK3}"
if [[ -z "${FOUND_GTK3_DEV}" ]]
then
    echo "Error, you must install the dependencies before."
    echo "Use option -u with sudo"
    exit 1
fi


if [[ -n "${GITHUB_PROXY}" ]]; then
    # 确保代理URL以斜杠结尾
    if [[ "${GITHUB_PROXY: -1}" != "/" ]]; then
        GITHUB_PROXY="${GITHUB_PROXY}/"
    fi

    # 如果设置了代理，也去确保ippicv的下载使用代理
    # NOTE: 如果opencv版本有变更这里也需要更新
    IPPICV_COMMIT="a56b6ac6f030c312b2dce17430eef13aed9af274"
    export OPENCV_IPPICV_URL="${GITHUB_PROXY}https://raw.githubusercontent.com/opencv/opencv_3rdparty/${IPPICV_COMMIT}/ippicv/"
fi


if [[ "${BUILD_TARGET}" == "deps" || "${BUILD_TARGET}" == "all" ]]
then
    build_deps ${CONFIG} ${PROJECT_DIR}/deps_install/${CONFIG}
fi


if [[ "${BUILD_TARGET}" == "slicer" || "${BUILD_TARGET}" == "all" ]]
then
    build_slicer ${CONFIG} ${PROJECT_DIR}/deps_install/${CONFIG} ${PROJECT_DIR}/build_slicer_${CONFIG}
fi

if [[ -e ${PROJECT_DIR}/build_slicer_${CONFIG}/src/BuildLinuxImage.sh ]]; then


echo "[9/9] Generating Linux app..."
    pushd build
        if [[ -n "${BUILD_IMAGE}" ]]
        then
           sh ${PROJECT_DIR}/build_slicer_${CONFIG}/src/BuildLinuxImage.sh -i
        else
           sh ${PROJECT_DIR}/build_slicer_${CONFIG}/src/BuildLinuxImage.sh
        fi
    popd
echo "done"
fi


cd -