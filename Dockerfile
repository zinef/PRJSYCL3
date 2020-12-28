FROM ubuntu:20.04
RUN apt-get update -y
RUN apt-get install build-essential -y
RUN apt-get install libreadline-dev -y
RUN mkdir /home/projetSycL3
COPY our_shell_zfi.c /home/projetSycL3/our_shell_zfi.c
COPY our_shell_zfi.h /home/projetSycL3/our_shell_zfi.h
COPY main.c /home/projetSycL3/main.c
COPY makefile /home/projetSycL3/makefile
COPY tar.h /home/projetSycL3/tar.h
COPY tests.c /home/projetSycL3/tests.c
