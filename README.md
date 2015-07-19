tinygl
======

A minuscule OpenGL example with a GLSL shader. This is about as small as it can get and still show how a shader program works. I've tried to comment exhaustively so that graphics-clueless people like myself have a hope of following along.

It's a cleaned-up version of [`nothing.c`][nothing.c] from Akihiko Fujii's [`glfw_samples`][glfw_samples] repository. The license is MIT.

The Makefile may need adaptation for your platform (I set it up for OS X). You will need OpenGL 4.1 (or later) and [GLFW 3.x][glfw]. Then:

    $ make
    $ ./tinygl

and enjoy the lovely colors.

[nothing.c]: https://github.com/akihiko-fujii/glfw_samples/blob/master/src/basic/nothing.c
[glfw_samples]: https://github.com/akihiko-fujii/glfw_samples
[glfw]: http://www.glfw.org
