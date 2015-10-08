var Geometry = require('gl-geometry')
var fit      = require('canvas-fit')
var glShader = require('gl-shader')
var mat4     = require('gl-mat4')
var normals  = require('normals')
var bunny    = require('bunny')
var canvasOrbitCamera = require('canvas-orbit-camera')
var glContext = require('gl-context')
var createVAO = require('gl-vao')

var VERTEX_SHADER =
"precision mediump float;" +
"attribute vec3 aPosition;" +
"attribute vec3 aNormal;" +
"varying vec3 vNormal;" +
"uniform mat4 uProjection;" +
"uniform mat4 uModel;" +
"uniform mat4 uView;" +
"void main() {" +
  "vNormal = aNormal;" +
  "gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);" +
"}";

var FRAGMENT_SHADER =
"precision mediump float;" +
"varying vec3 vNormal;" +
"void main() {" +
  "gl_FragColor = vec4(abs(vNormal), 1.0);" +
"}";

function get_shader(gl) {
  return glShader(gl, VERTEX_SHADER, FRAGMENT_SHADER);
}

function projection_matrix(out, width, height) {
  var aspectRatio = width / height;
  var fieldOfView = Math.PI / 4;
  var near = 0.01;
  var far  = 100;

  mat4.perspective(
    out,
    fieldOfView,
    aspectRatio,
    near,
    far
  )
}

function init_demo(container) {
  // Create a <canvas> element to do our drawing in. Then set it up to fill
  // the container and resize when the window resizes.
  var canvas = container.appendChild(document.createElement('canvas'))
  window.addEventListener('resize', fit(canvas), false)

  // Attach a `canvas-orbit-camera` thing, which handles user input for
  // manipulating the view.
  var camera = canvasOrbitCamera(canvas)

  // Initialize the OpenGL context with our rendering function.
  var gl = glContext(canvas, render);

  // Compile and link the shader program.
  var shader = get_shader(gl);

  // The `gl-geometry` library provides a wrapper for OpenGL buffers and such
  // to help with loading models and communicating with the shader.
  var geometry = Geometry(gl);

  // This is *super* black-boxy, but it assigns a couple of shader variables
  // according to the object we're rendering. We should eventually break this
  // out into the raw OpenGL code for clarity.
  geometry.attr('aPosition', bunny.positions);
  geometry.attr('aNormal', normals.vertexNormals(
      bunny.cells
    , bunny.positions
  ));
  geometry.faces(bunny.cells);

  // TODO
  geometry._vao = createVAO(
      geometry.gl
    , geometry._attributes
    , geometry._index
  )

  // Create the base matrices to be used
  // when rendering the bunny. Alternatively, can
  // be created using `new Float32Array(16)`
  var projection = mat4.create();
  var model      = mat4.create();
  var view       = mat4.create();

  // The main rendering loop.
  function render() {
    // TODO TODO Remove this!
    if(!shader.program) {
      console.log("relink!");
      shader._relink()
    }

    // Get the current size of the canvas.
    var width = gl.drawingBufferWidth;
    var height = gl.drawingBufferHeight;

    // Handle user input and update the resulting camera view matrix.
    camera.view(view);
    camera.tick();

    // Update the projection matrix for translating to 2D screen space.
    projection_matrix(projection, width, height);

    // Draw on the whole canvas.
    gl.viewport(0, 0, width, height);

    // Rendering flags.
    gl.enable(gl.DEPTH_TEST);  // Prevent triangle overlap.
    gl.enable(gl.CULL_FACE);  // Triangles not visible from behind.

    // Use our shader.
    gl.useProgram(shader.program);

    // TODO TODO TODO
    geometry._vao.bind();
    for (var i = 0; i < geometry._keys.length; i++) {
      var attr = shader.attributes[geometry._keys[i]];
      if (attr) attr.location = i;
    }

    // Set the shader parameters.
    shader.uniforms.uProjection = projection;
    shader.uniforms.uView = view;
    shader.uniforms.uModel = model;

    // Draw it!
    geometry.draw(gl.TRIANGLES);
  }
}

init_demo(document.body);
