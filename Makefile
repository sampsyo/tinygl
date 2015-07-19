TARGET := tinygl
SOURCE := tinygl.c
CFLAGS := -I/usr/local/include -g -std=gnu11
LIBFLAGS := -L/usr/local/lib -lglfw3 -framework OpenGL
DOCS := $(SOURCE:%.c=%.html)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(LIBFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGET) $(DOCS)

# The documentation.
$(DOCS): $(SOURCE)
	docco $^
