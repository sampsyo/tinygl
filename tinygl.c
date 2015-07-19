#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

const double PI = 3.141592;
const unsigned int NVERTICES = 13;
const unsigned int NDIMENSIONS = 3;
const unsigned int BUFSIZE = 1024;  // For error logs.

// Check for errors when compiling or linking the shader program. We need this
// 3 times: compiling the vertex shader, compiling the fragment (pixel)
// shader, and linking the two together into a complete OpenGL "program".
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

// Compile and link the GLSL shader source.
GLuint create_shader() {
  // The vertex shader.
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
  glShaderSource(vshader, 1, &vertex_shader, 0);
  glCompileShader(vshader);
  shader_error_check(vshader, "vertex shader", glGetShaderInfoLog,
                     glGetShaderiv, GL_COMPILE_STATUS);

  // The fragment (pixel) shader.
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
  glShaderSource(fshader, 1, &fragment_shader, 0);
  glCompileShader(fshader);
  shader_error_check(fshader, "fragment shader", glGetShaderInfoLog,
                     glGetShaderiv, GL_COMPILE_STATUS);

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

int main(int argc, char **argv){
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

  // Compile the shader program.
  GLuint program = create_shader();

  // Get the "location" (just an ID) of a "uniform" variable in the shader
  // program. We will use this to communicate to the shader inside the draw
  // loop.
  GLuint loc_phase = glGetUniformLocation(program, "phase");
  assert(loc_phase != -1 && "could not find `phase` variable");

  // An array for the vertices of the shape we will to draw. We need 3
  // coordinates per point for a 3-dimensional space.
  float points[NVERTICES * NDIMENSIONS];

  GLuint loc_position = glGetAttribLocation(program, "position");
  assert(loc_position != -1 && "could not find `position` variable");

  GLuint vao_id, vbo_id;
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_DYNAMIC_DRAW);
  // WTF is DYNAMIC_DRAW?

  glVertexAttribPointer(loc_position, NDIMENSIONS, GL_FLOAT, GL_FALSE, 0,
                        NULL);
  glEnableVertexAttribArray(loc_position);

  // Initialize the time to zero. We'll update it on every trip
  // through the loop.
  float t = 0;

  // The main draw loop. (Terminates when the user closes the window.)
  while (!glfwWindowShouldClose(window)) {
    // Position (rotate) the shape by updating its vertices.
    for (int i = 0; i < NVERTICES; ++i) {
      float *coords = points + NDIMENSIONS * i;
      coords[0] = cos(360. / (NVERTICES - 1) * PI / 180. * i + t);
      coords[1] = sin(360. / (NVERTICES - 1) * PI / 180. * i + t);
      coords[2] = 0.;
    }

    // Clear the frame so we can start drawing to it.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shader program to render the shape.
    glUseProgram(program);

    // Assign to a shader "uniform" variable. A "uniform" is a value passed
    // from the CPU to the GPU that is the same for all invocations (i.e., all
    // vertices). So this line essentially performs the assignment:
    // phase = sin(4 * t)
    glUniform1f(loc_phase, sin(4 * t));

    // Now draw the shape using the shader.
    // glEnableClientState(GL_VERTEX_ARRAY);
    // WTF update the data in the points array?
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);

    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, NVERTICES);
    // glDisableClientState(GL_VERTEX_ARRAY);

    // Display the frame and get window events.
    glfwSwapBuffers(window);
    glfwPollEvents();

    // Advance the time counter for the next frame.
    t += 0.01;
  }

  // WTF destroy VAO and VBO

  // Teardown.
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
