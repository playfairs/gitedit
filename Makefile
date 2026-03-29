CC = clang
CFLAGS = -std=c17 -Wall -Wextra -Werror -O2 -g
LDFLAGS = -lncurses
INCLUDES = -Iinclude -Ideps
SRCDIR = src
DEPDIR = deps
BINDIR = bin
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/utils.c $(SRCDIR)/backup.c $(SRCDIR)/git_objects.c $(SRCDIR)/editor.c $(SRCDIR)/tui.c $(SRCDIR)/process.c
DEPS = $(DEPDIR)/miniz_simple.c
OBJECTS = $(SOURCES:.c=.o) $(DEPS:.c=.o)
TARGET = $(BINDIR)/gitedit

all: $(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DEPDIR)/%.o: $(DEPDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f gitedit.log
	rm -rf .git_backup_*

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	chmod 755 /usr/local/bin/gitedit

uninstall:
	rm -f /usr/local/bin/gitedit

test-compile: clean
	$(MAKE) all
	@echo "Compilation test successful!"

debug: CFLAGS += -DDEBUG -fsanitize=address -fsanitize=undefined
debug: clean $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all          - Build the gitedit executable"
	@echo "  clean        - Remove build artifacts and backups"
	@echo "  install      - Install gitedit to /usr/local/bin"
	@echo "  uninstall    - Remove gitedit from /usr/local/bin"
	@echo "  test-compile - Clean build and test compilation"
	@echo "  debug        - Build with debug flags and sanitizers"
	@echo "  help         - Show this help message"

.PHONY: all clean install uninstall test-compile debug help