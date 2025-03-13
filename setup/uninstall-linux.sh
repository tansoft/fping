#!/bin/bash
# 卸载脚本 - 移除由install-linux.sh安装的fping服务和二进制文件

detector_type=${1:-fping-job}
TARGET="/usr/bin/${detector_type}"

echo "Uninstalling ${detector_type}..."

# 检测init system类型并停止/禁用服务
if command -v systemctl >/dev/null 2>&1; then
    # systemd
    echo "Detected systemd, stopping and disabling service..."
    systemctl stop ${detector_type}.service
    systemctl disable ${detector_type}.service
    
    # 删除服务文件
    if [ -f "/etc/systemd/system/${detector_type}.service" ]; then
        echo "Removing systemd service file..."
        rm -f /etc/systemd/system/${detector_type}.service
        systemctl daemon-reload
    fi

elif [ -f "/etc/init.d/${detector_type}" ]; then
    # sysvinit
    echo "Detected sysvinit, stopping service..."
    service ${detector_type} stop
    chkconfig --del ${detector_type}
    
    # 删除服务文件
    echo "Removing sysvinit service file..."
    rm -f /etc/init.d/${detector_type}

elif [ -f "/etc/init/${detector_type}.conf" ]; then
    # upstart
    echo "Detected upstart, stopping service..."
    stop ${detector_type}
    
    # 删除服务文件
    echo "Removing upstart service file..."
    rm -f /etc/init/${detector_type}.conf
    initctl reload-configuration
fi

# 删除二进制文件
if [ -f "${TARGET}" ]; then
    echo "Removing binary file ${TARGET}..."
    rm -f "${TARGET}"
fi

echo "Uninstallation of ${detector_type} completed."
