const CANVAS_WIDTH = 800;
const CANVAS_HEIGHT = 800;
const LocalSize_X = 128;

var frame_id = 0;
var total_num = 0;
var qc = new Uint32Array(4);

var bWaveFront = 1;

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

var RenderParameters = {
  nTaskNum: 1<<20,
  nWidth: 800,
  nHeight: 800,
  nMaxBounces: 5,
  color_scene:glMatrix.vec4.fromValues(1,0,0,1)
};

var queueCounter_ID;

function clearQueueCounter(gl)
{
  qc[0] = qc[1] = qc[2] = qc[3] = 0
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, queueCounter_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, qc, gl.DYNAMIC_COPY);  
}

function buffer2GPU(gl)
{

  const screenBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, screenBuffer_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 16*CANVAS_WIDTH * CANVAS_HEIGHT, gl.STATIC_READ); // sizeof( vec4 ) * CANVAS_WIDTH * CANVAS_HEIGHT

  // wavefront sizeof(RTRay) = 204
  const rayBuffer_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, rayBuffer_ID);
  //gl.bufferData(gl.SHADER_STORAGE_BUFFER, 32 * CANVAS_WIDTH * CANVAS_HEIGHT, gl.STATIC_READ); // sizeof( RTRay ) * SCRWIDTH * SCRHEIGHT
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 204 * RenderParameters.nTaskNum, gl.STATIC_READ); // 

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

  //wf manager
  var renderparameter_buffer = new Uint32Array(8);
  renderparameter_buffer[0] = RenderParameters.nTaskNum;
  renderparameter_buffer[1] = RenderParameters.nWidth;
  renderparameter_buffer[2] = RenderParameters.nHeight;
  renderparameter_buffer[3] = RenderParameters.nMaxBounces;

  const renderParameters_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, renderParameters_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, renderparameter_buffer, gl.STATIC_DRAW);

  queueCounter_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, queueCounter_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 16, gl.DYNAMIC_COPY);

  const genQueue_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, genQueue_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 4*RenderParameters.nTaskNum, gl.STATIC_DRAW);

  const materialsQueue_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, materialsQueue_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 4*RenderParameters.nTaskNum, gl.STATIC_DRAW);

  const extensionQueue_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, extensionQueue_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 4*RenderParameters.nTaskNum, gl.STATIC_DRAW);

  const shadowQueue_ID = gl.createBuffer();
  gl.bindBuffer(gl.SHADER_STORAGE_BUFFER, shadowQueue_ID);
  gl.bufferData(gl.SHADER_STORAGE_BUFFER, 4*RenderParameters.nTaskNum, gl.STATIC_DRAW);

  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, screenBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, rayBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, triangleBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, bvhBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 4, materialsBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 5, texturesBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 6, textureInfosBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 7, lightsBuffer_ID);
  gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 8, lightsNumBuffer_ID);

  gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 9, renderParameters_ID );
	gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 10, queueCounter_ID );
	gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 11, genQueue_ID );
	gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 12, materialsQueue_ID );
	gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 13, extensionQueue_ID );
  gl.bindBufferBase( gl.SHADER_STORAGE_BUFFER, 14, shadowQueue_ID );
  
  return queueCounter_ID;
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

var wf_reset_SID,wf_rayGen__SID,wf_logic__SID,wf_extensionShader__SID,wf_material__SID,wf_shadow__SID;
function loadWavefrontshader(context)
{
    const wf_resetShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_resetShader, wf_reset);
    context.compileShader(wf_resetShader);
    if (!context.getShaderParameter(wf_resetShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_resetShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_reset_SID = context.createProgram();
    context.attachShader(wf_reset_SID, wf_resetShader);
    context.linkProgram(wf_reset_SID);
    if (!context.getProgramParameter(wf_reset_SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_reset_SID));
      return;
    }

    const wf_rayGenShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_rayGenShader, wf_rayGen);
    context.compileShader(wf_rayGenShader);
    if (!context.getShaderParameter(wf_rayGenShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_rayGenShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_rayGen__SID = context.createProgram();
    context.attachShader(wf_rayGen__SID, wf_rayGenShader);
    context.linkProgram(wf_rayGen__SID);
    if (!context.getProgramParameter(wf_rayGen__SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_rayGen__SID));
      return;
    }

    const wf_logicShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_logicShader, wf_logic);
    context.compileShader(wf_logicShader);
    if (!context.getShaderParameter(wf_logicShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_logicShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_logic__SID = context.createProgram();
    context.attachShader(wf_logic__SID, wf_logicShader);
    context.linkProgram(wf_logic__SID);
    if (!context.getProgramParameter(wf_logic__SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_logic__SID));
      return;
    }

    const wf_extensionShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_extensionShader, wf_extension);
    context.compileShader(wf_extensionShader);
    if (!context.getShaderParameter(wf_extensionShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_extensionShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_extensionShader__SID = context.createProgram();
    context.attachShader(wf_extensionShader__SID, wf_extensionShader);
    context.linkProgram(wf_extensionShader__SID);
    if (!context.getProgramParameter(wf_extensionShader__SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_extensionShader__SID));
      return;
    }

    const wf_materialShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_materialShader, wf_material);
    context.compileShader(wf_materialShader);
    if (!context.getShaderParameter(wf_materialShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_materialShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_material__SID = context.createProgram();
    context.attachShader(wf_material__SID, wf_materialShader);
    context.linkProgram(wf_material__SID);
    if (!context.getProgramParameter(wf_material__SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_material__SID));
      return;
    }

    const wf_shadowShader = context.createShader(context.COMPUTE_SHADER);
    context.shaderSource(wf_shadowShader, wf_shadow);
    context.compileShader(wf_shadowShader);
    if (!context.getShaderParameter(wf_shadowShader, context.COMPILE_STATUS)) {
      console.log(context.getShaderInfoLog(wf_shadowShader));
      return;
    }

    // create WebGLProgram for ComputeShader
    wf_shadow__SID = context.createProgram();
    context.attachShader(wf_shadow__SID, wf_shadowShader);
    context.linkProgram(wf_shadow__SID);
    if (!context.getProgramParameter(wf_shadow__SID, context.LINK_STATUS)) {
      console.log(context.getProgramInfoLog(wf_shadow__SID));
      return;
    }
}

