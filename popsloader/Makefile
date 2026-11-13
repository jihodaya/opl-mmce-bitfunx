EE_BIN = popsloader.elf
EE_OBJS = loader.o
EE_LINKFILE = linkfile
EE_CFLAGS = -D_EE -DNEWLIB_PORT_AWARE -Os -G0 -Wall -ffunction-sections -fdata-sections
EE_LDFLAGS = -s -Wl,--gc-sections
EE_LIBS = -lfileXio

all: $(EE_BIN)

$(EE_BIN): linkfile

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
