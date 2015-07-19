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
