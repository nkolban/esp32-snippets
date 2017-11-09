#
# Main Makefile. This is basically the same as a component makefile.
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component_common.mk. By default, 
# this will take the sources in the src/ directory, compile them and link them into 
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
CFLAGS += -DCS_PLATFORM=15 -DMG_DISABLE_DIRECTORY_LISTING=1 -DMG_DISABLE_DAV=1 -DMG_DISABLE_CGI=1
include $(IDF_PATH)/make/component_common.mk
