const CANVAS_WIDTH = 800;
const CANVAS_HEIGHT = 800;

var frame_id = 1;

var requestFiles = ["triangles.bin","bvh.bin","material.bin","texture.bin","texIn.bin","lights.bin","lights_num.bin"];
var scene_buffer = [];

function loadSceneData(url, idx)
{
  const promise = new Promise(function(resolve, reject) {
    var oReq = new XMLHttpRequest();
    oReq.open("GET", url, true);
    oReq.responseType = "arraybuffer";

    oReq.onload = function (oEvent) {
      var arrayBuffer = oReq.response;
      if (arrayBuffer) {
        scene_buffer[idx] = new Float32Array(arrayBuffer);
        resolve(1);
      }
  };
  oReq.send(null);          
  });

  return promise;
}


function buffer2GPU(gl)
{

  const screenBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, screenBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 16*CANVAS_WIDTH * CANVAS_HEIGHT, gl.STATIC_READ); // sizeof( vec4 ) * CANVAS_WIDTH * CANVAS_HEIGHT

  const rayBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, rayBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 32 * CANVAS_WIDTH * CANVAS_HEIGHT, gl.STATIC_READ); // sizeof( RTRay ) * SCRWIDTH * SCRHEIGHT

  const triangleBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, triangleBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[0], gl.STATIC_DRAW);

  const bvhBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, bvhBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[1], gl.STATIC_DRAW);

  const materialsBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, materialsBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[2], gl.STATIC_DRAW);

  const texturesBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, texturesBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[3], gl.STATIC_DRAW);

  const textureInfosBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, textureInfosBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[4], gl.STATIC_DRAW);

  const lightsBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, lightsBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[5], gl.STATIC_DRAW);

  const lightsNumBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, lightsNumBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, scene_buffer[6], gl.STATIC_DRAW);

  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, screenBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, rayBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, triangleBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, bvhBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 4, materialsBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 5, texturesBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 6, textureInfosBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 7, lightsBuffer_ID);
}

function floatEqual(v1,v2)
{
  if(Math.abs(v1-v2)< Number.EPSILON)
  {
    return true;
  }

  return false;

}
function equal (buf1, buf2)
{
    for (var i = 0 ; i <16 ; i++)
    {
        if (!floatEqual(buf1[i],buf2[i])) return false;
    }
    return true;
}

function main() {  
  var canvas = document.getElementById('webgl');

  var context = canvas.getContext('webgl2-compute', {antialias: false});
  if (!context) {
    console.log('Failed to get the rendering context for WebGL');
    return;
  }

  // ComputeShader source from shader folder  

  // load scene
  ///////////////////////////////////////////////////////
  var promise_list = [];

  for (let index = 0; index < requestFiles.length; index++) {
    var url = "./data/" + requestFiles[index];
    promise_list.push(loadSceneData(url,index));    
  }

  Promise.all(promise_list).then(function(){
    // create WebGLShader for ComputeShader
    const genRayShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(genRayShader, genRayShaderSource);
    context.compileShader(genRayShader);
    if (!context.getShaderParameter(genRayShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(genRayShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    const genRay_SID = context.createProgram();
    context.attachShader(genRay_SID, genRayShader);
    context.linkProgram(genRay_SID);
    if (!context.getProgramParameter(genRay_SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(genRay_SID));
      return;
    }

    const tracingRayShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(tracingRayShader, tracingRayShaderSource);
    context.compileShader(tracingRayShader);
    if (!context.getShaderParameter(tracingRayShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(tracingRayShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    const tracingRay_SID = context.createProgram();
    context.attachShader(tracingRay_SID, tracingRayShader);
    context.linkProgram(tracingRay_SID);
    if (!context.getProgramParameter(tracingRay_SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(tracingRay_SID));
      return;
    }
    ///////////////////////////////////////////////////////
    buffer2GPU(context);
    /////////////////////////////////////////////////////////

    // create texture for ComputeShader write to
    const texture = context.createTexture();
    context.bindTexture(context.TEXTURE_2D, texture);
    context.texStorage2D(context.TEXTURE_2D, 1, context.RGBA8, CANVAS_WIDTH, CANVAS_HEIGHT);
    context.bindImageTexture(0, texture, 0, false, 0, context.WRITE_ONLY, context.RGBA8);

    // create frameBuffer to read from texture
    const frameBuffer = context.createFramebuffer();
    context.bindFramebuffer(context.READ_FRAMEBUFFER, frameBuffer);
    context.framebufferTexture2D(context.READ_FRAMEBUFFER, context.COLOR_ATTACHMENT0, context.TEXTURE_2D, texture, 0);

    var camera = new Camera();
    walk_callback(canvas,camera);
    setCameraSpeed(1);

    var mvMatrix = glMatrix.mat4.create();
    var lastMatrix = glMatrix.mat4.create();

    var tick = function() {

      updateCamera(camera);

      camera.copyMCamera(mvMatrix);
      //glMatrix.mat4.transpose(mvMatrix,mvMatrix)

      if(!equal(lastMatrix,mvMatrix))
      {
        glMatrix.mat4.copy(lastMatrix,mvMatrix)
			  frame_id = 0;
      }

      context.useProgram(genRay_SID);
      context.uniform1ui( context.getUniformLocation( genRay_SID, "frame_id" ), frame_id );

      context.uniformMatrix4fv(context.getUniformLocation(genRay_SID, 'mvMatrix'),false,mvMatrix);
      context.dispatchCompute(CANVAS_WIDTH / 32, CANVAS_HEIGHT / 4, 1);

      context.useProgram(tracingRay_SID);
      context.uniform1ui( context.getUniformLocation( tracingRay_SID, "frame_id" ), frame_id++ );

      context.uniformMatrix4fv(context.getUniformLocation(tracingRay_SID, 'mvMatrix'),false,mvMatrix);
      context.dispatchCompute(CANVAS_WIDTH / 32, CANVAS_HEIGHT / 4, 1);

      context.memoryBarrier(context.SHADER_IMAGE_ACCESS_BARRIER_BIT);

      // show computed texture to Canvas
      context.blitFramebuffer(
        0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
        0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
        context.COLOR_BUFFER_BIT, context.NEAREST);

      window.requestAnimationFrame(tick, canvas);
    }

    tick(); 
    });
}