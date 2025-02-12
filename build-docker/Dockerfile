FROM oraclelinux:9

RUN dnf groupinstall --with-optional -y "Development Tools"
RUN dnf install -y oracle-epel-release-el9
RUN dnf install -y vim zlib \
		   qt5-qtbase-devel minizip-devel SDL2-devel # for fceux

RUN echo "alias ls='ls --color'" >> /root/.bashrc

WORKDIR /root
