var EPSILON = 0.000001;

function Camera(){
    this.HALF_PI = Math.PI/2;
    this.VEC4_1 =  glMatrix.vec4.fromValues(1,0,0,0);
    this.VEC4_2 =  glMatrix.vec4.fromValues(0,1,0,0);
    this.VEC4_3 =  glMatrix.vec4.fromValues(0,0,1,0);
    this.VEC4_4 =  glMatrix.vec4.fromValues(0,0,0,1);

    this.VEC3_1 =  glMatrix.vec3.fromValues(1,0,0);
    this.VEC3_2 =  glMatrix.vec3.fromValues(0,1,0);
    this.VEC3_3 =  glMatrix.vec3.fromValues(0,0,1);

    this.tanFovHalf = glMatrix.vec2.fromValues(1,1);
    this.heading = 0.0;
    this.pitch = 0.0;
	this.bUpdate = true;
    this._moved_ = true;
    this.position =  this.VEC4_4;

    this.mRotation = glMatrix.mat4.create();
    this.eye = glMatrix.vec3.fromValues(0,0,0);
    this.right = null;
    this.up = null;
    this.head = null;
    
    this.updateRotationMatrix();
    this.Update();
}

Camera.prototype.updateRotationMatrix = function(){
    var mHeading = glMatrix.mat4.create();
    glMatrix.mat4.rotateY( mHeading, mHeading, this.heading );
    
    var mPitch = glMatrix.mat4.create();
    glMatrix.mat4.rotateX( mPitch, mPitch, this.pitch );

    glMatrix.mat4.multiply(this.mRotation,mHeading,mPitch);   
    
    this.bUpdate = true;
}

Camera.prototype.Update = function(){
    if ( this.bUpdate )	{

        this.eye[0] = this.position[0];
        this.eye[1] = this.position[1];
        this.eye[2] = this.position[2];

        var aixX = this.VEC4_1 ;
        var aixY = this.VEC4_2 ;
        var aixZ = this.VEC4_3 ;

        glMatrix.vec4.transformMat4(aixX,aixX,this.mRotation);
        glMatrix.vec4.transformMat4(aixY,aixY,this.mRotation);
        glMatrix.vec4.transformMat4(aixZ,aixZ,this.mRotation);

        this.right = glMatrix.vec3.fromValues(aixX[0],aixX[1],-aixX[2]);
        this.up = glMatrix.vec3.fromValues(aixY[0],aixY[1],-aixY[2]);
        this.ahead = glMatrix.vec3.fromValues(aixZ[0],aixZ[1],-aixZ[2]);

        this.bUpdate = false;
	}

}

Camera.prototype.turnLeft = function(rad){
    if (Math.abs(rad) > EPSILON) {
        this._moved_ = true;
    }
    this.heading += rad;
    this.updateRotationMatrix();
}

Camera.prototype.turnUp = function(rad){
    if (Math.abs(rad) > EPSILON) {
        this._moved_ = true;
    }
    this.pitch += rad;
    this.pitch = Math.max( this.pitch, -this.HALF_PI );
    this.pitch = Math.min( this.pitch, this.HALF_PI );
    this.updateRotationMatrix();
}

Camera.prototype.moveForward = function(d){
    if (Math.abs(d) > EPSILON) {
        this._moved_ = true;
    }

    var localD = glMatrix.vec4.fromValues(0,0,d,1);

    var identityM = glMatrix.mat4.create();
    glMatrix.mat4.rotateY( identityM, identityM, this.heading );

    // to global
    glMatrix.vec4.transformMat4(localD,localD,identityM);

    this.position[0] += localD[0];
    this.position[2] -= localD[2];

    this.bUpdate = true;
}

Camera.prototype.moveLeft = function(d){
    if (Math.abs(d) > EPSILON) {
        this._moved_ = true;
    }

    var localD = glMatrix.vec4.fromValues(-d,0,0,1);

    var identityM = glMatrix.mat4.create();
    glMatrix.mat4.rotateY( identityM, identityM, this.heading );

    // to global
    glMatrix.vec4.transformMat4(localD,localD,identityM);

    this.position[0] += localD[0];
    this.position[2] -= localD[2];
    
    this.bUpdate = true;
}

Camera.prototype.moveUp = function(d){
    if (Math.abs(d) > EPSILON) {
        this._moved_ = true;
    }
    this.position[1] += d;
    this.bUpdate = true;
}

Camera.prototype.bNeedUpdate = function(){
    return this.bNeedUpdate;
}

Camera.prototype.copyMCamera = function(out){
    for (var i = 0; i < 12; i++) {
        out[i] = this.mRotation[i];
    }
    for (var i = 0; i < 3; i++) {
        out[i + 12] = this.position[i];
    }

    out[15] = 1;
}

Camera.prototype.moved = function(){
    var temp = this._moved_;
    this._moved_ = false; 
    return temp;
}

Camera.prototype.setMoved = function(){
    this._moved_ = true;
}

var moving_speed = 0.1;

function setCameraSpeed(speed){
    moving_speed = speed; 
}

function walk_callback(canvas,camera){
    var left_pressed = false;
    var lastX,lastY;
    canvas.addEventListener("mousedown", doMouseDown, false);
    document.onkeydown = function (evt) {
        switch(evt.keyCode) {
            case 87: // W
                camera.moveForward( moving_speed );
                break;
            case 83: // S
                camera.moveForward( -moving_speed );
                break;
            case 65: // A
                camera.moveLeft( moving_speed );
                break;
            case 68: // D
                camera.moveLeft( -moving_speed );
                break;
            case 81: // Q
                camera.moveUp( -moving_speed );
                break;
            case 69: // E
                camera.moveUp( moving_speed );
                break;
            default: break;
        }
    }

    function doMouseDown(evt) {
        if (left_pressed)
           return;
        
        left_pressed = true;

        var box = canvas.getBoundingClientRect();
        lastX = evt.clientX - box.left;
        lastY = evt.clientY - box.top;
        
        document.addEventListener("mousemove", doMouseDrag, false);
        document.addEventListener("mouseup", doMouseUp, false);
    }

    function doMouseDrag(evt) {
        if (!left_pressed)
           return;

        var box = canvas.getBoundingClientRect();
        x = evt.clientX - box.left;
        y = evt.clientY - box.top;
        var rotX = ( lastX - x ) / 500.0;
        camera.turnLeft( rotX );
        var rotY = ( lastY - y ) / 500.0;
        camera.turnUp( rotY );

        lastX = x;
        lastY = y;
    }

    function doMouseUp(evt) {
        if (left_pressed) {
            document.removeEventListener("mousemove", doMouseDrag, false);
            document.removeEventListener("mouseup", doMouseUp, false);
            left_pressed = false;
        }
    }
}

function updateCamera(camera){
    var bUpdate = false;
	if (camera.bNeedUpdate())
	{
		bUpdate = true;
	}
	camera.Update();

	return bUpdate;
}