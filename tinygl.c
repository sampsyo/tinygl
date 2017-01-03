// This is an example of a trivial OpenGL program with a pair of GLSL
// shaders. It uses OpenGL 4.1 and [GLFW][] 3. Check out [the GitHub
// project][gh] for background and instructions on how to build and run this
// program.
//
// The annotated code is by [Adrian Sampson][] and based on an original
// example by [Akihiko Fujii][]. The license is [MIT][].
//
// [glfw]: http://www.glfw.org
// [gh]: https://github.com/sampsyo/tinygl
// [Adrian Sampson]: http://www.cs.cornell.edu/~asampson/
// [Akihiko Fujii]: https://github.com/akihiko-fujii
// [mit]: https://opensource.org/licenses/MIT

#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>
#ifndef __APPLE__
#include <GL/glcorearb.h>
#endif

const double PI = 3.141592;
const unsigned int NVERTICES = 13;
const unsigned int NDIMENSIONS = 3;
const unsigned int BUFSIZE = 1024;  // For error logs.

// # The Shaders
//
// A *[shader][]* is a small program that runs on the GPU as part of the
// graphics rendering pipeline. The graphics driver JITs shader programs from
// source code, which are passed in as a string from the application running
// on the CPU. In OpenGL, shader programs are written in [GLSL][].
//
// Despite the name, shaders are not just for "shading" shapes to determine
// the color of their pixels. One kind of shader (the "fragment" shader in
// OpenGL terminology) does that, but there are several other kinds that
// define other aspects of the rendering pipeline (such as a "vertex" shader,
// which determines the position of each vertex in 3D space). Nothing really
// gets done at all on modern GPUs without shaders.
//
// [glsl]: https://www.opengl.org/documentation/glsl/
// [shader]: https://en.wikipedia.org/wiki/Shader

// ## Shader Error Checker
// Check for errors when compiling or linking the shader program. We need this
// 3 times: compiling the vertex shader, compiling the fragment (pixel)
// shader, and linking the two together into a complete OpenGL "program."

typedef void (*GetLogFunc)(GLuint, GLsizei, GLsizei *, GLchar *);
typedef void (*GetParamFunc)(GLuint, GLenum, GLint *);
void shader_error_check(GLuint object, const char *kind,
        GetLogFunc getLog, GetParamFunc getParam, GLenum param) {
  // Get the error/warning log using either `glGetShaderInfoLog` or
  // `glGetProgramInfoLog` (as `getLog`).
  GLchar log[BUFSIZE];
  GLsizei length;
  getLog(object, BUFSIZE, &length, log);
  if (length)
    fprintf(stderr, "%s log:\n%s", kind, log);

  // Get the status flag using either `glGetShaderiv` with the
  // `GL_COMPILE_STATUS` parameter,  or `glGetProgramiv` with `GL_LINK_STATUS`.
  GLint status;
  getParam(object, param, &status);
  if (status == GL_FALSE)
    exit(1);
}

