var Geometry = require('gl-geometry')
var fit      = require('canvas-fit')
var glShader = require('gl-shader')
var mat4     = require('gl-mat4')
var normals  = require('normals')
var glslify  = require('glslify')
var bunny    = require('bunny')

// Creates a canvas element and attaches
// it to the <body> on your DOM.
var canvas = document.body.appendChild(document.createElement('canvas'))

// Creates an instance of canvas-orbit-camera,
// which later will generate a view matrix and
// handle interaction for you.
var camera = require('canvas-orbit-camera')(canvas)

// A small convenience function for creating
// a new WebGL context – the `render` function
// supplied here is called every frame to draw
// to the screen.
var gl = require('gl-context')(canvas, render)

// Resizes the <canvas> to fully fit the window
// whenever the window is resized.
window.addEventListener('resize'
  , fit(canvas)
  , false
)

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
var shader = glShader(gl,
    glslify('./shaders/bunny.vert')
  , glslify('./shaders/bunny.frag')
)

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

function render() {
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
}
