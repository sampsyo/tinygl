var fit      = require('canvas-fit')
var mat4     = require('gl-mat4')
var normals  = require('normals')
var bunny    = require('bunny')
var canvasOrbitCamera = require('canvas-orbit-camera')
var glContext = require('gl-context')
var pack = require('array-pack-2d')

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

function my_get_shader(gl) {
  // Compile.
  // Bizarrely, compiling these in the opposite order leads to very strange
  // effects.
  var vert = compile(gl, gl.VERTEX_SHADER, VERTEX_SHADER);
  var frag = compile(gl, gl.FRAGMENT_SHADER, FRAGMENT_SHADER);

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

function init_demo(container) {
  // Create a <canvas> element to do our drawing in. Then set it up to fill
  // the container and resize when the window resizes.
  var canvas = container.appendChild(document.createElement('canvas'));
  window.addEventListener('resize', fit(canvas), false);

  // Attach a `canvas-orbit-camera` thing, which handles user input for
  // manipulating the view.
  var camera = canvasOrbitCamera(canvas);

  // Initialize the OpenGL context with our rendering function.
  var gl = glContext(canvas, render);

  // TODO NEW!
  var my_program = my_get_shader(gl);
  var locations = {
    'uProjection': gl.getUniformLocation(my_program, 'uProjection'),
    'uView': gl.getUniformLocation(my_program, 'uView'),
    'uModel': gl.getUniformLocation(my_program, 'uModel'),
    'aPosition': gl.getAttribLocation(my_program, 'aPosition'),
    'aNormal': gl.getAttribLocation(my_program, 'aNormal'),
  };

  // TODO new!
  var cells_buffer = make_buffer(gl, bunny.cells, 'uint16', gl.ELEMENT_ARRAY_BUFFER);

  // TODO NEW!
  var position = bunny.positions;
  var normal = normals.vertexNormals(bunny.cells, bunny.positions);
  var position_buffer = make_buffer(gl, position, 'float32', gl.ARRAY_BUFFER);
  var normal_buffer = make_buffer(gl, normal, 'float32', gl.ARRAY_BUFFER);

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

    // TODO
    gl.bindBuffer(gl.ARRAY_BUFFER, position_buffer);
    gl.vertexAttribPointer(locations['aPosition'], 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(locations['aPosition']);

    // TODO
    gl.bindBuffer(gl.ARRAY_BUFFER, normal_buffer);
    gl.vertexAttribPointer(locations['aNormal'], 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(locations['aNormal']);

    // TODO
    // also, what is an *element* array? TODO
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, cells_buffer);

    // Draw it!
    var count = bunny.cells.length * bunny.cells[0].length;
    gl.drawElements(gl.TRIANGLES, count, gl.UNSIGNED_SHORT, 0);
  }
}

init_demo(document.body);
