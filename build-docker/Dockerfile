FROM opensuse/leap:15.4

RUN zypper install -y -t pattern devel_basis devel_C_C++
RUN zypper install -y vim zlib git \
		      cmake gcc-c++ libqt5-qtbase-devel minizip-devel libSDL2-devel # for fceux
RUN echo "alias ls='ls --color'" >> /root/.bashrc
RUN mkdir /root/output

WORKDIR /root
