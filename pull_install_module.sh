#!/bin/bash
set -x
sudo modprobe -r os_keystore
sudo rsync lgerber@10.0.1.8:/home/lgerber/git/linux_kernel/kernel/os_keystore.ko /lib/modules/4.9.48+/kernel/kernel/
sudo modprobe os_keystore
