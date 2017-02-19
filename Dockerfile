FROM ubuntu:16.04

RUN apt-get update

RUN apt-get install -y scons clang

COPY ./ /root/app/

WORKDIR /root/app

RUN scons