#!/bin/sh

die() {
	echo "$1"
	exit 1
}

# Make sure we have a loop
modprobe loop

# Do we have overlayfs?
if [ "`gzip -cd /proc/config.gz 2> /dev/null | grep OVERLAY`" ]; then
	EXT="squashfs"
	OVERLAYFS="yes"
else
	EXT="tar.gz"
	OVERLAYFS=""
fi

# Get latest image for us
mkdir -p /srv/libvirt/snapshots
cd /srv/libvirt/snapshots
ARCH="`uname -m`"
IMAGE_URL="`wget -O - http://obs.roz.lab.etn.com/images/$ARCH/ 2> /dev/null | sed -n 's|.*href="\(.*simpleimage.*\.'"$EXT"'\)".*|http://obs.roz.lab.etn.com/images/'"$ARCH"'/\1|p'`"
wget -c "$IMAGE_URL"
if [ "$1" ]; then
	IMAGE="$1"
else
	IMAGE="`ls -1 *.$EXT | sort -r | head -n 1`"
fi

# Destroy whatever was running
virsh -c lxc:// destroy latest 2> /dev/null > /dev/null
# may be wait for slow box
sleep 5
rm -rf "../overlays/$IMAGE"

# Mount squashfs
mkdir -p "../overlays/$IMAGE"
if [ "$OVERLAYFS" ]; then
	mkdir -p "../rootfs/$IMAGE-ro"
	umount -fl "../rootfs/$IMAGE-ro" 2> /dev/null > /dev/null
	mount -o loop "$IMAGE" "../rootfs/$IMAGE-ro" || die "Can't mount squashfs"
fi

# Cleanup of the rootfs
umount -fl "../rootfs/latest/lib/modules" 2> /dev/null > /dev/null
umount -fl "../rootfs/latest" 2> /dev/null > /dev/null
fusermount -u -z  "../rootfs/latest" 2> /dev/null > /dev/null

# clean up VM space
/bin/rm -rf "../rootfs/latest"
mkdir -p "../rootfs/latest"
# Mount rw image
if [ "$OVERLAYFS" ]; then
	mount -t overlayfs -o lowerdir="../rootfs/$IMAGE-ro",upperdir="../overlays/$IMAGE" overlayfs "../rootfs/latest" 2> /dev/null || die "Can't mount rw directory"
else
	mkdir -p "../rootfs/latest"
	tar -C "../rootfs/latest" -xzf "$IMAGE"
fi

# Bind mount modules
mkdir -p ../rootfs/latest/lib/modules
mount -o bind /lib/modules ../rootfs/latest/lib/modules

# copy root's ~/.ssh
cp -r --preserve ~/.ssh "../rootfs/latest/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "../rootfs/latest/etc/ssh"

# setup debian proxy
echo 'Acquire::http::Proxy "http://gate:3142";' > "../rootfs/latest/etc/apt/apt.conf.d/01proxy-apt-cacher"

# Start the virtual machine
virsh -c lxc:// start latest || die "Can't start the virtual machine"
