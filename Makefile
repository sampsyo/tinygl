SOURCE := tinygl.c
CFLAGS := -g
DOCDIR := docs

ifeq ($(shell uname -s),Darwin)
	# glfw, as installed by Homebrew.
	CFLAGS += -I/usr/local/include
	LDLIBS := -L/usr/local/lib -framework OpenGL -lglfw
else
	LDLIBS := -lGL -lglfw -lm
endif

TARGET := $(basename $(SOURCE))
$(TARGET): $(SOURCE)

.PHONY: clean
clean:
	$(RM) -r $(TARGET) $(DOCDIR)

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
