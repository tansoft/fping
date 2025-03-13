#!/bin/bash
# 使用systemd的发行版：
#   Amazon Linux 2/2023
#   Red Hat Enterprise Linux (RHEL) 7及以上
#   CentOS 7及以上
#   Ubuntu 16.04及以上
#   Debian 8及以上
#   SUSE Linux Enterprise Server 12及以上
# 不使用systemd的发行版：
#   Amazon Linux 1 (使用upstart)
#   Ubuntu 14.04及以下 (使用upstart)
#   Debian 7及以下 (使用sysvinit)
#   Alpine Linux (使用OpenRC)

detector_type=${1:-fping-job}

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
        
        # 直接解压文件到目标位置
        echo "Extracting ${PACKAGE_NAME}..."
        tar -xzf "${DOWNLOAD_PATH}" -O > "${TARGET}"
        
        # 检查解压是否成功
        if [ $? -ne 0 ] || [ ! -s "${TARGET}" ]; then
            echo "Error: Failed to extract ${PACKAGE_NAME}"
            rm -f "${DOWNLOAD_PATH}" "${TARGET}"
            exit 1
        fi
        
        # 设置可执行权限
        chmod +x "${TARGET}"
        echo "Successfully installed ${TARGET}"
        
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

# 检测init system类型
if command -v systemctl >/dev/null 2>&1; then
    # systemd
    cat > /etc/systemd/system/${detector_type}.service << EOF
[Unit]
Description=${detector_type} Service
After=network.target

[Service]
Type=simple
Environment=FPING_API_URL="http://my-fping-job.com/job"
ExecStart=/usr/bin/${detector_type}
Restart=always

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    systemctl enable ${detector_type}.service
    systemctl start ${detector_type}.service

elif [ -f /etc/init.d ]; then
    # sysvinit
    cat > /etc/init.d/${detector_type} << EOF
#!/bin/sh
### BEGIN INIT INFO
# Provides:          ${detector_type}
# Required-Start:    $network
# Required-Stop:     $network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       ${detector_type} Service
### END INIT INFO

case "\$1" in
    start)
        export FPING_API_URL="http://my-fping-job.com/job"
        /usr/bin/${detector_type} &
        ;;
    stop)
        killall ${detector_type}
        ;;
    *)
        echo "Usage: \$0 {start|stop}"
        exit 1
        ;;
esac
exit 0
EOF

    chmod +x /etc/init.d/${detector_type}
    chkconfig --add ${detector_type}
    service ${detector_type} start

elif [ -f /etc/init ]; then
    # upstart
    cat > /etc/init/${detector_type}.conf << EOF
description "${detector_type} Service"
start on runlevel [2345]
stop on runlevel [016]
respawn
env FPING_API_URL="http://my-fping-job.com/job"
exec /usr/bin/${detector_type}
EOF

    initctl reload-configuration
    start ${detector_type}
fi
