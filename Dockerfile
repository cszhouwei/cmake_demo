#https://github.com/crashsystems/gitlab-docker/blob/master/Dockerfile
#https://github.com/widuu/chinese_docker/blob/master/userguide/dockerimages.md

FROM ubuntu

MAINTAINER Wei Zhou <cszhouwei@gmail.com>

RUN apt-get update; \
    apt-get -y upgrade

RUN apt-get -y install g++ cmake git subversion

RUN mkdir /home/git; \
    cd /home/git; \
    sudo git clone https://github.com/cszhouwei/cmake_demo.git -b master; \
    cd cmake_demo; \
    mkdir build; \
    cd build; \
    cmake -DCMAKE_BUILD_TYPE=Release ../; \
    make
    
CMD ["/home/git/cmake_demo/buld/module_xxx/module_xxx", "--config", "/etc/module_xxx.conf"]
