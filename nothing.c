#include <stdio.h>
/* #include <OpenGL/gl.h> */
#include "glfw.h"
#include <OpenGL/glext.h>
#include <OpenGL/gl3ext.h>
#include <stdlib.h>
#include <math.h>

const double pi = 3.141592;
float t;
float univar;
GLuint uniID;

static const GLchar *vertex_shader_source= 
  {
    "attribute vec4 position;attribute vec3 normal;attribute vec4 color;varying vec4 vColor;\n"
    "uniform float uniID;varying vec3 varyingnormal;\n"
    "void main()\n"
    "{vColor = position;   gl_Position = position; varyingnormal = vec3(position.x,position.y,0.);}\n"
  };

static const GLchar *fragment_shader_source=
  {
    "uniform float uniID;varying vec4 vColor;\nvarying vec3 varyingnormal;\n"
    "void main()\n"
    "{ float r2 = (vColor.x+1.)*(vColor.x+1.)+(vColor.y+1.)*(vColor.y+1.);"
    "gl_FragColor = vec4((vColor.x+1.)/r2,(vColor.y+1.)/r2,uniID,1.);}\n"
  };

GLuint shader_program;
static GLboolean printShaderInfoLog(GLuint shader, const char *str);
static GLboolean printProgramInfoLog(GLuint program);

void createshader(){

  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_source, 0); /* third argument is pointer to shader source */
  glCompileShader(vshader);
  printShaderInfoLog(vshader, "vertex shader");

  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_source, 0); /* third argument is pointer to shader source */
  glCompileShader(fshader);
  printShaderInfoLog(fshader, "fragment shader");


  shader_program = glCreateProgram();
  glAttachShader(shader_program,vshader);
  glDeleteShader(vshader);
  glAttachShader(shader_program,fshader);
  glDeleteShader(fshader);

  glBindAttribLocation(shader_program, 0, "position");
  /* glBindFragDataLocation(shader_program, 0, "fc"); */
  glLinkProgram(shader_program);
  printProgramInfoLog(shader_program);
}

float *points;

void prepare_dodecagon(){

  points = (float *)calloc(13*3,sizeof(float));

  int i;
  for(i=0;i<13;i++){
    points[3*i] = cos(360./12.*pi/180.*i);
    points[3*i+1] = sin(360./12.*pi/180.*i);
    points[3*i+2] = 0.;
  }

  for(i=0;i<12;i++){
    printf("points %d:%lf %lf %lf\n",i,points[3*i],points[3*i+1],points[3*i+2]);
  }

}

void draw_dodecagon(){

  glEnableClientState(GL_VERTEX_ARRAY); 
  glVertexPointer(3, GL_FLOAT, 0, points); /* 3 float values for each vertex, offset 0*/
  glDrawArrays(GL_TRIANGLE_FAN,0,13);	     /* 12 vertices */
  glDisableClientState(GL_VERTEX_ARRAY); 
}

void rotate_dodecagon(){

  int i;
  for(i=0;i<13;i++){
    points[3*i] = cos(360./12.*pi/180.*i+t);
    points[3*i+1] = sin(360./12.*pi/180.*i+t);
    points[3*i+2] = 0.;
  }
  t += 1e-2;
}

int main(int argc, char **argv){

  t = 0.;

  glfwInit();

  /* glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3); */
  /* glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2); */
  /* glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); */

  glfwOpenWindow(0,0,0,0,0,0,0,0,GLFW_WINDOW);

  glfwSetWindowTitle("Do Nothing");

  createshader();

  glfwSwapInterval(1);

  glClearColor(0.,0.,0.5,1.);	/* blue background */

  prepare_dodecagon();

  while(glfwGetWindowParam(GLFW_OPENED)){

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    univar = sin(4.*t);

    /* this is called after glUseProgram() is called. */
    uniID = glGetUniformLocation(shader_program,"uniID");
    glUniform1f(uniID,univar);

    draw_dodecagon();

    rotate_dodecagon();

    glfwSwapBuffers();
  }

  glfwTerminate();

  return 0;
}


static GLboolean printShaderInfoLog(GLuint shader, const char *str)
{
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) printf("shader compile ERROR\n");
  
  GLsizei bufSize;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1)
    {
      GLchar infoLog[1024];
      GLsizei length;
      glGetShaderInfoLog(shader, bufSize, &length, infoLog);
      printf("ERROR:%s\n",infoLog);
    }
  
  return (GLboolean)status;
}

static GLboolean printProgramInfoLog(GLuint program)
{
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) printf("Link error\n");
  
  GLsizei bufSize;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH , &bufSize);
  
  if (bufSize > 1)
  {
    GLchar infoLog[1024];
    GLsizei length;
    glGetProgramInfoLog(program, bufSize, &length, infoLog);
    fprintf(stderr,"%s",infoLog);
  }
  return (GLboolean)status;
}
