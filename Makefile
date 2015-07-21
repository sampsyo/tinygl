TARGET := tinygl
SOURCE := tinygl.c
CFLAGS := -g
LIBFLAGS :=
DOCDIR := docs

ifeq ($(shell uname -s),Darwin)
	# glfw3, as installed by Homebrew.
	CFLAGS += -I/usr/local/include
	LIBFLAGS += -L/usr/local/lib -framework OpenGL -lglfw3
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

.PHONY: pubdocs
HTMLFILE := $(SOURCE:%.c=%.html)
pubdocs: $(DOCDIR)
	git checkout gh-pages
	cp $(DOCDIR)/$(HTMLFILE) index.html
	git commit -m "Sync docco output" index.html
	git checkout master
