ms8236usb:	ms8236usb.c
	gcc -Wall ms8236usb.c -o ms8236usb
install:	ms8236usb
	sudo mv ms8236usb /usr/local/bin
