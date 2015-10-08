var Geometry = require('gl-geometry')
var fit      = require('canvas-fit')
var glShader = require('gl-shader')
var mat4     = require('gl-mat4')
var normals  = require('normals')
var bunny    = require('bunny')
var canvasOrbitCamera = require('canvas-orbit-camera')
var glContext = require('gl-context')

var vertex_shader =
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

var fragment_shader =
"precision mediump float;" +
"varying vec3 vNormal;" +
"void main() {" +
  "gl_FragColor = vec4(abs(vNormal), 1.0);" +
"}";


function init_demo(container) {
  // Create a <canvas> element to do our drawing in. Then set it up to fill
  // the container and resize when the window resizes.
  var canvas = container.appendChild(document.createElement('canvas'))
  window.addEventListener('resize', fit(canvas), false)

  // Attach a `canvas-orbit-camera` thing, which handles user input for
  // manipulating the view.
  var camera = canvasOrbitCamera(canvas)

  // Initialize the OpenGL context with our rendering function.
  var gl = glContext(canvas, function () {
    update()

    // Sets the viewport, i.e. tells WebGL to draw the
    // scene across the full canvas.
    gl.viewport(0, 0, width, height)

    // Enables depth testing, which prevents triangles
    // from overlapping.
    gl.enable(gl.DEPTH_TEST)

    // Enables face culling, which prevents triangles
    // being visible from behind.
    gl.enable(gl.CULL_FACE)

    // Binds the geometry and sets up the shader's attribute
    // locations accordingly.
    geometry.bind(shader)

    // Updates our model/view/projection matrices, sending them
    // to the GPU as uniform variables that we can use in
    // `shaders/bunny.vert` and `shaders/bunny.frag`.
    shader.uniforms.uProjection = projection
    shader.uniforms.uView = view
    shader.uniforms.uModel = model

    // Finally: draws the bunny to the screen! The rest is
    // handled in our shaders.
    geometry.draw(gl.TRIANGLES)
  });

  // Load the bunny mesh data (a simplicial complex)
  // into a gl-geometry instance, calculating vertex
  // normals for you. A simplicial complex is simply
  // a list of vertices and faces – conventionally called
  // `positions` and `cells` respectively. If you're familiar
  // with three.js, this is essentially equivalent to an array
  // of `THREE.Vector3` and `THREE.Face3` instances, except specified
  // as arrays for simplicity and interoperability.
  var geometry = Geometry(gl)

  geometry.attr('aPosition', bunny.positions)
  geometry.attr('aNormal', normals.vertexNormals(
      bunny.cells
    , bunny.positions
  ))

  geometry.faces(bunny.cells)

  // Create the base matrices to be used
  // when rendering the bunny. Alternatively, can
  // be created using `new Float32Array(16)`
  var projection = mat4.create()
  var model      = mat4.create()
  var view       = mat4.create()
  var height
  var width

  // Pulls up our shader code and returns an instance
  // of gl-shader. Using the glslify browserify transform,
  // these will be passed through glslify first to pull in
  // any external GLSL modules (of which there are none in
  // this example) and perform the uniform/attribute parsing
  // step ahead of time. We can make some dramatic file size
  // savings by doing this in Node rather then at runtime in
  // the browser.
  var shader = glShader(gl, vertex_shader , fragment_shader);

  // The logic/update loop, which updates all of the variables
  // before they're used in our render function. It's optional
  // for you to keep `update` and `render` as separate steps.
  function update() {
    // Updates the width/height we use to render the
    // final image.
    width  = gl.drawingBufferWidth
    height = gl.drawingBufferHeight

    // Updates our camera view matrix.
    camera.view(view)

    // Optionally, flush the state of the camera. Required
    // for user input to work correctly.
    camera.tick()

    // Update our projection matrix. This is the bit that's
    // responsible for taking 3D coordinates and projecting
    // them into 2D screen space.
    var aspectRatio = gl.drawingBufferWidth / gl.drawingBufferHeight
    var fieldOfView = Math.PI / 4
    var near = 0.01
    var far  = 100

    mat4.perspective(projection
      , fieldOfView
      , aspectRatio
      , near
      , far
    )
  }

}

init_demo(document.body);
