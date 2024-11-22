# Building Container

source uzibuild_env
uzibuild docker-build

# Building cc65

uzibuild docker-shell
cd ~/cc65
make -j 4

# Building fceux

uzibuild docker-shell
cd ~/fceux
mkdir build
cd build
cmake ..
make -j 4

# Running nestile

Install:

python3-tkinter

# Builing prototype

# Running fceux in userspace (OL9)

Requires:

qt5-devel
SDL2-devel
minizip-devel
