FROM nvidia/cuda:10.1-cudnn7-devel-ubuntu18.04

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get -y update && apt-get install -y wget git sudo software-properties-common && apt-get purge && apt-get clean

RUN mkdir -p /opt/r2dtu

COPY del /opt/r2dtu/del
COPY depend /opt/r2dtu/depend
COPY scripts /opt/r2dtu/scripts

WORKDIR /opt/r2dtu

RUN ./scripts/bootstrap
RUN ./scripts/setup

COPY cmake /opt/r2dtu/cmake
COPY data /opt/r2dtu/data
COPY testing /opt/r2dtu/testing
COPY include /opt/r2dtu/include
COPY src /opt/r2dtu/src
COPY CMakeLists.txt /opt/r2dtu/CMakeLists.txt

RUN ./scripts/build

