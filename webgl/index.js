"use strict";

var fit = require('canvas-fit');
var mat4 = require('gl-mat4');
var normals = require('normals');
var bunny = require('bunny');
var canvasOrbitCamera = require('canvas-orbit-camera');
var glContext = require('gl-context');
var pack = require('array-pack-2d');

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

// Compile a single shader program, given the type (gl.VERTEX_SHADER,
// gl.FRAGMENT_SHADER, etc.) and the source code.
function compile(gl, type, src) {
  var shader = gl.createShader(type)
  gl.shaderSource(shader, src)
  gl.compileShader(shader)
  if(!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    var errLog = gl.getShaderInfoLog(shader)
    console.error('Error compiling shader:', errLog)
  }
  return shader
}

// Compile and link an entire shader program consisting of a vertex shader
// and a fragment shader.
function get_shader(gl, vertex_source, fragment_source) {
  // Compile.
  // Bizarrely, compiling these in the opposite order leads to very strange
  // effects (but not errors!).
  var vert = compile(gl, gl.VERTEX_SHADER, vertex_source);
  var frag = compile(gl, gl.FRAGMENT_SHADER, fragment_source);

  // Link.
  var program = gl.createProgram()
  gl.attachShader(program, vert)
  gl.attachShader(program, frag)
  gl.linkProgram(program)

  // Check for errors.
  if(!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    var errLog = gl.getProgramInfoLog(program)
    console.error('Error linking program:', errLog)
  }

  return program
}

// Compute a project matrix (placed in the `out` matrix allocation) given the
// width and height of a viewport.
function projection_matrix(out, width, height) {
  var aspectRatio = width / height;
  var fieldOfView = Math.PI / 4;
  var near = 0.01;
  var far  = 100;

  mat4.perspective(out, fieldOfView, aspectRatio, near, far)
}

function make_buffer(gl, data, type, mode) {
  // Initialize a buffer.
  var buf = gl.createBuffer();

  // Flatten the data to a packed array.
  var arr = pack(data, type);

  // Insert the data into the buffer.
  gl.bindBuffer(mode, buf);
  gl.bufferData(mode, arr, gl.STATIC_DRAW);

  return buf;
}

// Given a mesh, with the fields `positions` and `cells`, create three buffers
// for drawing the thing. Return an object with the fields:
// - `cells`, a 3-dimensional uint16 element array buffer
// - `positions`, a 3-dimensional float32 array buffer
// - `normals`, ditto
function mesh_buffers(gl, obj) {
  var norm = normals.vertexNormals(bunny.cells, bunny.positions);

  return {
    cells: make_buffer(gl, obj.cells, 'uint16', gl.ELEMENT_ARRAY_BUFFER),
    positions: make_buffer(gl, obj.positions, 'float32', gl.ARRAY_BUFFER),
    normals: make_buffer(gl, norm, 'float32', gl.ARRAY_BUFFER),
  }
}

// Set a buffer as an attribute array.
function bind_attrib_buffer(gl, location, buffer) {
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.vertexAttribPointer(location, 3, gl.FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(location);
}

// Set a buffer as the element array.
function bind_element_buffer(gl, buffer) {
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
}

// Our shader demo. Set up the demo in a container element (a <canvas> element
// is added therein).
function demo(container) {
  // Create a <canvas> element to do our drawing in. Then set it up to fill
  // the container and resize when the window resizes.
  var canvas = container.appendChild(document.createElement('canvas'));
  window.addEventListener('resize', fit(canvas), false);

  // Attach a `canvas-orbit-camera` thing, which handles user input for
  // manipulating the view.
  var camera = canvasOrbitCamera(canvas);

  // Initialize the OpenGL context with our rendering function.
  var gl = glContext(canvas, render);

  // Compile the shader program.
  var my_program = get_shader(gl, VERTEX_SHADER, FRAGMENT_SHADER);
  var locations = {
    'uProjection': gl.getUniformLocation(my_program, 'uProjection'),
    'uView': gl.getUniformLocation(my_program, 'uView'),
    'uModel': gl.getUniformLocation(my_program, 'uModel'),
    'aPosition': gl.getAttribLocation(my_program, 'aPosition'),
    'aNormal': gl.getAttribLocation(my_program, 'aNormal'),
  };

  // Load the shape data into buffers.
  var bunny_buffers = mesh_buffers(gl, bunny);

  // Create the base matrices to be used
  // when rendering the bunny. Alternatively, can
  // be created using `new Float32Array(16)`
  var projection = mat4.create();
  var model      = mat4.create();
  var view       = mat4.create();

  // The main rendering loop.
  function render() {
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
    gl.useProgram(my_program);

    // Set the shader "uniform" parameters.
    gl.uniformMatrix4fv(locations['uProjection'], false, projection);
    gl.uniformMatrix4fv(locations['uView'], false, view);
    gl.uniformMatrix4fv(locations['uModel'], false, model);

    // Bind our VAO to communicate the vertex (varying) data to the shader.
    // vao.bind();

    // Set the attribute arrays.
    bind_attrib_buffer(gl, locations.aPosition, bunny_buffers.positions);
    bind_attrib_buffer(gl, locations.aNormal, bunny_buffers.normals);

    // And the element array.
    // TODO What is an element array?
    bind_element_buffer(gl, bunny_buffers.cells);

    // Draw it!
    var count = bunny.cells.length * bunny.cells[0].length;
    gl.drawElements(gl.TRIANGLES, count, gl.UNSIGNED_SHORT, 0);
  }
}

demo(document.body);
