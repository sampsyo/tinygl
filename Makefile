TARGET := tinygl
SOURCE := tinygl.c
CFLAGS := -g
LIBFLAGS :=
DOCDIR := docs

ifeq ($(shell uname -s),Darwin)
	# glfw, as installed by Homebrew.
	CFLAGS += -I/usr/local/include
	LIBFLAGS += -L/usr/local/lib -framework OpenGL -lglfw
else
	LIBFLAGS += -lGL -lglfw -lm
endif

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(LIBFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -rf $(DOCDIR)

# The documentation.
$(DOCDIR): $(SOURCE)
	docco $^

# Deploy documentation to server.
.PHONY: deploy
RSYNCARGS := --compress --recursive --checksum --itemize-changes \
	--delete -e ssh
DEST := dh:domains/adriansampson.net/doc/tinygl
deploy: $(DOCDIR)
	mv $(DOCDIR)/$(SOURCE:%.c=%.html) $(DOCDIR)/index.html
	rsync $(RSYNCARGS) $(DOCDIR)/ $(DEST)
