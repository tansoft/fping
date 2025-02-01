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

# 定义下载URL和目标路径
URL="https://github.com/tansoft/fping/raw/refs/heads/develop/setup/fping.x86_64"
TARGET="/usr/bin/${detector_type}"
TIMEOUT=120

download_with_curl() {
    curl -sSL --connect-timeout "${TIMEOUT}" "${URL}" -o "${TARGET}"
    return $?
}

download_with_wget() {
    wget -q --timeout="${TIMEOUT}" -O "${TARGET}" "${URL}"
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
if [ $? -eq 0 ] && [ -f "${TARGET}" ]; then
    # 检查文件大小
    if [ -s "${TARGET}" ]; then
        chmod +x "${TARGET}"
        echo "Successfully downloaded and set executable permission for ${TARGET}"
    else
        echo "Error: Downloaded file is empty"
        rm -f "${TARGET}"
        exit 1
    fi
else
    echo "Error: Failed to download file"
    rm -f "${TARGET}"
    exit 1
fi

# 检测init system类型
if command -v systemctl >/dev/null 2>&1; then
    # systemd
    cat > /etc/systemd/system/${detector_type}.service << 'EOF'
[Unit]
Description=${detector_type} Service
After=network.target

[Service]
Type=simple
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
    cat > /etc/init.d/${detector_type} << 'EOF'
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
    cat > /etc/init/${detector_type}.conf << 'EOF'
description "${detector_type} Service"
start on runlevel [2345]
stop on runlevel [016]
respawn
exec /usr/bin/${detector_type}
EOF

    initctl reload-configuration
    start ${detector_type}
fi
