
TARGET_NAME = gnunetchat
SOURCE_DIR  = src/
INCLUDE_DIR = include/
TESTS_DIR   = tests/
INSTALL_DIR ?= /usr/local/

LIBRARY = lib$(TARGET_NAME).so
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

TESTS   = test_gnunet_chat_handle.c

LIBRARIES = gnunetarm\
            gnunetfs\
            gnunetmessenger\
            gnunetregex\
            gnunetutil

GNU_CC ?= gcc
GNU_LD ?= gcc
GNU_RM ?= rm

CFLAGS  += -fPIC -pedantic -Wall -Wextra -march=native -ggdb3
LDFLAGS += -shared

DEBUGFLAGS   = -O0 -D _DEBUG
RELEASEFLAGS = -O2 -D NDEBUG

SOURCE_FILES  = $(addprefix $(SOURCE_DIR), $(SOURCES))
OBJECT_FILES  = $(SOURCE_FILES:%.c=%.o)
HEADER_FILES  = $(addprefix $(INCLUDE_DIR), $(HEADERS))
TEST_FILES    = $(addprefix $(TESTS_DIR), $(TESTS))
TEST_CASES    = $(TEST_FILES:%.c=%.test)
LIBRARY_FLAGS = $(addprefix -l, $(LIBRARIES))
TEST_FLAGS    = $(LIBRARY_FLAGS) -lcheck -l$(TARGET_NAME)

all: $(LIBRARY)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(LIBRARY)

release: CFLAGS += $(RELEASEFLAGS)
release: $(LIBRARY)

%.o: %.c
	$(GNU_CC) $(CFLAGS) -c $< -o $@ -I $(INCLUDE_DIR) $(LIBRARY_FLAGS)

$(LIBRARY): $(OBJECT_FILES)
	$(GNU_LD) $(LDFLAGS) $^ -o $@ $(LIBRARY_FLAGS)

check: $(TEST_CASES)
	./$(TEST_CASES)

%.test: %.c
	$(GNU_CC) $(CFLAGS) $< -o $@ -I $(INCLUDE_DIR) $(TEST_FLAGS)

.PHONY: install

install:
	install -m 755 $(LIBRARY) $(addprefix $(INSTALL_DIR), lib/)
	mkdir -p $(addprefix $(INSTALL_DIR), include/gnunet/)
	install -m 644 $(HEADER_FILES) $(addprefix $(INSTALL_DIR), include/gnunet/)

.PHONY: uninstall

uninstall:
	$(GNU_RM) -f $(addsuffix $(HEADERS), $(addprefix $(INSTALL_DIR), include/gnunet/))
	$(GNU_RM) -f $(addsuffix $(LIBRARY), $(addprefix $(INSTALL_DIR), lib/))

.PHONY: clean

clean:
	$(GNU_RM) -f $(LIBRARY)
	$(GNU_RM) -f $(OBJECT_FILES)
	$(GNU_RM) -f $(TEST_CASES)
