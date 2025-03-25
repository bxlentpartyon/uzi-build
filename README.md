# UZI for NES

This is the central repo where I've collected all the work I've done so
far to port Doug Braun's UZI kernel to the NES.

The majority of my work lives in the uzi-nes and uzi-nes-port
subdirectories, but I've also made modifications to the fceux emulator
and the cc65 C compiler to get where I am today.

The uzi-nes directory was originally a clone of paulie-g's dump of Doug
Braun's UZI kernel, here:

https://github.com/paulie-g/uzi

However, my work has diverged significantly from that original fork at
this point.  The work I did in there was to re-organize everything, and
then modify all of the code _just enough_ to get it to build with my
modified cc65.  Those goals were achieved quite a while ago at this
point, but the resulting binary is nowhere close to being a functional
NES ROM image.

The real heart of my work now lives in the uzi-nes-port subdirectory,
where I've taken the buildable code from the uzi-nes repo and slowly
started porting it over, piece by piece, into a functional NES ROM.  At
this point, I have something that is beginning to take the shape of an
operating system, using the MMC5 mapper to squeeze every bit of
functionality that I can get out of the NES.

While I believe that I could have achieved a more-or-less functional UZI
port using only the stock functionality provided by the MMC5, I have
ended up modifying fceux just a bit to give the MMC5 even more power, in
the form of battery-backed CHR-RAM.  This gives me something analogous
to a 4M disk, with a 2M volatile portion, and a 2M persistent writeable
portion, that gets saved off to the standard .sav file when the system
is shut down.  That will obviously require some custom hardware to
realize on a physical machine, but that was going to be a necessity
anwyay, as a real MMC5 can obviously only be had by cannabalizing a
donor cartridge, which I would absolutely not take part in (and would
probably screw up anyway).  The plan at this point, if I ever get that
far, is to implement the MMC5, and, realistically, most of the cartridge
hardware, on an FPGA.  This should hopefully make it relatively trivial
to make the emulator-only battery-backed CHR-RAM a reality.

Aside from all the UZI/UNIX/NES work, I've also made some very minor
modifications to cc65, basically to create the most barebones possible
build target, containing none of the nice helper code provided by cc65's
existing NES target.  I originally tried to work with the existing
target, but I felt that it was more geared towards writing "userland"
style programs, or games for the NES, which was higher-level than what I
wanted or needed, and also took away some of the fun/challenege of
learning how to operate the machine (or emulator, as it stands now)
completely on my own.

The repo also contains scripts to build a container where cc65 and fceux
can be built, to avoid polluting my machine with unnecessary development
packages.

Lastly I've archived a bit of reference material in the archive
subdirectory, which I've used to some degree to help me get to where I'm
at now.  In there is a PDF of a 1982 issue of Byte magazine, which I
used to figure out how the RTC from an old Z80 machine would have
worked, in this case the MM58167A.  This bit was probably unnecessary,
but it's done now, so whatever...  I've also archived a copy of Doug
Braun's Z280 port of UZI, which I gathered from here:

https://dangerousdoug.com/software/z280stuff.tar.Z

That is also intended to be used as reference material, mostly to flesh
out the "system software" and userspace portion of things.  So far, I
have used it to create Linux versions of the mkfs/fsck utilities, and
one custom tool (fsutil) which I've used to create the bare filesystem
image that my "PPU disk" uses.  This tarball probably deserves to be
dumped into its own repository, but I haven't gotten around to that yet.
I at least wanted to stash it here, in the event that Doug's copy of it
disappears someday.

The fork of nestile was ultimately unnecessary, but I've kept it around
for posterity.  It was used to craft the tileset that I use for my font,
but I haven't made any modifications to it.

# Build dependencies

docker
python
make
indent

# Building container

source uzibuild_env  
uzibuild docker-build  

# Building cc65 in the container

uzibuild docker-shell  
cd ~/cc65  
make -j 4  

# Building fceux in the container

uzibuild docker-shell  
cd ~/fceux  
mkdir build  
cd build  
cmake ..  
make -j 4  

# Building prototype

source uzibuild_env  
cd uzi-nes-port  
make  

# Running fceux in userspace (OL9)

## Requires

qt5-devel  
SDL2-devel  
minizip-devel  

# Running nestile

## Requires

python3-tkinter  

# Building a filesystem image for the PPU disk

I usually back up any existing uzi.sav first, though I haven't actually
written any code that _should_ modify the filesystem yet.

./mkfs test.img 128 4096
dd if=/dev/zero of=./test.sav bs=1 count=65536
cat test.sav test.img > uzi.sav
cp uzi.sav ~/.fceux/sav
