FROM ubuntu

MAINTAINER Wei Zhou <cszhouwei@gmail.com>

RUN apt-get update; \
    apt-get -y upgrade

RUN apt-get -y install cmake
