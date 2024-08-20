# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.2/components/bootloader/subproject"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/tmp"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/src"
  "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Melao/SoftwareEmbarcado/projeto16Timer/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
