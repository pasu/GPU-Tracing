#define SCRWIDTH			800
#define SCRHEIGHT			800

//#define  WAVEFRONT

#ifdef WAVEFRONT
    #define LocalSize_X 128
    #define LocalSize_Y 1
#else
    #define LocalSize_X 32
    #define LocalSize_Y 4
#endif


#define NUM_CELLS	16.0	    // Needs to be a multiple of TILES!
#define TILES 		2.0		// Normally set to 1.0 for a creating a tileable texture.

#define SAMPLE_NUM 16
#define SAMPLE_NUM2 SAMPLE_NUM *SAMPLE_NUM

#define SAH_ON
#define BIN_NUM 128