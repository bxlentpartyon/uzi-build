#! /bin/bash

./mkfs test.img 128 4096
cp test.img test.img.orig
./fsck test.img
xxd test.img > test.img.hex
xxd test.img.orig > test.img.orig.hex

if ! diff test.img.hex test.img.orig.hex; then
	echo "Test failed - mkfs and fsck results differ!"
	exit 1
else
	echo "Success!"
	rm test.img test.img.orig test.img.hex test.img.orig.hex
fi

