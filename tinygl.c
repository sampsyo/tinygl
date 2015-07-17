#include <GLFW/glfw3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl3ext.h>
#include <stdlib.h>
#include <math.h>

const double pi = 3.141592;

GLuint create_shader() {
  // The vertex shader.
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_shader = " \
    attribute vec4 position; \
    varying vec4 myPos; \
    void main() { \
      myPos = position; \
      gl_Position = position; \
    } \
  ";
  glShaderSource(vshader, 1, &vertex_shader, 0);
  glCompileShader(vshader);

  // The fragment (pixel) shader.
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_shader = " \
    uniform float phase; \
    varying vec4 myPos; \
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

  glBindAttribLocation(shader_program, 0, "position");
  glLinkProgram(shader_program);

  return shader_program;
}

int main(int argc, char **argv){
  // Set up the OpenGL context and the GLFW window that contains it.
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(512, 512, "Look at Me!", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glClearColor(0., 0., 0.5, 1.);

  // Compile the shader program.
  GLuint program = create_shader();

  // An array for the vertices of the shape we will to draw.
  float points[13 * 3];

  // Initialize the time to zero. We'll update it on every trip
  // through the loop.
  float t = 0;

  // The main draw loop. (Terminates when the user closes the window.)
  while (!glfwWindowShouldClose(window)) {
    // Position (rotate) the shape by updating its vertices.
    for(int i = 0; i < 13; ++i){
      points[3 * i] = cos(360. / 12. * pi / 180. * i + t);
      points[3 * i + 1] = sin(360. / 12. * pi / 180. * i + t);
      points[3 * i + 2] = 0.;
    }

    // Clear the frame so we can start drawing to it.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shader program to render the shape.
    glUseProgram(program);

    // Assign to a shader "uniform" variable: phase = 4 * t
    GLuint phase_id = glGetUniformLocation(program, "phase");
    glUniform1f(phase_id, sin(4 * t));

    // Now draw the shape using the shader.
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 13);
    glDisableClientState(GL_VERTEX_ARRAY);

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
