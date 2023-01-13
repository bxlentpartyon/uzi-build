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

# Builing prototype

# Running fceux in userspace (Fedora 37)

Requires:

minizip
