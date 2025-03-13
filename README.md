[![Build Status](https://travis-ci.org/schweikert/fping.svg?branch=develop)](https://travis-ci.org/schweikert/fping)
[![Coverage Status](https://coveralls.io/repos/github/schweikert/fping/badge.svg?branch=develop)](https://coveralls.io/github/schweikert/fping?branch=develop)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/11559/badge.svg?flat=1")](https://scan.coverity.com/projects/schweikert-fping)

# 关于项目

该项目引用自 [fping](https://fping.org/) ，增加了获取任务接口，多进程运行采集任务，并进行采集数据上传功能。如果带有参数运行该程序，和标准的fping功能一致。

## 一键启动机器并安装(AWS)

```bash
# 使用arm机型
instance_type="t4g.nano"
arch="arm64"

# 使用x86机型
instance_type="t3.nano"
arch="x86_64"

deploy_location="us-east-1"

instance=`aws ec2 run-instances \
    --image-id resolve:ssm:/aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-${arch} \
    --instance-type ${instance_type} \
    --user-data "#!/bin/bash
curl -sSL https://raw.githubusercontent.com/tansoft/fping/refs/heads/develop/setup/install-linux.sh | bash" \
    --query 'Instances[0].InstanceId' \
    --output json \
    --region ${deploy_location}`
echo "instance id: ${instance}"
```

## 已有机器一键安装

> 该脚本只在 Amazon Linux 2023 上进行了测试。

```bash
curl -sSL https://raw.githubusercontent.com/tansoft/fping/refs/heads/develop/setup/install-linux.sh | bash
```

## 可执行程序下载

预编译好的程序可以从以下地址下载，程序使用 Amazon Linux 2023 镜像进行编译，详见 [build-package.sh](setup/build-package.sh)。

* x86版本：[fping-x86_64.tar.gz](https://github.com/tansoft/fping/raw/refs/heads/develop/setup/fping-x86_64.tar.gz)
* arm版本：[fping-arm64.tar.gz](https://github.com/tansoft/fping/raw/refs/heads/develop/setup/fping-arm64.tar.gz)

## 编译方法

因为程序需要进行多机器分布式部署，因此把程序编译成 static 方式，减少依赖库版本和缺失问题。

```bash
# lib from yum
#yum -y install git automake g++ glibc-static libstdc++-static
# lib from dnf
#dnf -y install git automake g++ glibc-static libstdc++-static
# mac for test
#brew install gcc
# 源码编译
git clone https://github.com/tansoft/fping
cd fping
# 需要 libstdc++ 静态库，sudo yum install libstdc++ libstdc++-devel libstdc++-static
# 修改 src/Makefile.am 增加 libstdc++.a 的路径，如：AM_LDFLAGS = -static -L/usr/lib/gcc/x86_64-amazon-linux/11/32/libstdc++.a
# 参考：https://blog.csdn.net/weixin_43074760/article/details/131616272
./autogen.sh
# 这里指定任务获取接口url，注意为了缩少客户端编译后大小，这里只能使用http方式进行请求
# 如果需要调试，可以增加参数 --enable-debug
./configure --enable-centralmode="http://my-fping-job.com/job"
make
./src/fping
```

为了复用已经编译好的程序，程序会先判断环境变量 FPING_API_URL ，如果有则以该环境变量指向的地址为准。

# fping

fping is a program to send ICMP echo probes to network hosts, similar to ping,
but much better performing when pinging multiple hosts. fping has a long long
story: Roland Schemers did publish a first version of it in 1992 and it has
established itself since then as a standard tool.

_Current maintainer_:  
  David Schweikert \<david@schweikert.ch\>

_Website_:  
  https://fping.org/

_Mailing-list_:  
  https://groups.google.com/group/fping-users

## Installation

If you want to install fping from source, proceed as follows:

0. Run `./autogen.sh`
   (only if you got the source from Github).
1. Run `./configure` with the correct arguments.
   (see: `./configure --help`)
2. Run `make; make install`.
3. Make fping either setuid, or, if under Linux:
   `sudo setcap cap_net_raw,cap_net_admin+ep fping`

If you can't run fping as root or can't use the cap_net_raw capability, you can
also run fping in unprivileged mode. This works on MacOS and also on Linux,
provided that your GID is included in the range defined in
`/proc/sys/net/ipv4/ping_group_range`. This is particularly useful for running
fping in rootless / unprivileged containers. The --fwmark option needs root or
cap_net_admin.

## Usage

Have a look at the [fping(8)](doc/fping.pod) manual page for usage help.
(`fping -h` will also give a minimal help output.)

## Credits

* Original author:  Roland Schemers (schemers@stanford.edu)
* Previous maintainer:  RL "Bob" Morgan (morgan@stanford.edu)
* Initial IPv6 Support: Jeroen Massar (jeroen@unfix.org / jeroen@ipng.nl)
* Other contributors: see [CHANGELOG.md](CHANGELOG.md)
