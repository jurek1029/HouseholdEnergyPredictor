# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Programing/_Libs/esp2/esp-idf/components/bootloader/subproject"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix/tmp"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix/src"
  "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Programing/Magister/ESP32/Esp32TFL/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
