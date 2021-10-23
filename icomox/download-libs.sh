#!/usr/bin/env bash
# Skript na stiahnutie knižníc a zmazanie nepotrebných súčastí
# AnalogDevices.ADuCM4x50_DFP.4.0.0   https://developer.arm.com/tools-and-software/embedded/cmsis/cmsis-packs
# ARM.CMSIS.5.8.0                     https://github.com/ARM-software/CMSIS_5/releases/latest

HAL_DIR="ADuCM4x50"
HAL_SOURCE="AnalogDevices.ADuCM4x50_DFP.4.0.0.pack"
CMSIS_DIR="CMSIS"
CMSIS_SOURCE="ARM.CMSIS.5.8.0.pack"
OPENOCD_CODE="openocd"
OPENOCD_PATCH="4f9cfbd.fixed.patch"

wget "https://download.analog.com/tools/EZBoards/ADuCM4050/Releases/$HAL_SOURCE"
wget "https://github.com/ARM-software/CMSIS_5/releases/download/5.8.0/$CMSIS_SOURCE"
git clone "https://github.com/ArrowElectronics/iCOMOX.git"
git clone "https://git.code.sf.net/p/openocd/code" "$OPENOCD_CODE"

unzip -d "${HAL_DIR}" "$HAL_SOURCE"
rm -r "${HAL_DIR}/ARM" "${HAL_DIR}/Boards" "${HAL_DIR}/Flash" "${HAL_DIR}/tools" \
      "${HAL_DIR}/Source/ARM" "${HAL_DIR}/Source/IAR" "${HAL_DIR}/Source/drivers/flash" "${HAL_DIR}/Source/drivers/rtc" \
      "${HAL_DIR}/AnalogDevices.ADuCM4x50_DFP.pdsc" "${HAL_DIR}/examples.xml"

unzip -d "${CMSIS_DIR}" "$CMSIS_SOURCE"
cp "${CMSIS_DIR}/LICENCE.txt" "${CMSIS_DIR}/CMSIS/LICENCE.txt"
cp -r "${CMSIS_DIR}/CMSIS/" ./
rm -r "${CMSIS_DIR}/CMSIS/" "${CMSIS_DIR}/Core_A" "${CMSIS_DIR}/DAP" "${CMSIS_DIR}/NN" "${CMSIS_DIR}/RTOS" "${CMSIS_DIR}/RTOS2" \
      "${CMSIS_DIR}/Driver" "${CMSIS_DIR}/Utilities" "${CMSIS_DIR}/Device" "${CMSIS_DIR}/ARM.CMSIS.pdsc" "${CMSIS_DIR}/ARM.CMSIS.sha1"
      
cp "$OPENOCD_PATCH" "$OPENOCD_CODE/"
cd "$OPENOCD_CODE"
git apply --stat "$OPENOCD_PATCH"
git apply --check "$OPENOCD_PATCH"
git am < "$OPENOCD_PATCH"
./bootstrap
./configure
make
sudo make install
cd ..

   
   
#RTOS_DIR="FreeRTOS"
#RTOS_SOURCE="FreeRTOSv202012.02-LTS.zip"
#FATFS_DIR="FatFS"
#FATFS_SOURCE="ff14b.zip"

#wget "http://elm-chan.org/fsw/ff/arc/$FATFS_SOURCE"
#wget "https://github.com/FreeRTOS/FreeRTOS-LTS/releases/download/202012.02-LTS/$RTOS_SOURCE"

#unzip -d "${RTOS_DIR}" "$RTOS_SOURCE"
#cp -r "${RTOS_DIR}/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/" ./
#rm -r "$RTOS_DIR"
#cp -r "FreeRTOS-Kernel" "$RTOS_DIR"
#rm -r "FreeRTOS-Kernel"
#mkdir "${RTOS_DIR}/portable-temp"
#cp -r "${RTOS_DIR}/portable/GCC/ARM_CM4F/" "${RTOS_DIR}/portable/MemMang/" "${RTOS_DIR}/portable-temp"
#rm -r "${RTOS_DIR}/portable"
#cp -r "${RTOS_DIR}/portable-temp/" "${RTOS_DIR}/portable"
#rm -r "${RTOS_DIR}/portable-temp"
#unzip -d "${FATFS_DIR}" "$FATFS_SOURCE"


