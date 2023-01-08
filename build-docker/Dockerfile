FROM fedora:37

RUN dnf groupinstall -y "Development Tools" "C Development Tools and Libraries"
RUN dnf install -y vim zlib git \
		      cmake gcc-c++ qt5-qtbase-devel minizip-devel SDL2-devel # for fceux
RUN echo "alias ls='ls --color'" >> /root/.bashrc
RUN mkdir /root/output

WORKDIR /root
