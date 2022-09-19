
VERSION     = 0.1.0
TARGET_NAME = gnunetchat
SOURCE_DIR  = src/
INCLUDE_DIR = include/
TESTS_DIR   = tests/
INSTALL_DIR ?= /usr/local/
DOXYGEN_DIR = doc/

PACKAGE = lib$(TARGET_NAME)
LIBRARY = lib$(TARGET_NAME).so
SOURCES = gnunet_chat_lib.c\
          gnunet_chat_account.c\
          gnunet_chat_contact.c\
          gnunet_chat_context.c\
          gnunet_chat_file.c\
          gnunet_chat_group.c\
          gnunet_chat_handle.c\
          gnunet_chat_invitation.c\
          gnunet_chat_lobby.c\
          gnunet_chat_message.c\
          gnunet_chat_uri.c\
          gnunet_chat_util.c
		  
HEADERS = gnunet_chat_lib.h

TESTS   = test_gnunet_chat_handle.c\
		  test_gnunet_chat_lobby.c\
		  test_gnunet_chat_file.c

LIBRARIES = gnunetarm\
            gnunetfs\
            gnunetidentity\
            gnunetgns\
            gnunetmessenger\
            gnunetnamestore\
            gnunetregex\
            gnunetutil

DIST_FILES = Makefile\
             AUTHORS\
             CHANGES.md\
             COPYING\
             Doxyfile\
             HOWTO.md\
             README.md

GNU_CC	?= gcc
GNU_LD	?= gcc
GNU_RM	?= rm
GNU_CP  ?= cp
GNU_TAR ?= tar
DOXYGEN	?= doxygen

CFLAGS  += -fPIC -pedantic -Wall -Wextra -ggdb3
LDFLAGS += -shared

DEBUGFLAGS   = -O0 -D _DEBUG
RELEASEFLAGS = -O2 -D NDEBUG

DIST_DIR = $(PACKAGE)-$(VERSION)/
DIST_TAR = $(PACKAGE)-$(VERSION).tar.gz

SOURCE_FILES  = $(addprefix $(SOURCE_DIR), $(SOURCES))
OBJECT_FILES  = $(SOURCE_FILES:%.c=%.o)
HEADER_FILES  = $(addprefix $(INCLUDE_DIR), $(HEADERS))
TEST_FILES    = $(addprefix $(TESTS_DIR), $(TESTS))
TEST_CASES    = $(TEST_FILES:%.c=%.test)
LIBRARY_FLAGS = $(addprefix -l, $(LIBRARIES))
TEST_FLAGS    = $(LIBRARY_FLAGS) -lcheck -l$(TARGET_NAME) -L.

all: $(LIBRARY)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(LIBRARY)

release: CFLAGS += $(RELEASEFLAGS)
release: $(LIBRARY)

%.o: %.c
	$(GNU_CC) $(CFLAGS) -c $< -o $@ -I $(INCLUDE_DIR)

$(LIBRARY): $(OBJECT_FILES)
	$(GNU_LD) $(LDFLAGS) $^ -o $@ $(LIBRARY_FLAGS)

check: $(LIBRARY) $(TEST_CASES)
	$(foreach TEST_CASE,$(TEST_CASES),LD_LIBRARY_PATH=. ./$(TEST_CASE);)

%.test: %.c
	ln -s ../$(INCLUDE_DIR) $(addprefix $(TESTS_DIR), gnunet)
	$(GNU_CC) $(CFLAGS) $< -o $@ -I $(TESTS_DIR) $(TEST_FLAGS)
	$(GNU_RM) $(addprefix $(TESTS_DIR), gnunet)

.PHONY: install

install:
	install -m 755 $(LIBRARY) $(addprefix $(INSTALL_DIR), lib/)
	mkdir -p $(addprefix $(INSTALL_DIR), include/gnunet/)
	install -m 644 $(HEADER_FILES) $(addprefix $(INSTALL_DIR), include/gnunet/)

.PHONY: uninstall

uninstall:
	$(GNU_RM) -f $(addsuffix $(HEADERS), $(addprefix $(INSTALL_DIR), include/gnunet/))
	$(GNU_RM) -f $(addsuffix $(LIBRARY), $(addprefix $(INSTALL_DIR), lib/))

.PHONY: dist

dist: clean
	mkdir $(DIST_DIR)
	$(GNU_CP) -r $(INCLUDE_DIR) $(DIST_DIR)
	$(GNU_CP) -r $(SOURCE_DIR) $(DIST_DIR)
	$(GNU_CP) -r $(TESTS_DIR) $(DIST_DIR)
	$(foreach DIST_FILE,$(DIST_FILES),$(GNU_CP) $(DIST_FILE) $(DIST_DIR);)
	$(GNU_TAR) -czf $(DIST_TAR) $(DIST_DIR)
	$(GNU_RM) -r $(DIST_DIR)

.PHONY: docs

docs:
	mkdir -p $(DOXYGEN_DIR)
	$(DOXYGEN)

.PHONY: clean

clean:
	$(GNU_RM) -f $(LIBRARY)
	$(GNU_RM) -f $(OBJECT_FILES)
	$(GNU_RM) -f $(TEST_CASES)
	$(GNU_RM) -f $(DIST_TAR)