// Now we compile and link the GLSL shader source.
GLuint create_shader() {
  // ## Vertex Shader
  // This is the program that runs on the GPU for every vertex of the shape
  // we want to draw.

  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_shader =
    // Shader programs need to specify the version of the language they're
    // targeting in the source code. "410" corresponds to OpenGL 4.1, which is
    // from 2010.
    "#version 410\n"

    // The `in` annotation indicates that this is an input from the CPU to the
    // vertex shader. We provide this below via a buffer. Confusingly, this is
    // a `vec4` while the data we provide is only 3-dimensional; there is a
    // 4th "W" dimension that has something to do with clipping.
    "in vec4 position;\n"

    // The `out` annotation here indicates that this variable will be used to
    // communicate *from* this vertex shader *to* the fragment shader.
    "out vec4 myPos;\n"

    "void main() {\n"
    // Send output to the fragment shader.
    "  myPos = position;\n"

    // Assigning to the special variable `gl_Position` (a member of the
    // built-in "named block" `gl_PerVertex`) constitutes the *output* of a
    // vertex shader.
    "  gl_Position = position;\n"
    "}\n"
  ;

  // Compile the vertex shader.
  glShaderSource(vshader, 1, &vertex_shader, 0);
  glCompileShader(vshader);
  shader_error_check(vshader, "vertex shader", glGetShaderInfoLog,
                     glGetShaderiv, GL_COMPILE_STATUS);

  // ## Fragment (Pixel) Shader
  // This shader program runs per pixel in the rendered shape. The fragment
  // shader is invoked many times *per* vertex shader. The vertex shader
  // program can communicate with the fragment shader to "fan out" data from
  // one stage to the next.

  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_shader =
    "#version 150\n"

    // The `uniform` annotation indicates an input from the CPU to the
    // fragment shader that is "global" for the invocation; it is not
    // per-vertex or per-pixel. We provide this value below using a
    // `glUniform*` call.
    "uniform float phase;\n"

    // The `in` annotation here matches with the `out` annotation on the
    // variable of the same name in the vertex shader.
    "in vec4 myPos;\n"

    // The `out`-annotated variable declared for a fragment shader is
    // *implicitly* the color of the pixel.
    "out vec4 color;\n"

    "void main() {\n"
    "  float r2 = (myPos.x + 1.) * (myPos.x + 1.) +\n"
    "             (myPos.y + 1.) * (myPos.y + 1.);\n"
    "  color = vec4((myPos.x + 1.) / r2,\n"
    "               (myPos.y + 1.) / r2,\n"
    "               phase,\n"
    "               1.);\n"
    "}\n"
  ;

  // Compile the fragment shader.
  glShaderSource(fshader, 1, &fragment_shader, 0);
  glCompileShader(fshader);
  shader_error_check(fshader, "fragment shader", glGetShaderInfoLog,
                     glGetShaderiv, GL_COMPILE_STATUS);

  // ## Shader Program
  // Create a program that stitches the two shader stages together.
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vshader);
  glDeleteShader(vshader);
  glAttachShader(shader_program, fshader);
  glDeleteShader(fshader);

  // Link the program so it's ready to apply during drawing.
  glLinkProgram(shader_program);
  shader_error_check(shader_program, "program", glGetProgramInfoLog,
                     glGetProgramiv, GL_LINK_STATUS);
  return shader_program;
}

// # Draw Stuff

// Set the vertex positions of our shape according to the current time step.
// We call this inside the draw loop to make the shape animate.
void update_vertices(float *points, float t) {
  for (int i = 0; i < NVERTICES; ++i) {
    float *coords = points + NDIMENSIONS * i;
    coords[0] = cos(360. / (NVERTICES - 1) * PI / 180. * i + t);
    coords[1] = sin(360. / (NVERTICES - 1) * PI / 180. * i + t);
    coords[2] = 0.;
  }
}

