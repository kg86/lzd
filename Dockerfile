FROM ubuntu:16.04

RUN apt-get update &&  apt-get install -y scons clang libgtest-dev cmake
#RUN apt-get install -y scons clang libgtest-dev cmake

COPY ./ /root/app/

WORKDIR /root/app

RUN /root/app/scripts/install_gtest_ubuntu.sh
RUN scons