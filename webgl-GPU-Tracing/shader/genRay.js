const genRayShaderSource = `#version 310 es
  layout (local_size_x = 32, local_size_y = 4, local_size_z = 1) in;
  layout (rgba8, binding = 0) writeonly uniform highp image2D destTex;
  layout( std430, binding = 0 ) buffer SCREEN_BUFFER
  {
    vec4 colors[];
  };
  uniform uint frame_id;
  uniform mat4 mvMatrix;

  struct RTRay
  {
    vec4 pos;
    vec4 dir;
  };

  layout( std430, binding = 1 ) buffer RTRAY_BUFFER
  {
	RTRay rays[];
  };

  uint random_seed;
  uint SCRWIDTH = 800u;
  uint HALF_SCRWIDTH = 400u;

  const float very_large_float = 1e9f;

  float xorshift32()
  {
    random_seed ^= random_seed << 13;
    random_seed ^= random_seed >> 17;
    random_seed ^= random_seed << 5;
    return float(random_seed) * 2.3283064365387e-10f;
  }

  vec4 randomDirection()
  {
    float longitude = xorshift32() * 3.1415926535897932384626433832795f * 2.0f;
    float lattitude = xorshift32() * 3.1415926535897932384626433832795f;
    float z = sin( lattitude );
    float s = cos( lattitude );
    return vec4( s * cos( longitude ), s * sin( longitude ), z, z );
  }

  void primaryRay( in uint x, in uint y, out RTRay ray )
  {
    ray.pos = vec4( mvMatrix * vec4( 0, 0, 0, 1 ) );
    ray.dir = normalize( vec4( mvMatrix * vec4( vec3( float( x ) + xorshift32(), float( SCRWIDTH - y ) + xorshift32(), 0 ) - vec3( HALF_SCRWIDTH ), 1 ) ) - ray.pos );
  }

  void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

    uint largePrime1 = 386030683u;
	uint largePrime2 = 919888919u;
    uint largePrime3 = 101414101u;
    
    random_seed = ( ( uint(storePos.x) * largePrime1 + uint(storePos.y) ) * largePrime1 + frame_id * largePrime3 );
    int idx = storePos.y * int( SCRWIDTH ) + storePos.x;
    primaryRay( gl_GlobalInvocationID.x, gl_GlobalInvocationID.y,rays[idx]);
  }
  `;