// Our vertex shader is run once for each of these
// vectors, to determine the final position of the vertex
// on the screen and pass data off to the fragment shader.

precision mediump float;

// Our attributes, i.e. the arrays of vectors in the bunny mesh.
attribute vec3 aPosition;
attribute vec3 aNormal;

// This is passed from here to be used in `bunny.frag`.
varying vec3 vNormal;

uniform mat4 uProjection;
uniform mat4 uModel;
uniform mat4 uView;

void main() {
  vNormal = aNormal;

  // - `uProjection` will apply our perspective matrix, and
  // - `uView` will apply our camera transforms.
  // - `uModel` is unused here, but is traditionally used to
  //   move the object around the scene.
  gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