int main(int argc, char **argv) {
  // ## Setup
  // Set up the OpenGL context and the GLFW window that contains it. We'll
  // request a reasonably modern version of OpenGL, >= 4.1.
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(512, 512, "Look at Me!", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Which OpenGL version did we actually get?
  printf("OpenGL version %s\n", glGetString(GL_VERSION));

  // ## Shaders and Shader Variables

  // Compile the shader program.
  GLuint program = create_shader();

  // To communicate with the shader program, we need to get the "locations"
  // (just IDs, really) of the shader-language variables based on their names.
  // We will use these IDs to send data to the shader inside the draw loop.

  // First, the variable `phase`, for the fragment shader.
  GLuint loc_phase = glGetUniformLocation(program, "phase");
  assert(loc_phase != -1 && "could not find `phase` variable");

  // Then, the variable `position`, for the vertex shader.
  GLuint loc_position = glGetAttribLocation(program, "position");
  assert(loc_position != -1 && "could not find `position` variable");

  // ## Vertex Position Buffer
  // We next need to set up a buffer to hold the positions of the vertices in
  // the shape we want to draw. We actually need *two* regions of memory: one
  // on the CPU (just an ordinary `float[]`) and one on the GPU. The GPU-side
  // one is called a Vertex Buffer Object (VBO) and it is wrapped in a
  // structure called a Vertex Array Object (VAO) for communication between
  // the CPU and GPU.

  // An array for the vertices of the shape we will to draw. We need 3
  // coordinates per point for a 3-dimensional space.
  float points[NVERTICES * NDIMENSIONS];

  // Create an OpenGL Vertex Array Object (VAO), which contains a bundle of
  // state to be passed with one draw call to the vertex shader.
  GLuint array;
  glGenVertexArrays(1, &array);

  // Also create a Vertex Buffer Object (VBO), which (as the name implies)
  // holds the actual *data* to be passed to the vertex shader. A VAO can have
  // multiple VBOs, but we just use one here (for the position).
  GLuint buffer;
  glGenBuffers(1, &buffer);

  // To set up the buffer, we "bind" to the "target" GL_ARRAY_BUFFER, which
  // lets us use other calls to manipulate it.
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // Allocate space for the buffer and fill it with zeros. (GL_DYNAMIC_DRAW
    // indicates that we may change the contents of this buffer later. This is
    // just a performance hint; nothing will break if you change this to
    // GL_STATIC_DRAW.)
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_DYNAMIC_DRAW);

    glBindVertexArray(array);
      // Associate the `position` variable with our buffer object in the array
      // object. This call *implictly* refers to the currently-bound array
      // object and buffer object, and *explicitly* refers to the `position`
      // variable in the shader program via its "location". (A "vertex
      // attribute" is just a fancy name for an `in`-annotated variable in a
      // vertex shader.)
      glVertexAttribPointer(loc_position, NDIMENSIONS, GL_FLOAT,
                            GL_FALSE, 0, 0);

      // For some unknowable reason, you also have to "enable" the variable
      // ("attribute") in the array object to make it actually work.
      glEnableVertexAttribArray(loc_position);
    glBindVertexArray(0);  // Unbind.

  glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind.

  // ## Draw Loop

  // Initialize the time to zero. We'll update it on every trip
  // through the loop.
  float t = 0;

  // The main draw loop terminates when the user closes the window.
  while (!glfwWindowShouldClose(window)) {
    // Poll for user input.
    glfwPollEvents();

    // ### Update Phase Variable

    // Assign to a shader "uniform" variable. A "uniform" is a value passed
    // from the CPU to the GPU that is the same for all invocations (i.e., all
    // vertices).
    // Morally: `phase = sin(4 * t);`
    glUniform1f(loc_phase, sin(4 * t));

    // ### Update Vertex Positions

    // Position the shape by updating the `points` array.
    update_vertices(points, t);

    // Set the contents of the `position` vertex list. This is a bit more
    // complicated than the uniform above: we have to "bind" the buffer first
    // so we can manipulate it.
    // Morally: `position = points;`
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
      // Copy the contents of our CPU-side `points` to the bound GPU buffer.
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind.

    // Advance the time counter for the next frame.
    t += 0.01;

    // ### Draw
    // Actually draw something!

    // Clear the frame so we can draw to it.
    glClear(GL_COLOR_BUFFER_BIT);
    // Use our shader program for the next draw call.
    glUseProgram(program);
    // Use the VAO to communicate with the shaders.
    glBindVertexArray(array);
      // Draw!
      glDrawArrays(GL_TRIANGLE_FAN, 0, NVERTICES);
    glBindVertexArray(0);  // Unbind.
    // Display the frame to the screen.
    glfwSwapBuffers(window);
  }

  // ## Teardown

  // Tear down the windowing system and deallocate the OpenGL resources.
  glfwDestroyWindow(window);
  glDeleteProgram(program);
  glDeleteBuffers(1, &buffer);
  glDeleteVertexArrays(1, &array);
  glfwTerminate();
  return 0;
}
