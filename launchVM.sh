#!/bin/bash -e

PATH_TO_COPY=$(dirname $0)"/"

echo "Path to be copied: $PATH_TO_COPY"

REBOOT=${REBOOT:-"no"}

VM_NAME=ubuntu16.04

USERNAME=root
PASSWORD=root

VM_TEST_DIR=/tmp/WLTEST

VM_FOUND=0
for vm in $(virsh list --all --name); do
	if [ $vm == $VM_NAME ]; then
		VM_FOUND=1
		break;
	fi
done

if [ "$VM_FOUND" -eq 0 ]; then
	echo "Virsh VM not found, exiting"
	exit -1
fi

for ((;;)); do
	if [ "$(virsh list --state-running | grep $VM_NAME | wc -l)" -eq "0" ]; then
		echo "VM was not running... Starting"
		virsh start $VM_NAME
		break
	elif [ "$REBOOT" == "yes" ]; then
		echo "VM already running, shutting down"
		virsh shutdown $VM_NAME
		sleep 5
	else
		break
	fi
done


# Fetch mac address

VM_MAC_ADDR=$(virsh dumpxml ubuntu16.04 | grep "mac address" | sed -e "s/[<>/=' ]\|mac address//g")

echo "VM's MAC address: $VM_MAC_ADDR"

VM_IP_ADDR=""
echo "Fetching IP address of the VM..."
for ((;;)); do
	VM_IP_ADDR=$(arp -n | grep $VM_MAC_ADDR | cut -f 1 -d " ")
	if [ "$VM_IP_ADDR" != "" ]; then
		break;
	fi
	sleep 1
done
echo "VM's IP address: $VM_IP_ADDR"

echo "Copying folder to remote"
sshpass -p $PASSWORD rsync -avh $PATH_TO_COPY $USERNAME@$VM_IP_ADDR:$VM_TEST_DIR # --delete 

echo "Running test script"
sshpass -p $PASSWORD ssh -CX $USERNAME@$VM_IP_ADDR $VM_TEST_DIR/doall.sh

echo "DONE, exiting"
exit

KERNEL_IMG_PATH=/work/git/linux/arch/x86/boot

#qemu-system-x86_64 -hda /work/git/lisa/tools/wltests/platforms/qemu/artifacts/disk.qcow2

#exit

#VM_KERNEL_ROOT=/work/git/linux/

    #x-serial tcp::1234,server,nowait \
    #-serial mon:stdio \
    #-nographic \
kvm \
    -no-acpi \
    -hda /work/git/lisa/tools/wltests/platforms/qemu/artifacts/disk.qcow2 \
    -cpu qemu64 \
    -smp 4 \
    -m 2048 \
    -kernel $KERNEL_IMG_PATH/bzImage \
    -append 'nokaslr root=/dev/sda1 rw kgdbwait kgdboc=ttyS0,115200'
    #-append 'nokaslr root=/dev/sda1 rw kgdbwait kgdboc=ttyS0,115200'

    
    
    #-serial pty \
    #-netdev user,id=net0,hostfwd=tcp::5555-:22 \
    #-device e1000,netdev=net0 \

#-soundhw es1370 -no-acpi -snapshot -localtime -boot c -usb -usbdevice tablet #-net nic,vlan=0,macaddr=00:00:10:52:37:48 -net tap,vlan=0,ifname=tap0,script=no
