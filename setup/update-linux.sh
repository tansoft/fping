#!/bin/bash
# 卸载脚本 - 移除由install-linux.sh安装的fping服务和二进制文件

detector_type=${1:-fping-job}
TARGET="/usr/bin/${detector_type}"

echo "Update ${detector_type}..."

# 检测系统架构
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
    echo "Detected x86_64 architecture"
    PACKAGE_NAME="fping-x86_64.tar.gz"
elif [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
    echo "Detected arm64 architecture"
    PACKAGE_NAME="fping-arm64.tar.gz"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

# 定义下载URL和目标路径
URL="https://github.com/tansoft/fping/raw/refs/heads/develop/setup/${PACKAGE_NAME}"
DOWNLOAD_PATH="/tmp/${PACKAGE_NAME}"
TARGET="/usr/bin/${detector_type}"
TIMEOUT=120

download_with_curl() {
    curl -sSL --connect-timeout "${TIMEOUT}" "${URL}" -o "${DOWNLOAD_PATH}"
    return $?
}

download_with_wget() {
    wget -q --timeout="${TIMEOUT}" -O "${DOWNLOAD_PATH}" "${URL}"
    return $?
}

# 检查并使用可用的下载工具
if command -v curl >/dev/null 2>&1; then
    echo "Using curl to download..."
    download_with_curl
elif command -v wget >/dev/null 2>&1; then
    echo "Using wget to download..."
    download_with_wget
else
    echo "Error: Neither curl nor wget is available"
    exit 1
fi

# 检查下载是否成功
if [ $? -eq 0 ] && [ -f "${DOWNLOAD_PATH}" ]; then
    # 检查文件大小
    if [ -s "${DOWNLOAD_PATH}" ]; then
        echo "Successfully downloaded ${PACKAGE_NAME}"

        # 检测init system类型并停止/禁用服务
        if command -v systemctl >/dev/null 2>&1; then
            # systemd
            echo "Detected systemd, stopping service..."
            systemctl stop ${detector_type}.service

        elif [ -f "/etc/init.d/${detector_type}" ]; then
            # sysvinit
            echo "Detected sysvinit, stopping service..."
            service ${detector_type} stop

        elif [ -f "/etc/init/${detector_type}.conf" ]; then
            # upstart
            echo "Detected upstart, stopping service..."
            stop ${detector_type}
            
        fi

        # 直接解压文件到目标位置
        echo "Extracting ${PACKAGE_NAME}..."
        tar -xzf "${DOWNLOAD_PATH}" -O > "${TARGET}"
        
        # 检查解压是否成功
        if [ $? -ne 0 ] || [ ! -s "${TARGET}" ]; then
            echo "Error: Failed to extract ${PACKAGE_NAME}"
            rm -f "${DOWNLOAD_PATH}" "${TARGET}"
            exit 1
        fi

        echo `md5sum ${TARGET}`

        # 设置可执行权限
        chmod +x "${TARGET}"

        # 检测init system类型并停止/禁用服务
        if command -v systemctl >/dev/null 2>&1; then
            # systemd
            echo "Detected systemd, starting service..."
            systemctl start ${detector_type}.service

        elif [ -f "/etc/init.d/${detector_type}" ]; then
            # sysvinit
            echo "Detected sysvinit, starting service..."
            service ${detector_type} start

        elif [ -f "/etc/init/${detector_type}.conf" ]; then
            # upstart
            echo "Detected upstart, starting service..."
            start ${detector_type}
            
        fi
        
        # 清理临时文件
        rm -f "${DOWNLOAD_PATH}"
    else
        echo "Error: Downloaded file is empty"
        rm -f "${DOWNLOAD_PATH}"
        exit 1
    fi
else
    echo "Error: Failed to download file"
    rm -f "${DOWNLOAD_PATH}"
    exit 1
fi

echo "Update of ${detector_type} completed."
