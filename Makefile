TARGET := tinygl
SOURCE := tinygl.c
CFLAGS := -I/usr/local/include -g -std=gnu11
LIBFLAGS := -L/usr/local/lib -lglfw3 -framework OpenGL
DOCDIR := docs

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
