#!/bin/bash

API_URL="http://my-fping-job.com/job"

ARCHS="x86_64 arm64"

TMP_PATH=$(mktemp -d)

for ARCH in ${ARCHS}
do
    docker run -v ${TMP_PATH}:/var/task "public.ecr.aws/sam/build-provided.al2023:latest-${ARCH}" /bin/sh -c "dnf -y install glibc-static libstdc++-static;git clone https://github.com/tansoft/fping;cd fping;./autogen.sh;./configure --enable-centralmode='${API_URL}';make;cd ..;mv fping/src/fping fping;exit"
    (
        cd ${TMP_PATH}
        tar -zcvf fping-${ARCH}.tar.gz fping
        rm -f fping
    )
done

mv -f ${TMP_PATH}/fping-*.tar.gz .
rm -rf ${TMP_PATH}
