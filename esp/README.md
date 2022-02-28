https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html

sudo pacman -S --needed gcc git make flex bison gperf python-pip cmake ninja ccache dfu-util libusb

git clone -b v4.4 --recursive https://github.com/espressif/esp-idf.git esp-idf
git clone https://github.com/espressif/esp-dsp.git

cd ~/esp/esp-idf
./install.sh esp32
. ./export.sh

. ~/fiit-stu/BP/bachelor-thesis/esp/esp-idf/export.sh


# /home/miroslav/.espressif/ ##########################################

cp -r $IDF_PATH/examples/get-started/hello_world .
cd ~/esp/hello_world

idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p  flash