tinygl.c
========

A minuscule OpenGL example with a GLSL shader. This is about as small as it can get and still show how a shader program works. I've tried to comment exhaustively so that graphics-clueless people like myself have a hope of following along.

See [the source displayed in a literate-programming style][rendered] for nicer reading.

[rendered]: http://adriansampson.net/doc/tinygl/


Run It
------

The Makefile may need adaptation for your platform (I set it up for macOS). You will need OpenGL 4.1 (or later, I think) and [GLFW 3.x][glfw]. Then:

    $ make
    $ ./tinygl

and enjoy the lovely colors.

[glfw]: http://www.glfw.org


Why?
----

The Web is chock-full of inconsistent nonsense about graphics programming. The overwhelming majority of text about OpenGL seems to be have been written for or by novices misguidedly trying to write their first video game from the ground up by copy n' paste from forum posts. It doesn't help that the programming model is an unsafe mess, the domain is full of unfamiliar jargon, and the evolving standards mean that most example code is out of date.

I wrote this small example so I could understand, with no ambiguity, precisely what goes on in a minimal OpenGL program. If you too are a greenhorn w.r.t. graphics programming, I hope the code and comments serve as an inroad to understanding that nest of ridiculousness.

The example demonstrates:

* Setting up a drawing context and a window using the [GLFW][] library.
* A draw loop that animates vertex positions and colors over time.
* Vertex and fragment shader programs written in [GLSL][].
* Communicating between the host and the GPU programs using "vertex attributes," "vertex array objects," "vertex buffer objects," and "uniforms."
* Communicating between the vertex and fragment shaders using the `in` and `out` GLSL qualifiers.

It does not (yet) demonstrate:

* Tessellation shaders.
* Geometry shaders.
* Multiple vertex attributes (currently, the only attribute is position).
* Using uniforms for view and projection transforms (a common pattern).

Because I am totally clueless, I probably got some stuff wrong; please [let me know][newbug] if you see any mistakes.

[GLSL]: https://www.opengl.org/documentation/glsl/
[newbug]: https://github.com/sampsyo/tinygl/issues/new


Credits
-------

This is a cleaned-up version of [`nothing.c`][nothing.c] from Akihiko Fujii's [`glfw_samples`][glfw_samples] repository. The license is MIT.

[nothing.c]: https://github.com/akihiko-fujii/glfw_samples/blob/master/src/basic/nothing.c
[glfw_samples]: https://github.com/akihiko-fujii/glfw_samples
