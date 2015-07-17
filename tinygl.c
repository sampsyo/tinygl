#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

const double PI = 3.141592;
const unsigned int NVERTICES = 13;
const unsigned int NDIMENSIONS = 3;

GLuint create_shader() {
  // The vertex shader.
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_shader = " \
    in vec3 position; \
    out vec4 myPos; \
    void main() { \
      myPos = position; \
      gl_Position = gl_Vertex; \
    } \
  ";
  glShaderSource(vshader, 1, &vertex_shader, 0);
  glCompileShader(vshader);

  // The fragment (pixel) shader.
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_shader = " \
    uniform float phase; \
    in vec4 myPos; \
    void main() { \
      float r2 = (myPos.x + 1.) * (myPos.x + 1.) + \
                 (myPos.y + 1.) * (myPos.y + 1.); \
      gl_FragColor = vec4((myPos.x + 1.) / r2, \
                          (myPos.y + 1.) / r2, \
                          phase, \
                          1.); \
    } \
  ";
  glShaderSource(fshader, 1, &fragment_shader, 0);
  glCompileShader(fshader);

  // Create a program that stitches the two shader stages together.
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program,vshader);
  glDeleteShader(vshader);
  glAttachShader(shader_program,fshader);
  glDeleteShader(fshader);

  // Link the program so it's ready to apply during drawing.
  glLinkProgram(shader_program);
  return shader_program;
}

int main(int argc, char **argv){
  // Set up the OpenGL context and the GLFW window that contains it. We'll
  // request a reasonably modern version of OpenGL, >= 3.2.
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

  // An array for the vertices of the shape we will to draw. We need 3
  // coordinates per point for a 3-dimensional space.
  float points[NVERTICES * NDIMENSIONS];

  GLuint loc_position = glGetAttribLocation(program, "position");

  GLuint vao_id, vbo_id;
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

  glVertexAttribPointer(loc_position, NDIMENSIONS, GL_FLOAT, GL_FALSE, 0,
                        points);
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
    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, NVERTICES);
    // glDisableClientState(GL_VERTEX_ARRAY);

    // Display the frame and get window events.
    glfwSwapBuffers(window);
    glfwPollEvents();

    // Advance the time counter for the next frame.
    t += 0.01;
  }

  // Teardown.
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
