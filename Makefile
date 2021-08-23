
SOURCE_DIR  = src/
INCLUDE_DIR = include/
INSTALL_DIR ?= /usr/local/

LIBRARY = libgnunetchat.so
SOURCES = gnunet_chat_lib.c\
  		  gnunet_chat_contact.c\
		  gnunet_chat_context.c\
		  gnunet_chat_file.c\
		  gnunet_chat_group.c\
  		  gnunet_chat_handle.c\
  		  gnunet_chat_invitation.c\
		  gnunet_chat_message.c\
		  gnunet_chat_util.c
		  
HEADERS = gnunet_chat_lib.h

LIBRARIES = gnunetarm\
            gnunetfs\
            gnunetmessenger\
            gnunetregex\
            gnunetutil

CC ?= gcc
LD ?= gcc
RM ?= rm

CFLAGS  += -fPIC -pedantic -Wall -Wextra -march=native -ggdb3
LDFLAGS += -shared

DEBUGFLAGS   = -O0 -D _DEBUG
RELEASEFLAGS = -O2 -D NDEBUG

SOURCE_FILES  = $(addprefix $(SOURCE_DIR), $(SOURCES))
OBJECT_FILES  = $(SOURCE_FILES:%.c=%.o)
HEADER_FILES  = $(addprefix $(INCLUDE_DIR), $(HEADERS))
LIBRARY_FLAGS = $(addprefix -l, $(LIBRARIES))

all: $(LIBRARY)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(LIBRARY)

release: CFLAGS += $(RELEASEFLAGS)
release: $(LIBRARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INCLUDE_DIR) $(LIBRARY_FLAGS)

$(LIBRARY): $(OBJECT_FILES)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBRARY_FLAGS)

.PHONY: install

install:
	install -m 755 $(LIBRARY) $(addprefix $(INSTALL_DIR), lib/)
	install -m 644 $(HEADER_FILES) $(addprefix $(INSTALL_DIR), include/gnunet/)

.PHONY: uninstall

uninstall:
	$(RM) -f $(addsuffix $(HEADERS), $(addprefix $(INSTALL_DIR), include/gnunet/))
	$(RM) -f $(addsuffix $(LIBRARY), $(addprefix $(INSTALL_DIR), lib/))

.PHONY: clean

clean:
	$(RM) -f $(LIBRARY)
	$(RM) -f $(OBJECT_FILES)