precision mediump float;

// Sets the color of the current fragment (pixel)
// to display the normal at the current position.
// Using `abs()` to prevent negative values, which
// would just end up being black.

varying vec3 vNormal;

void main() {
  gl_FragColor = vec4(abs(vNormal), 1.0);
}
