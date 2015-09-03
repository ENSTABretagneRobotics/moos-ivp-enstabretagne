Install :

sudo apt-get install libusb-1.0-0-dev mono-runtime libmono-winforms2.0-cil
copy the file 99-pololu.rules to /etc/udev/rules.d/ in order to grant permission for all users to use Pololu USB devices
sudo udevadm control --reload-rules
sudo adduser theuser dialout