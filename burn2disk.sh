#! /usr/bin/bash

# Write disk image to hard disk


USB_DEVICE=$1

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# check whether /dev/sdb exists
if [ ! -b ${USB_DEVICE} ]; then
    echo -e "${RED}File not found: ${USB_DEVICE}${NC}";
    exit 1 ;
fi

# disp disk information
echo -e "${GREEN}Display disk information${NC}"
sudo fdisk -l ${USB_DEVICE}

# confirmation
echo -e "${GREEN}Operation will erase all data on above disk, do you confirm? (Yes or Not)${NC}"
read result ;
if [ $result != "Yes" ]; then
    exit 1;
fi
echo -e "${GREEN}Please reconfirm (Yes or Not)${NC}"
read result ;
if [ $result != "Yes" ]; then
    exit 1;
fi

# perform write operation
echo -e "${BLUE}Write to USB driver ${USB_DEVICE}...${NC}"
sudo dd status=none if=disk.img of=${USB_DEVICE} bs=512 count=512000
sync
