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
    attribute vec3 normal; \
    attribute vec4 color; \
    varying vec4 vColor; \
    uniform float uniID; \
    varying vec3 varyingnormal; \
    void main() { \
      vColor = position; \
      gl_Position = position; \
      varyingnormal = vec3(position.x,position.y,0.); \
    } \
  ";
  glShaderSource(vshader, 1, &vertex_shader, 0);
  glCompileShader(vshader);

  // The fragment (pixel) shader.
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_shader = " \
    uniform float uniID; \
    varying vec4 vColor; \
    varying vec3 varyingnormal; \
    void main() { \
      float r2 = (vColor.x+1.)*(vColor.x+1.)+(vColor.y+1.)*(vColor.y+1.); \
      gl_FragColor = vec4((vColor.x+1.)/r2,(vColor.y+1.)/r2,uniID,1.); \
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
  // Set up the vertices for the shape we want to draw.
  float *points = (float *)calloc(13*3,sizeof(float));
  for(int i=0;i<13;i++){
    points[3*i] = cos(360./12.*pi/180.*i);
    points[3*i+1] = sin(360./12.*pi/180.*i);
    points[3*i+2] = 0.;
  }

  // Set up the OpenGL context and the GLFW window that contains it.
  glfwInit();
  GLFWwindow* window = glfwCreateWindow(640, 480, "Look at Me!", NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glClearColor(0.,0.,0.5,1.);

  // Compile the shader program.
  GLuint program = create_shader();

  // Initialize the time to zero. We'll update it on every trip
  // through the loop.
  float t = 0;

  while (!glfwWindowShouldClose(window)) {
    // Clear the frame so we can start drawing to it.
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shader program.
    glUseProgram(program);
    float univar = sin(4.*t);
    GLuint uniID = glGetUniformLocation(program, "uniID");
    glUniform1f(uniID, univar);

    // Now draw the shape using the shader.
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points);
    glDrawArrays(GL_TRIANGLE_FAN,0,13);
    glDisableClientState(GL_VERTEX_ARRAY);

    // Rotate the shape and update the timestep.
    for(int i=0;i<13;i++){
      points[3*i] = cos(360./12.*pi/180.*i+t);
      points[3*i+1] = sin(360./12.*pi/180.*i+t);
      points[3*i+2] = 0.;
    }
    t += 0.01;

    // Display the frame and get window events.
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Teardown.
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