var genRay_SID,tracingRay_SID;
function loadmegaKernelshader(context)
{
  // create WebGLShader for ComputeShader
  const genRayShader = context.createShader(context.COMPUTE_SHADER);
  context.shaderSource(genRayShader, genRayShaderSource);
  context.compileShader(genRayShader);
  if (!context.getShaderParameter(genRayShader, context.COMPILE_STATUS)) {
    console.log(context.getShaderInfoLog(genRayShader));
    return;
  }

  // create WebGLProgram for ComputeShader
  genRay_SID = context.createProgram();
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
  tracingRay_SID = context.createProgram();
  context.attachShader(tracingRay_SID, tracingRayShader);
  context.linkProgram(tracingRay_SID);
  if (!context.getProgramParameter(tracingRay_SID, context.LINK_STATUS)) {
    console.log(context.getProgramInfoLog(tracingRay_SID));
    return;
  }
}

function main() {  
  var canvas = document.getElementById('webgl');
  var stats = new Stats();

  var container = document.createElement( 'div' );
  document.body.appendChild( container );
  container.appendChild( stats.dom );

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

    if(bWaveFront){
      loadWavefrontshader(context);
    }
    else{
      loadmegaKernelshader(context);
    }
    
    buffer2GPU(context);

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

      if(bWaveFront){
        if(frame_id == 0)
      {
        total_num = 0;

        context.useProgram(wf_reset_SID);
        context.uniform1ui( context.getUniformLocation( wf_reset_SID, "frame_id" ), frame_id );
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.useProgram(wf_rayGen__SID);
        context.uniform1ui( context.getUniformLocation( wf_rayGen__SID, "total_num" ), total_num );
        context.uniform1ui( context.getUniformLocation( wf_rayGen__SID, "frame_id" ), frame_id++ );
        context.uniformMatrix4fv(context.getUniformLocation(wf_rayGen__SID, 'mvMatrix'),false,mvMatrix);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.bindBuffer(context.SHADER_STORAGE_BUFFER, queueCounter_ID);
        context.getBufferSubData(context.SHADER_STORAGE_BUFFER, 0, qc);

        total_num += qc[0];
        total_num = total_num % ( CANVAS_WIDTH * CANVAS_HEIGHT );
        
        context.useProgram(wf_extensionShader__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.useProgram(wf_material__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.useProgram(wf_shadow__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        clearQueueCounter(context);

        context.bindBuffer(context.SHADER_STORAGE_BUFFER, queueCounter_ID);
        context.getBufferSubData(context.SHADER_STORAGE_BUFFER, 0, qc);
      }
      else{
        context.useProgram(wf_logic__SID);
        context.uniform1ui( context.getUniformLocation( wf_logic__SID, "frame_id" ), frame_id );
        context.uniformMatrix4fv(context.getUniformLocation(wf_logic__SID, 'mvMatrix'),false,mvMatrix);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);   
        
        context.useProgram(wf_rayGen__SID);
        context.uniform1ui( context.getUniformLocation( wf_rayGen__SID, "total_num" ), total_num );
        context.uniform1ui( context.getUniformLocation( wf_rayGen__SID, "frame_id" ), frame_id++ );
        context.uniformMatrix4fv(context.getUniformLocation(wf_rayGen__SID, 'mvMatrix'),false,mvMatrix);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.bindBuffer(context.SHADER_STORAGE_BUFFER, queueCounter_ID);
        context.getBufferSubData(context.SHADER_STORAGE_BUFFER, 0, qc);

        total_num += qc[0];
        total_num = total_num % ( CANVAS_WIDTH * CANVAS_HEIGHT );
        
        context.useProgram(wf_extensionShader__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.useProgram(wf_material__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        context.useProgram(wf_shadow__SID);
        context.dispatchCompute(RenderParameters.nTaskNum / LocalSize_X, 1, 1);

        clearQueueCounter(context);

        context.bindBuffer(context.SHADER_STORAGE_BUFFER, queueCounter_ID);
        context.getBufferSubData(context.SHADER_STORAGE_BUFFER, 0, qc);
      }
      }
      else{
        context.useProgram(genRay_SID);
        context.uniform1ui( context.getUniformLocation( genRay_SID, "frame_id" ), frame_id );

        context.uniformMatrix4fv(context.getUniformLocation(genRay_SID, 'mvMatrix'),false,mvMatrix);
        context.dispatchCompute(CANVAS_WIDTH / 32, CANVAS_HEIGHT / 4, 1);

        context.useProgram(tracingRay_SID);
        context.uniform1ui( context.getUniformLocation( tracingRay_SID, "frame_id" ), frame_id++ );

        context.uniformMatrix4fv(context.getUniformLocation(tracingRay_SID, 'mvMatrix'),false,mvMatrix);
        context.dispatchCompute(CANVAS_WIDTH / 32, CANVAS_HEIGHT / 4, 1);
      }

      context.memoryBarrier(context.SHADER_IMAGE_ACCESS_BARRIER_BIT);

      // show computed texture to Canvas
      context.blitFramebuffer(
        0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
        0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
        context.COLOR_BUFFER_BIT, context.NEAREST);

      stats.update();

      window.requestAnimationFrame(tick, canvas);
    }

    tick(); 
    });
}