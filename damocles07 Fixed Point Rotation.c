#define WIN32_LEAN_AND_MEAN
#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windef.h>

#define IDM_EXIT           	106

#define INITIAL_DIB_WIDTH  	160	// initial dimensions of DIB
#define INITIAL_DIB_HEIGHT	128	//  into which we'll draw
#define MAX_POLY_VERTS      256
#define MAX_OBJECTS 		256
#define MAX_OBJECT_POINTS   256
#define MAX_SCREEN_HEIGHT   INITIAL_DIB_HEIGHT*2
#define PI                  3.141592f
#define NUM_FRUSTUM_PLANES  5
#define EPSILON  			0.0001f
#define FIELD_OF_VIEW 		2.0f //3.0 //4.0 //2.0

#define LEFT 0
#define RIGHT 1

#define ALL_ON 0
#define ALL_OFF 1
#define INTERSECTING 2

#define CLIP_FRONT 1
#define CLIP_LEFT 2
#define CLIP_RIGHT 4
#define CLIP_BOTTOM 8
#define CLIP_TOP 16

#define SKY 0 // 18 //10 // BLUE
#define GND 1 // 14 //10 // GREEN
#define ROD 2 // 12 //20 // BLACK //20
#define BLD 3 // 01 //WHITE

// {0,0,255},{0,255,0},{255,0,0},{255,255,255} ;

#define HR_AG 0.7071067812f
#define HR_SZ 8192.0f

#define DEBUG

#define FILL_TYPE 0

//#define TURN_SPEED PI / 24576.0f //32767.0f //16384.0f //1024.0f //4096 //512.0 //2048.0 //1024.0
//#define TILT_LIMIT TURN_SPEED*64.0f
//#define TILT_FRICTION TURN_SPEED*0.66f

#define TURN_SPEED PI / 4096.0f //1024.0f //4096 //512.0 //2048.0 //1024.0
#define TILT_LIMIT TURN_SPEED*16.0f
#define TILT_FRICTION TURN_SPEED*0.4f
#define ACCEL 0.097f
#define FRICTION 0.97f //0.95f

#define MS 4096
#define MO MS/2
#define RD 8

#define COLOURS_USED 4

typedef struct 
{
  float x;
  float y;
  float z;
} point_t;

typedef struct 
{
  signed short int x, y;
} point2D_t;

typedef struct 
{
  float distance;
  point_t normal;
} plane_t;

typedef struct 
{
  BYTE ClipCode;
  point_t Point;
} Point3D;

typedef struct 
{
  BYTE Points; 		// number of points
  // BYTE hull_points; // number of points making up convex hull for inscreen check - not implemented
  point_t *Point; // pointer to list of points
  BYTE Faces; 		// number of faces
  BYTE *Face; 		// pointer to list of faces (face point count, face colour, face point index, face point count, face colour, face point index, etc)
  float BoundingSphere; // object bounding sphere
  float VanishingPoint; // point where object isnt drawn
} OBJECT;

typedef struct 
{
  point_t Location;
  BYTE Type;    // object type
} OBJECTS;

struct pBITMAPINFO
{
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[256];
} BMInfo;

struct pLOGPALETTE
{
  WORD palVersion;
  WORD palNumEntries;
  PALETTEENTRY palPalEntry[256];
} PalInfo;

char *pDIB, *pDIBBase;		// pointers to DIB section we'll draw into
HBITMAP hDIBSection;        // handle of DIB section
HWND hwndOutput;
int DIBWidth=INITIAL_DIB_WIDTH, DIBHeight=INITIAL_DIB_HEIGHT, DIBPitch;

float currentspeed=0;
point_t currentpos={0,0,0};
point_t currentdir={0,0,0};
point_t currentturnspeed={0,0,0};

signed short int xcenter, ycenter; 
float xscreenscale, yscreenscale, maxscale;

plane_t frustumplanes[NUM_FRUSTUM_PLANES];
point_t vpn, vright, vup, ovpn, ovright, ovup;
Point3D Vertex[MAX_POLY_VERTS];

OBJECTS groundobjects[] = {
{ {0,0,0},  1},

{ { MS*(0-8), 0, MS*(0-8)+MO},  2},
{ { MS*(0-8), 0, MS*(1-8)+MO},  2},
{ { MS*(0-8), 0, MS*(2-8)+MO},  2},
{ { MS*(0-8), 0, MS*(3-8)+MO},  2},
{ { MS*(0-8), 0, MS*(4-8)+MO},  2},
{ { MS*(0-8), 0, MS*(5-8)+MO},  2},
{ { MS*(0-8), 0, MS*(6-8)+MO},  2},
{ { MS*(0-8), 0, MS*(7-8)+MO},  2},
{ { MS*(0-8), 0, MS*(8-8)+MO},  2},
{ { MS*(0-8), 0, MS*(9-8)+MO},  2},
{ { MS*(0-8), 0, MS*(10-8)+MO},  2},
{ { MS*(0-8), 0, MS*(11-8)+MO},  2},
{ { MS*(0-8), 0, MS*(12-8)+MO},  2},
{ { MS*(0-8), 0, MS*(13-8)+MO},  2},
{ { MS*(0-8), 0, MS*(14-8)+MO},  2},

{ { MS*(1-8), 0, MS*(0-8)+MO},  2},
{ { MS*(1-8), 0, MS*(1-8)+MO},  2},
{ { MS*(1-8), 0, MS*(2-8)+MO},  2},
{ { MS*(1-8), 0, MS*(3-8)+MO},  2},
{ { MS*(1-8), 0, MS*(4-8)+MO},  2},
{ { MS*(1-8), 0, MS*(5-8)+MO},  2},
{ { MS*(1-8), 0, MS*(6-8)+MO},  2},
{ { MS*(1-8), 0, MS*(7-8)+MO},  2},
{ { MS*(1-8), 0, MS*(8-8)+MO},  2},

{ { MS*(2-8), 0, MS*(4-8)+MO},  2},
{ { MS*(2-8), 0, MS*(5-8)+MO},  2},
{ { MS*(2-8), 0, MS*(9-8)+MO},  2},
{ { MS*(2-8), 0, MS*(10-8)+MO},  2},
{ { MS*(2-8), 0, MS*(11-8)+MO},  2},
{ { MS*(2-8), 0, MS*(12-8)+MO},  2},

{ { MS*(3-8), 0, MS*(3-8)+MO},  2},
{ { MS*(3-8), 0, MS*(4-8)+MO},  2},
{ { MS*(3-8), 0, MS*(5-8)+MO},  2},
{ { MS*(3-8), 0, MS*(6-8)+MO},  2},
{ { MS*(3-8), 0, MS*(7-8)+MO},  2},
{ { MS*(3-8), 0, MS*(8-8)+MO},  2},
{ { MS*(3-8), 0, MS*(9-8)+MO},  2},
{ { MS*(3-8), 0, MS*(10-8)+MO},  2},
{ { MS*(3-8), 0, MS*(11-8)+MO},  2},

{ { MS*(4-8), 0, MS*(4-8)+MO},  2},
{ { MS*(4-8), 0, MS*(5-8)+MO},  2},
{ { MS*(4-8), 0, MS*(6-8)+MO},  2},
{ { MS*(4-8), 0, MS*(7-8)+MO},  2},
{ { MS*(4-8), 0, MS*(8-8)+MO},  2},
{ { MS*(4-8), 0, MS*(9-8)+MO},  2},
{ { MS*(4-8), 0, MS*(10-8)+MO},  2},
{ { MS*(4-8), 0, MS*(11-8)+MO},  2},
{ { MS*(4-8), 0, MS*(12-8)+MO},  2},
{ { MS*(4-8), 0, MS*(13-8)+MO},  2},
{ { MS*(4-8), 0, MS*(14-8)+MO},  2},

{ { MS*(5-8), 0, MS*(0-8)+MO},  2},
{ { MS*(5-8), 0, MS*(1-8)+MO},  2},
{ { MS*(5-8), 0, MS*(2-8)+MO},  2},
{ { MS*(5-8), 0, MS*(3-8)+MO},  2},
{ { MS*(5-8), 0, MS*(4-8)+MO},  2},
{ { MS*(5-8), 0, MS*(5-8)+MO},  2},
{ { MS*(5-8), 0, MS*(6-8)+MO},  2},
{ { MS*(5-8), 0, MS*(7-8)+MO},  2},
{ { MS*(5-8), 0, MS*(8-8)+MO},  2},
{ { MS*(5-8), 0, MS*(9-8)+MO},  2},
{ { MS*(5-8), 0, MS*(10-8)+MO},  2},
{ { MS*(5-8), 0, MS*(11-8)+MO},  2},
{ { MS*(5-8), 0, MS*(12-8)+MO},  2},
{ { MS*(5-8), 0, MS*(13-8)+MO},  2},
{ { MS*(5-8), 0, MS*(14-8)+MO},  2},

{ { MS*(6-8), 0, MS*(3-8)+MO},  2},
{ { MS*(6-8), 0, MS*(4-8)+MO},  2},
{ { MS*(6-8), 0, MS*(5-8)+MO},  2},
{ { MS*(6-8), 0, MS*(6-8)+MO},  2},
{ { MS*(6-8), 0, MS*(8-8)+MO},  2},
{ { MS*(6-8), 0, MS*(9-8)+MO},  2},
{ { MS*(6-8), 0, MS*(10-8)+MO},  2},
{ { MS*(6-8), 0, MS*(11-8)+MO},  2},
{ { MS*(6-8), 0, MS*(12-8)+MO},  2},

{ { MS*(7-8), 0, MS*(3-8)+MO},  2},
{ { MS*(7-8), 0, MS*(4-8)+MO},  2},
{ { MS*(7-8), 0, MS*(5-8)+MO},  2},
{ { MS*(7-8), 0, MS*(6-8)+MO},  2},
{ { MS*(7-8), 0, MS*(7-8)+MO},  2},
{ { MS*(7-8), 0, MS*(8-8)+MO},  2},
{ { MS*(7-8), 0, MS*(9-8)+MO},  2},
{ { MS*(7-8), 0, MS*(10-8)+MO},  2},
{ { MS*(7-8), 0, MS*(11-8)+MO},  2},
{ { MS*(7-8), 0, MS*(12-8)+MO},  2},

{ { MS*(8-8), 0, MS*(11-8)+MO},  2},
{ { MS*(8-8), 0, MS*(12-8)+MO},  2},
{ { MS*(8-8), 0, MS*(13-8)+MO},  2},
{ { MS*(8-8), 0, MS*(14-8)+MO},  2},

{ { MS*(9-8), 0, MS*(6-8)+MO},  2},
{ { MS*(9-8), 0, MS*(7-8)+MO},  2},
{ { MS*(9-8), 0, MS*(8-8)+MO},  2},

{ { MS*(11-8), 0, MS*(9-8)+MO},  2},
{ { MS*(11-8), 0, MS*(10-8)+MO},  2},
{ { MS*(11-8), 0, MS*(11-8)+MO},  2},
{ { MS*(11-8), 0, MS*(12-8)+MO},  2},

{ { MS*(13-8), 0, MS*(0-8)+MO},  2},
{ { MS*(13-8), 0, MS*(1-8)+MO},  2},
{ { MS*(13-8), 0, MS*(2-8)+MO},  2},
{ { MS*(13-8), 0, MS*(3-8)+MO},  2},
{ { MS*(13-8), 0, MS*(4-8)+MO},  2},
{ { MS*(13-8), 0, MS*(5-8)+MO},  2},
{ { MS*(13-8), 0, MS*(6-8)+MO},  2},
{ { MS*(13-8), 0, MS*(7-8)+MO},  2},
{ { MS*(13-8), 0, MS*(8-8)+MO},  2},
{ { MS*(13-8), 0, MS*(9-8)+MO},  2},
{ { MS*(13-8), 0, MS*(10-8)+MO},  2},
{ { MS*(13-8), 0, MS*(11-8)+MO},  2},

{ { MS*(15-8), 0, MS*(0-8)+MO},  2},
{ { MS*(15-8), 0, MS*(1-8)+MO},  2},
{ { MS*(15-8), 0, MS*(2-8)+MO},  2},
{ { MS*(15-8), 0, MS*(3-8)+MO},  2},
{ { MS*(15-8), 0, MS*(4-8)+MO},  2},
{ { MS*(15-8), 0, MS*(5-8)+MO},  2},
{ { MS*(15-8), 0, MS*(6-8)+MO},  2},
{ { MS*(15-8), 0, MS*(7-8)+MO},  2},
{ { MS*(15-8), 0, MS*(8-8)+MO},  2},
{ { MS*(15-8), 0, MS*(9-8)+MO},  2},
{ { MS*(15-8), 0, MS*(10-8)+MO},  2},
{ { MS*(15-8), 0, MS*(11-8)+MO},  2},
{ { MS*(15-8), 0, MS*(12-8)+MO},  2},
{ { MS*(15-8), 0, MS*(13-8)+MO},  2},
{ { MS*(15-8), 0, MS*(14-8)+MO},  2},

{ { MS*(0-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(2-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(3-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(7-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(11-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(12-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(13-8)+MO, 0, MS*(1-8)},  3},
{ { MS*(14-8)+MO, 0, MS*(1-8)},  3},

{ { MS*(0-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(2-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(3-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(7-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(11-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(12-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(13-8)+MO, 0, MS*(2-8)},  3},
{ { MS*(14-8)+MO, 0, MS*(2-8)},  3},

{ { MS*(3-8)+MO, 0, MS*(3-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(3-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(3-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(3-8)},  3},

{ { MS*(0-8)+MO, 0, MS*(4-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(4-8)},  3},
{ { MS*(2-8)+MO, 0, MS*(4-8)},  3},
{ { MS*(3-8)+MO, 0, MS*(4-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(4-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(4-8)},  3},

{ { MS*(7-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(11-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(12-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(13-8)+MO, 0, MS*(5-8)},  3},
{ { MS*(14-8)+MO, 0, MS*(5-8)},  3},

{ { MS*(1-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(2-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(7-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(11-8)+MO, 0, MS*(6-8)},  3},
{ { MS*(12-8)+MO, 0, MS*(6-8)},  3},

{ { MS*(3-8)+MO, 0, MS*(7-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(7-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(7-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(7-8)},  3},

{ { MS*(3-8)+MO, 0, MS*(8-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(8-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(8-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(8-8)},  3},

{ { MS*(0-8)+MO, 0, MS*(9-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(9-8)},  3},
{ { MS*(7-8)+MO, 0, MS*(9-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(9-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(9-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(9-8)},  3},

{ { MS*(0-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(2-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(3-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(4-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(5-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(6-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(7-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(8-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(9-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(10-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(11-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(12-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(13-8)+MO, 0, MS*(10-8)},  3},
{ { MS*(14-8)+MO, 0, MS*(10-8)},  3},

{ { MS*(0-8)+MO, 0, MS*(15-8)},  3},
{ { MS*(1-8)+MO, 0, MS*(15-8)},  3},

{ {-400,0,-400},  6},
{ {+400,0,-400},  6},
{ {-400,0,+400},  6},
{ {+400,0,+400},  6},

{ {+40000,0,+40000},  12},

 };


point_t objectpoint002[] = { {-RD,+0,-MO-7}, 
                            {-RD,+0,+MO+7}, 
                            {+RD,+0,+MO+7}, 
                            {+RD,+0,-MO-7}, };
BYTE objectface002[] = {  4,  0,1,2,3, };
point_t objectpoint003[] = { {-MO-7,+0,-RD},
                            {-MO-7,+0,+RD}, 
                            {+MO+7,+0,+RD}, 
                            {+MO+7,+0,-RD}, };
BYTE objectface003[] = {  4,  0,1,2,3, };

point_t objectpoint000[] = {
{0,0,0},
{HR_SZ*1,0,0}, {HR_AG*(HR_SZ*1),0,-HR_AG*(HR_SZ*1)}, {0,0,-HR_SZ*1}, {-HR_AG*(HR_SZ*1),0,-HR_AG*(HR_SZ*1)}, {-HR_SZ*1,0,0}, {-HR_AG*(HR_SZ*1),0,HR_AG*(HR_SZ*1)}, {0,0,HR_SZ*1},  {HR_AG*(HR_SZ*1),0,HR_AG*(HR_SZ*1)},
{HR_SZ*2,0,0}, {HR_AG*(HR_SZ*2),0,-HR_AG*(HR_SZ*2)}, {0,0,-HR_SZ*2}, {-HR_AG*(HR_SZ*2),0,-HR_AG*(HR_SZ*2)}, {-HR_SZ*2,0,0}, {-HR_AG*(HR_SZ*2),0,HR_AG*(HR_SZ*2)}, {0,0,HR_SZ*2},  {HR_AG*(HR_SZ*2),0,HR_AG*(HR_SZ*2)},
{HR_SZ*3,0,0}, {HR_AG*(HR_SZ*3),0,-HR_AG*(HR_SZ*3)}, {0,0,-HR_SZ*3}, {-HR_AG*(HR_SZ*3),0,-HR_AG*(HR_SZ*3)}, {-HR_SZ*3,0,0}, {-HR_AG*(HR_SZ*3),0,HR_AG*(HR_SZ*3)}, {0,0,HR_SZ*3}, {HR_AG*(HR_SZ*3),0,HR_AG*(HR_SZ*3)},
{HR_SZ*4,0,0}, {HR_AG*(HR_SZ*4),0,-HR_AG*(HR_SZ*4)}, {0,0,-HR_SZ*4}, {-HR_AG*(HR_SZ*4),0,-HR_AG*(HR_SZ*4)}, {-HR_SZ*4,0,0}, {-HR_AG*(HR_SZ*4),0,HR_AG*(HR_SZ*4)}, {0,0,HR_SZ*4},  {HR_AG*(HR_SZ*4),0,HR_AG*(HR_SZ*4)},
{HR_SZ*5,0,0}, {HR_AG*(HR_SZ*5),0,-HR_AG*(HR_SZ*5)}, {0,0,-HR_SZ*5}, {-HR_AG*(HR_SZ*5),0,-HR_AG*(HR_SZ*5)}, {-HR_SZ*5,0,0}, {-HR_AG*(HR_SZ*5),0,HR_AG*(HR_SZ*5)}, {0,0,HR_SZ*5},  {HR_AG*(HR_SZ*5),0,HR_AG*(HR_SZ*5)},
{HR_SZ*6,0,0}, {HR_AG*(HR_SZ*6),0,-HR_AG*(HR_SZ*6)}, {0,0,-HR_SZ*6}, {-HR_AG*(HR_SZ*6),0,-HR_AG*(HR_SZ*6)}, {-HR_SZ*6,0,0}, {-HR_AG*(HR_SZ*6),0,HR_AG*(HR_SZ*6)}, {0,0,HR_SZ*6}, {HR_AG*(HR_SZ*6),0,HR_AG*(HR_SZ*6)},
{HR_SZ*7,0,0}, {HR_AG*(HR_SZ*7),0,-HR_AG*(HR_SZ*7)}, {0,0,-HR_SZ*7}, {-HR_AG*(HR_SZ*7),0,-HR_AG*(HR_SZ*7)}, {-HR_SZ*7,0,0}, {-HR_AG*(HR_SZ*7),0,HR_AG*(HR_SZ*7)}, {0,0,HR_SZ*7}, {HR_AG*(HR_SZ*7),0,HR_AG*(HR_SZ*7)},
{HR_SZ*8,0,0}, {HR_AG*(HR_SZ*8),0,-HR_AG*(HR_SZ*8)}, {0,0,-HR_SZ*8}, {-HR_AG*(HR_SZ*8),0,-HR_AG*(HR_SZ*8)}, {-HR_SZ*8,0,0}, {-HR_AG*(HR_SZ*8),0,HR_AG*(HR_SZ*8)}, {0,0,HR_SZ*8}, {HR_AG*(HR_SZ*8),0,HR_AG*(HR_SZ*8)},
};
BYTE objectface000[] =  { 3,   0,1,2,  3,   0,2,3,  3,   0,3,4,  3,   0,4,5,  3,   0,5,6,  3,   0,6,7,  3,   0,7,8,  3,   0,8,1,
4,   1+(0*8),1+(1*8),2+(1*8),2+(0*8), 4,   2+(0*8),2+(1*8),3+(1*8),3+(0*8), 4,   3+(0*8),3+(1*8),4+(1*8),4+(0*8), 4,   4+(0*8),4+(1*8),5+(1*8),5+(0*8),
4,   5+(0*8),5+(1*8),6+(1*8),6+(0*8), 4,   6+(0*8),6+(1*8),7+(1*8),7+(0*8), 4,   7+(0*8),7+(1*8),8+(1*8),8+(0*8), 4,   8+(0*8),8+(1*8),1+(1*8),1+(0*8),
4,   1+(1*8),1+(2*8),2+(2*8),2+(1*8), 4,   2+(1*8),2+(2*8),3+(2*8),3+(1*8), 4,   3+(1*8),3+(2*8),4+(2*8),4+(1*8), 4,   4+(1*8),4+(2*8),5+(2*8),5+(1*8),
4,   5+(1*8),5+(2*8),6+(2*8),6+(1*8), 4,   6+(1*8),6+(2*8),7+(2*8),7+(1*8), 4,   7+(1*8),7+(2*8),8+(2*8),8+(1*8), 4,   8+(1*8),8+(2*8),1+(2*8),1+(1*8),
4,   1+(2*8),1+(3*8),2+(3*8),2+(2*8), 4,   2+(2*8),2+(3*8),3+(3*8),3+(2*8), 4,   3+(2*8),3+(3*8),4+(3*8),4+(2*8), 4,   4+(2*8),4+(3*8),5+(3*8),5+(2*8),
4,   5+(2*8),5+(3*8),6+(3*8),6+(2*8), 4,   6+(2*8),6+(3*8),7+(3*8),7+(2*8), 4,   7+(2*8),7+(3*8),8+(3*8),8+(2*8), 4,   8+(2*8),8+(3*8),1+(3*8),1+(2*8),
4,   1+(3*8),1+(4*8),2+(4*8),2+(3*8), 4,   2+(3*8),2+(4*8),3+(4*8),3+(3*8), 4,   3+(3*8),3+(4*8),4+(4*8),4+(3*8), 4,   4+(3*8),4+(4*8),5+(4*8),5+(3*8),
4,   5+(3*8),5+(4*8),6+(4*8),6+(3*8), 4,   6+(3*8),6+(4*8),7+(4*8),7+(3*8), 4,   7+(3*8),7+(4*8),8+(4*8),8+(3*8), 4,   8+(3*8),8+(4*8),1+(4*8),1+(3*8),
4,   1+(4*8),1+(5*8),2+(5*8),2+(4*8), 4,   2+(4*8),2+(5*8),3+(5*8),3+(4*8), 4,   3+(4*8),3+(5*8),4+(5*8),4+(4*8), 4,   4+(4*8),4+(5*8),5+(5*8),5+(4*8),
4,   5+(4*8),5+(5*8),6+(5*8),6+(4*8), 4,   6+(4*8),6+(5*8),7+(5*8),7+(4*8), 4,   7+(4*8),7+(5*8),8+(5*8),8+(4*8), 4,   8+(4*8),8+(5*8),1+(5*8),1+(4*8),
4,   1+(5*8),1+(6*8),2+(6*8),2+(5*8), 4,   2+(5*8),2+(6*8),3+(6*8),3+(5*8), 4,   3+(5*8),3+(6*8),4+(6*8),4+(5*8), 4,   4+(5*8),4+(6*8),5+(6*8),5+(5*8),
4,   5+(5*8),5+(6*8),6+(6*8),6+(5*8), 4,   6+(5*8),6+(6*8),7+(6*8),7+(5*8), 4,   7+(5*8),7+(6*8),8+(6*8),8+(5*8), 4,   8+(5*8),8+(6*8),1+(6*8),1+(5*8),
4,   1+(6*8),1+(7*8),2+(7*8),2+(6*8), 4,   2+(6*8),2+(7*8),3+(7*8),3+(6*8), 4,   3+(6*8),3+(7*8),4+(7*8),4+(6*8), 4,   4+(6*8),4+(7*8),5+(7*8),5+(6*8),
4,   5+(6*8),5+(7*8),6+(7*8),6+(6*8), 4,   6+(6*8),6+(7*8),7+(7*8),7+(6*8), 4,   7+(6*8),7+(7*8),8+(7*8),8+(6*8), 4,   8+(6*8),8+(7*8),1+(7*8),1+(6*8),
4,   1+(7*8),1+(8*8),2+(8*8),2+(7*8), 4,   2+(7*8),2+(8*8),3+(8*8),3+(7*8), 4,   3+(7*8),3+(8*8),4+(8*8),4+(7*8), 4,   4+(7*8),4+(8*8),5+(8*8),5+(7*8),
4,   5+(7*8),5+(8*8),6+(8*8),6+(7*8), 4,   6+(7*8),6+(8*8),7+(8*8),7+(7*8), 4,   7+(7*8),7+(8*8),8+(8*8),8+(7*8), 4,   8+(7*8),8+(8*8),1+(8*8),1+(7*8),
};

point_t objectpoint001[] = {
{MS*(0-8),0,MS*(0-8)},
{MS*(0-8),0,MS*(15-8)},
{MS*(1-8),0,MS*(0-8)},
{MS*(1-8),0,MS*(9-8)},
{MS*(2-8),0,MS*(4-8)},
{MS*(2-8),0,MS*(6-8)},
{MS*(2-8),0,MS*(9-8)},
{MS*(2-8),0,MS*(13-8)},
{MS*(3-8),0,MS*(3-8)},
{MS*(3-8),0,MS*(12-8)},
{MS*(4-8),0,MS*(4-8)},
{MS*(4-8),0,MS*(15-8)},
{MS*(5-8),0,MS*(0-8)},
{MS*(5-8),0,MS*(15-8)},
{MS*(6-8),0,MS*(3-8)},
{MS*(6-8),0,MS*(7-8)},
{MS*(6-8),0,MS*(8-8)},
{MS*(6-8),0,MS*(13-8)},
{MS*(7-8),0,MS*(3-8)},
{MS*(7-8),0,MS*(13-8)},
{MS*(8-8),0,MS*(11-8)},
{MS*(8-8),0,MS*(15-8)},
{MS*(9-8),0,MS*(6-8)},
{MS*(9-8),0,MS*(9-8)},
{MS*(11-8),0,MS*(9-8)},
{MS*(11-8),0,MS*(13-8)},
{MS*(13-8),0,MS*(0-8)},
{MS*(13-8),0,MS*(12-8)},
{MS*(15-8),0,MS*(0-8)},
{MS*(15-8),0,MS*(15-8)},
{MS*(0-8),0,MS*(1-8)},
{MS*(15-8),0,MS*(1-8)},
{MS*(0-8),0,MS*(2-8)},
{MS*(15-8),0,MS*(2-8)},
{MS*(0-8),0,MS*(4-8)},
{MS*(6-8),0,MS*(4-8)},
{MS*(7-8),0,MS*(5-8)},
{MS*(15-8),0,MS*(5-8)},
{MS*(1-8),0,MS*(6-8)},
{MS*(3-8),0,MS*(6-8)},
{MS*(4-8),0,MS*(6-8)},
{MS*(6-8),0,MS*(6-8)},
{MS*(7-8),0,MS*(6-8)},
{MS*(13-8),0,MS*(6-8)},
{MS*(3-8),0,MS*(7-8)},
{MS*(7-8),0,MS*(7-8)},
{MS*(3-8),0,MS*(8-8)},
{MS*(7-8),0,MS*(8-8)},
{MS*(0-8),0,MS*(9-8)},
{MS*(7-8),0,MS*(9-8)},
{MS*(0-8),0,MS*(10-8)},
{MS*(2-8),0,MS*(10-8)},
{MS*(3-8),0,MS*(10-8)},
{MS*(15-8),0,MS*(10-8)},
{MS*(0-8),0,MS*(11-8)},
{MS*(2-8),0,MS*(11-8)},
{MS*(3-8),0,MS*(11-8)},
};
BYTE objectline001[] = { 
 0, 1,
 2, 3,
 4, 5, 
 6, 7,
 8, 9,
 10, 11,
 12, 13,
 14, 15, 
 16, 17,
 18, 19,
 20, 21,
 22, 23,
 24, 25,
 26, 27,
 28, 29,
 30, 31,
 32, 33,
 8, 18,
 34, 35,
 36, 37,
 38, 39, 
 40, 41, 
 42, 43,
 44, 45,
 46, 47,
 48, 6, 
 49, 24,
 50, 51, 
 52, 53,
 54, 55, 
 56, 20,
 17, 25,
 1, 29,
};

point_t objectpoint206[] = {  {-75,+0,-75}, 
                            {-75,+0,+75}, 
                            {+75,+0,-75}, 
                            {+75,+0,+75} };
BYTE objectface206[] =  { 4,  0,1,3,2 };

point_t objectpoint308[] = {  {0.0*100, -0.71*100, -1*100}, 
                            {0.0*100, -0.71*100, +1*100}, 
                            {-1*100, +0.71*100, 0.0*100}, 
                            {+1*100, +0.71*100, 0.0*100} };
BYTE objectline308[] =  { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };

point_t objectpoint309[] = {  {0.0*10, -0.71*10, -1*10}, 
                            {0.0*10, -0.71*10, +1*10}, 
                            {-1*10, +0.71*10, 0.0*10}, 
                            {+1*10, +0.71*10, 0.0*10} };
BYTE objectface309[] =  { 3,  0,1,2,
  3, 1,0,3,
  3, 0,2,3,
  3, 1,3,2, };

point_t objectpoint310[] = {  {0.0*10, -0.171*10, -1*10}, 
                            {0.0*10, -0.171*10, +1*10}, 
                            {-1*10, +0.71*10, 0.0*10}, 
                            {+1*10, +0.71*10, 0.0*10} };
BYTE objectface310[] =  { 3, 0,1,2,
  3, 1,0,3,
  3, 0,2,3,
  3, 1,3,2, };

point_t objectpoint211[] = { {-40,+0,-160}, 
                            {-40,+0,+160}, 
                            {+40,+0,+160}, 
                            {+40,+0,-160}, };
BYTE objectface211[] = {  4, 0,1,2,3, };

point_t objectpoint212[] = { {-16400,+0,-16400}, 
                            {-16400,+0,+16400}, 
                            {+16400,+0,+16400}, 
                            {+16400,+0,-16400}, };
BYTE objectface212[] = {  4, 0,1,2,3, };

OBJECTS cityobjects[] = {
{ {100,100,100},  13},
{ {400,100,400},  13},
};

OBJECTS movingobjects[] = {
{ {500,500,500},  10},
{ {900,900,900},  10},
};

point_t objectpoint401[] = {{-50,-100,-50}, 
                            {-50,-100,+50}, 
                            {-50,+100,-50}, 
                            {-50,+100,+50}, 
                            {+50,-100,-50}, 
                            {+50,-100,+50}, 
                            {+50,+100,-50}, 
                            {+50,+100,+50} };
BYTE objectline401[] = {  
	0,1,
	1,3,
	3,2,
	2,0,
	4,5,
	5,7,
	7,6,
	6,4,
	0,4,	
	1,5,
	2,6,
	3,7,
};

OBJECT object[] = {
                    { 65, &objectpoint000[0], 64, &objectface000[0], 65537.0f, 10000000.0f},
                    { 57, &objectpoint001[0], 33, &objectline001[0], 46341.0f, 10000000.0f},
                    { 4, &objectpoint002[0], 1, &objectface002[0], 2049.0f, 6145.0f},
                    { 4, &objectpoint003[0], 1, &objectface003[0], 2049.0f, 6145.0f},
	{ 0, NULL, 0, NULL, 0.0f, 0.0f},
	{ 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 4, &objectpoint206[0], 1, &objectface206[0], 107.0f, 4097.0f},
	{ 0, NULL, 0, NULL, 0.0f, 0.0f},
	{ 0, NULL, 0, NULL, 0.0f, 0.0f},
	{ 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 4, &objectpoint308[0], 6, &objectline308[0], 900.0f, 2000.0f},
                    { 4, &objectpoint211[0], 1, &objectface211[0], 300.0f, 32767.0f},
                    { 4, &objectpoint212[0], 1, &objectface212[0], 33767.0f, 10000000.0f},
                    { 8, &objectpoint401[0], 12, &objectline401[0], 2049.0f, 6145.0f},
};

FILE *file;
const BYTE ClipCodeList[NUM_FRUSTUM_PLANES]={CLIP_FRONT, CLIP_LEFT, CLIP_RIGHT, CLIP_BOTTOM, CLIP_TOP};
BYTE ClippedPolygon[NUM_FRUSTUM_PLANES][MAX_POLY_VERTS];

int curr_frame=0;
point2D_t project[MAX_POLY_VERTS];
int frame[MAX_POLY_VERTS];
point2D_t polygon[MAX_POLY_VERTS];
BYTE VisibleFace[MAX_OBJECT_POINTS]; // index into faces that need to be displayed
int EdgeList[MAX_SCREEN_HEIGHT][2];

BYTE Points; // number of points in object - set initially then increases as clipped points are added

point_t frustum_normal[4];

BOOL Fly=FALSE;

char string[256];

void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

void debug(const char* str) { sprintf(string,"%s\n",str); debugstring(); }

void dbg(const char* str)
{
#ifdef DEBUG
 debug(str); 
#endif
}

const void bb()
{
	for (int o=0; o!=sizeof(object)/sizeof(object[0]); ++o)
	{
		float sphere=0.0f;
		for (int p=0; p!=object[o].Points; ++p)
		{
			const float bs=sqrt((object[o].Point[p].x*object[o].Point[p].x) + (object[o].Point[p].y*object[o].Point[p].y) + (object[o].Point[p].z*object[o].Point[p].z));
			if (sphere<bs) sphere=bs;
//			sprintf(string,"bb %i %i %f %f %f %f %f\n",o,p,object[o].Point[p].x,object[o].Point[p].y,object[o].Point[p].z, bs, sphere); debugstring();
		}
		sprintf(string,"bb %i %f\n",o,sphere); debugstring();
	}
}

point_t PointSet(const float in1, const float in2, const float in3)
{
  	const point_t p = {in1, in2, in3};
  	return p;
}

void Set_DIB_FOV()
{
  	if (BMInfo.bmiHeader.biHeight > 0)
  	{
    	pDIB = (pDIBBase + (DIBHeight - 1) * DIBWidth);
    	DIBPitch = -DIBWidth;
  	}
  	else
  	{
  		pDIB = pDIBBase;
    	DIBPitch = DIBWidth;
  	}

  	xscreenscale = DIBWidth / FIELD_OF_VIEW;
  	yscreenscale = DIBHeight / FIELD_OF_VIEW;
  	maxscale = max(xscreenscale, yscreenscale);
  	xcenter = (DIBWidth / 2.0f) - 0.5f;
  	ycenter = (DIBHeight / 2.0f) + 0.5f;

  	const float xangle = atan(2.05f / FIELD_OF_VIEW * maxscale / xscreenscale); //temp
  	const float xs = sin(xangle);
  	const float xc = cos(xangle);
  	const float yangle = atan(2.05f / FIELD_OF_VIEW * maxscale / yscreenscale); //temp
  	const float ys = sin(yangle);
  	const float yc = cos(yangle);

  	frustum_normal[0]=PointSet(0.0f, 0.0f, 1.0f);
  	frustum_normal[1]=PointSet(xs, 0.0f, xc);
  	frustum_normal[2]=PointSet(-xs, 0.0f, xc);
  	frustum_normal[3]=PointSet(0.0f ,ys, yc);
  	frustum_normal[4]=PointSet(0.0f ,-ys, yc);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM uParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_COMMAND:  // message: command from application menu
        switch (LOWORD(uParam)) 
        {
        case IDM_EXIT:
            DestroyWindow (hwnd);
            break;
        default:
            return (DefWindowProc(hwnd, message, uParam, lParam));
        }
        break;

    case WM_SIZE:   // window size changed
        if (uParam != SIZE_MINIMIZED) 
        {
          	if (hDIBSection != 0) // Skip when this is called before the first DIB section is created
          	{
          		const HBITMAP holdDIBSection = hDIBSection; // Resize the DIB section to the new size
            	BMInfo.bmiHeader.biWidth = (LOWORD(lParam) +3) & ~3;
            	BMInfo.bmiHeader.biHeight = HIWORD(lParam);;

            	hDIBSection = CreateDIBSection(GetDC(hwnd), (BITMAPINFO*)&BMInfo, DIB_RGB_COLORS, &pDIBBase, NULL, 0);
            	if (hDIBSection) 
            	{
            		DIBWidth = BMInfo.bmiHeader.biWidth;
              		DIBHeight = BMInfo.bmiHeader.biHeight;
              		DeleteObject(holdDIBSection);
              		Set_DIB_FOV();
            	}	
          	} 
        }
        break;

    case WM_DESTROY:  // message: window being destroyed
        DeleteObject(hDIBSection);                     
        PostQuitMessage(0);
        break;

    default:
        return (DefWindowProc(hwnd, message, uParam, lParam)); // Passes it on if unproccessed
    }
    return 0;
}

const BOOL InitApp(const HINSTANCE hInstance)
{
  	WNDCLASS  wc;
  	wc.style         = CS_HREDRAW | CS_VREDRAW; // Fill in window class structure with parameters that describe the main window.
  	wc.lpfnWndProc   = (WNDPROC)WndProc;
  	wc.cbClsExtra    = wc.cbWndExtra = 0;
  	wc.hInstance     = hInstance;
  	wc.hIcon         = LoadIcon (hInstance, "Clip");
  	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  	wc.lpszMenuName  = wc.lpszClassName = "Clip";
  	return RegisterClass(&wc); // Register the window class and return success/failure code.
}

const BOOL InitInst(const HINSTANCE hInstance, const int nCmdShow)
{
  	RECT rctmp={0, 0, DIBWidth, DIBHeight};
  	AdjustWindowRect(&rctmp, WS_OVERLAPPEDWINDOW, FALSE);

  	const HWND hwnd = CreateWindow("Clip", "Clip", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_DLGFRAME /*WS_OVERLAPPEDWINDOW*/, GetSystemMetrics(SM_CXSCREEN) - (rctmp.right - rctmp.left), GetSystemMetrics(SM_CYSCREEN) - (rctmp.bottom - rctmp.top), rctmp.right - rctmp.left, rctmp.bottom - rctmp.top, NULL, NULL, hInstance, NULL);
  	if (!hwnd)
  	{
    	return FALSE;
  	}

	const BITMAPINFOHEADER BMIH = {sizeof(BITMAPINFOHEADER), DIBWidth, DIBHeight, 1, 8, BI_RGB, 0, 0, 0, COLOURS_USED, COLOURS_USED};
  	memcpy(&BMInfo.bmiHeader, &BMIH, sizeof(BMIH));

	const RGBQUAD colours[] = { {255,0,0,0}, {0,128,0,0}, {0,0,0,0}, {255,255,255,0} };
  	memcpy(&BMInfo.bmiColors[0], &colours[0], sizeof(BMInfo.bmiColors[0])*COLOURS_USED);

  	const HPALETTE log_palette = CreatePalette((LOGPALETTE*)&PalInfo); // create the palette
  	const HDC hdc = GetDC(hwnd);
  	SelectPalette(hdc, log_palette, FALSE); // select it for that DC
  	RealizePalette(hdc); // realize a palette on that DC
  	DeleteObject(log_palette); // delete palette handler

  	hDIBSection = CreateDIBSection(hdc, (BITMAPINFO*)&BMInfo, DIB_RGB_COLORS, &pDIBBase, NULL, 0);
  	if (!hDIBSection) 
  	{
   		return FALSE;
  	}

  	Set_DIB_FOV();

  	ShowWindow(hwnd, nCmdShow); // Show the window

  	ReleaseDC(hwnd, hdc);
  	hwndOutput = hwnd;

  	file=fopen("log.txt","w");
//  	sprintf(string,"bb %f\n",123.45); debugstring();
 
  bb();

  	return TRUE;
}

const point_t PointAdd(const point_t *in1, const point_t *in2)
{
  	return PointSet(in1->x+in2->x, in1->y+in2->y, in1->z+in2->z);
}
const point_t PointMinus(const point_t *in1, const point_t *in2)
{
  	return PointSet(in1->x-in2->x, in1->y-in2->y, in1->z-in2->z);
}
const float DotProduct(const point_t *vec1, const point_t *vec2)
{
	return (vec1->x * vec2->x) + (vec1->y * vec2->y) + (vec1->z * vec2->z);
}
const point_t CrossProduct(const point_t *in1, const point_t *in2)
{
  	return PointSet((in1->y * in2->z) - (in1->z * in2->y), (in1->z * in2->x) - (in1->x * in2->z), (in1->x * in2->y) - (in1->y * in2->x));
}

const void Plot(const int x, const int y, const BYTE colour) { *(pDIB + (DIBPitch * y) + x)=colour; }

const void Line(const int startx, const int starty, const int endx, const int endy, const BYTE colour)
{
	float sx;
	int sy,ey,ex;
	if (startx<=endx)
	{
		sx=startx;
		ex=endx;
		sy=starty;
		ey=endy;
	}
	else
	{
		sx=endx;
		ex=startx;
		sy=endy;
		ey=starty;
	}
	int pitch,ydir,scy,ecy;
	if (sy<=ey)
	{
		pitch=DIBPitch;
		scy=sy;
		ecy=ey+1;
		ydir=1;
	}
	else
	{
		pitch=-DIBPitch;
		scy=sy+1;
		ecy=ey;
		ydir=-1;
	}
	const float xslope=fabs((ex-sx) / (ecy-scy));
	if (xslope <=1.0f)
	{
		for (char *screen_line=(char *)pDIB + (sy*DIBPitch); scy!=ecy; scy+=ydir, sx+=xslope, screen_line+=pitch)
		{
		  *(screen_line + (int)sx)=colour;
		}
	}
	else
	{
		for (char *screen_line=(char *)pDIB + (sy*DIBPitch); scy!=ecy; scy+=ydir, sx+=xslope, screen_line+=pitch)
		{
			memset(screen_line+(int)sx,colour,xslope);
		}
	}
}

const void Fill(const point2D_t *point, const BYTE points, const BYTE colour)
{
  	memset(&EdgeList[0][0],0,sizeof(EdgeList[0][0])*DIBHeight);
  	memset(&EdgeList[0][1],0,sizeof(EdgeList[0][0])*DIBHeight);

	int ymax=point[0].y;
	int ymin=point[0].y;

	for (int p=0, curr=points-1; p!=points; curr=p, ++p)
	{
		float xs, xe;
		int edge, ys, ye;

		if (point[curr].y>point[p].y)
		{
			edge=0;
			xs=point[p].x;
			xe=point[curr].x;
			ys=point[p].y;
			ye=point[curr].y;
		}
		else
		{
			edge=1;
			xs=point[curr].x;
			xe=point[p].x;
			ys=point[curr].y;
			ye=point[p].y;
		}

		if (ye>ymax) ymax=ye;
		if (ys<ymin) ymin=ys;

		if (ys==ye)
		{
			if (xs<xe)
			{
        		if (EdgeList[ys][0]==0) EdgeList[ys][0]=xs-1;
			}
			else
			{
       			if (EdgeList[ys][1]==0) EdgeList[ys][1]=xs+1;
			}
		}
		else
		{
			const int ydiff=ye-ys;
			const float xdiff=xe-xs;
			const float xslope=xdiff/ydiff;

			for (; ys<ye; ++ys, xs+=xslope)
			{
				EdgeList[ys][edge]=xs;
			}
		}
	}

	for (char *screen_line=(char *)pDIB+(DIBPitch*ymin); ymin<ymax; ++ymin, screen_line+=DIBPitch)
	{
		if (EdgeList[ymin][0] <= EdgeList[ymin][1])
		{
			memset(screen_line + EdgeList[ymin][0], colour,EdgeList[ymin][1] - EdgeList[ymin][0] + 1);
		}
		else
		{
			memset(screen_line + EdgeList[ymin][1], colour,EdgeList[ymin][0] - EdgeList[ymin][1] + 1);
		}
	}
}

const point2D_t ProjectPoint(const point_t *pin)
{
  	const float zrecip = (1.0f / (pin->z+EPSILON)) * maxscale;
  	point2D_t pout;
  	pout.x = pin->x * zrecip + xcenter;
  	pout.y = DIBHeight - (pin->y * zrecip + ycenter);

  	if (pout.x <0) 
  	{
    	sprintf(string,"ppxl %i\n",pout.x); debugstring();
    	pout.x=0; 
  	}
  	else if (pout.x >DIBWidth-1) 
  	{
  		sprintf(string,"ppxr %i %i\n",pout.x, DIBWidth-1); debugstring();
    	pout.x=DIBWidth-1;
  	}
  	if (pout.y <0) 
  	{
  		sprintf(string,"ppyl %i\n",pout.y); debugstring();
    	pout.y=0;
  	}
  	else if (pout.y >DIBHeight-1) 
  	{
  		sprintf(string,"ppyr %i %i\n",pout.y, DIBHeight-1); debugstring();
  	  pout.y=DIBHeight-1;
  	}
  	return pout;
}

const int BoundingSphere(const point_t *location, const float Bounding_Sphere)
{
	int result=ALL_ON;
	for (int f=0; f!=NUM_FRUSTUM_PLANES; ++f)
  	{
	    const float dot = DotProduct(location, &frustumplanes[f].normal);
    	if (dot+Bounding_Sphere<frustumplanes[f].distance)
    	{
    		return ALL_OFF;
    	}
	    if (dot-frustumplanes[f].distance<Bounding_Sphere)
    	{
			result=INTERSECTING;
	    }
 	}
  	return result;
}

const BOOL Flying()
{
  	return ( (Fly==TRUE) && (currentpos.y!=1.0f) );
}

const void Rotate(const point_t *dir, point_t *right, point_t *up, point_t *in)
{
  	const float rs = sin(dir->x); // Set up the world-to-view rotation. Note: much of the work done in concatenating these matrices can be factored out, since it contributes nothing to the final result; multiply the three matrices together on paper to generate a minimum equation for each of the 9 final elements
  	const float rc = cos(dir->x);
  	const float ps = sin(dir->y);
  	const float pc = cos(dir->y);
  	const float ys = sin(dir->z);
  	const float yc = cos(dir->z);

  	const float mtemp2[3][3] =
	{
		{
			(rc * yc), 
			rs, 
			(rc * -ys)
		}, 
		{ 
			(pc * -rs * yc) + (ps * ys),
			(pc * rc), 
			(pc * -rs * -ys) + (ps * yc) 
		},
		{
			(-ps * -rs * yc) + (pc * ys),
			-ps * rc, 
			(-ps * -rs * -ys) + (pc * yc)
		} 
	};
  	*right = PointSet( mtemp2[0][0], mtemp2[0][1], mtemp2[0][2] ); // Break out the rotation matrix into vright, vup, and vpn. We could work directly with the matrix; breaking it out into three vectors is just to make things clearer
  	*up = PointSet( mtemp2[1][0],mtemp2[1][1],mtemp2[1][2] );
  	*in = PointSet( mtemp2[2][0], mtemp2[2][1], mtemp2[2][2] );

	const char irs = rs *127.0f;
	const char irc = rc *127.0f;
	const char ips = ps *127.0f;
	const char ipc = pc *127.0f;
	const char iys = ys *127.0f;
	const char iyc = yc *127.0f;
	
	const short iprc_pyc = irc * iyc;
	const float fprc_pyc = rc * yc;
	
	const short iprc_mys = irc * -iys;
	const float fprc_mys = rc * -ys;

	const long ippc_mrs_pyc = (ipc * -irs * iyc) /16384;
	const float fppc_mrs_pyc = pc * -rs * yc;

	const short ipps_pys = (ips * iys) /128;
	const float fpps_pys = ps * ys;

	const short ippc_prc = ipc * irc;
	const float fppc_prc = pc * rc;

	const long ippc_mrs_mys = (ipc * -irs * -iys) /16384;
	const float fppc_mrs_mys = pc * -rs * -ys;

	const short ipps_pyc = (ips * iyc) /128;
	const float fpps_pyc = ps * yc;

	const long imps_mrs_pyc = (-ips * -irs * iyc) /16384;
	const float fmps_mrs_pyc = -ps * -rs * yc;

	const short ippc_pys = (ipc * iys) /128;
	const float fppc_pys = pc * ys;

	const short imps_prc = (-ips * irc) /128;
	const float fmps_prc = -ps * rc;

	const long imps_mrs_mys = (-ips * -irs * -iys) /16384;
	const float fmps_mrs_mys = -ps * -rs * -ys;

	const short ippc_pyc = (ipc * iyc) /128;
	const float fppc_pyc = pc * yc;

	const short ix0= iprc_pyc /128;
	const char iy0= irs;
	const short iz0= iprc_mys /128;
	const long ix1= (ippc_mrs_pyc) + (ipps_pys);
	const short iy1= (ippc_prc /128);
	const long iz1= ippc_mrs_mys + ipps_pyc;
	const long ix2= imps_mrs_pyc + ippc_pys;
	const short iy2= imps_prc;
	const long iz2= imps_mrs_mys + ippc_pyc;

	const float fx0= fprc_pyc;
	const float fy0= rs;
	const float fz0= fprc_mys;
	const float fx1= fppc_mrs_pyc + fpps_pys;

	sprintf(string,"Tst %f %f %i %i\n",fppc_mrs_pyc,fpps_pys,ippc_mrs_pyc,ipps_pys); debugstring();

	const float fy1= fppc_prc;
	const float fz1= fppc_mrs_mys + fpps_pyc;
	const float fx2= fmps_mrs_pyc + fppc_pys;
	const float fy2= fmps_prc;
	const float fz2= fmps_mrs_mys + fppc_pyc;
	
//	sprintf(string,"Rot %f %f %f\n",dir->x,dir->y,dir->z); debugstring();

//	sprintf(string,"fcs %f %f %f %f %f %f\n",rs,rc,ps,pc,ys,yc); debugstring();
//	sprintf(string,"ics %i %i %i %i %i %i\n",irs,irc,ips,ipc,iys,iyc); debugstring();

//	sprintf(string,"00 %f %i\n",mtemp2[0][0],ix0); debugstring();
//	sprintf(string,"01 %f %i/%i\n",mtemp2[0][1],ix0,ix1,iy0); debugstring();

//	sprintf(string,"RX %f %f %f\n",right->x,right->y,right->z); debugstring();
//	sprintf(string,"RY %f %f %f\n",up->x,up->y,up->z); debugstring();
//	sprintf(string,"RZ %f %f %f\n",in->x,in->y,in->z); debugstring();
///	sprintf(string,"RT %f %f %f\n",right->x+right->y+right->z, up->x+up->y+up->z, in->x+in->y+in->z); debugstring();

//	sprintf(string,"RXi %i %i %i\n",ix0,ix1,ix2); debugstring();
//	sprintf(string,"RYi %i %i %i\n",iy0,iy1,iy2); debugstring();
//	sprintf(string,"RZi %i %i %i\n",iz0,iz1,iz2); debugstring();
///	sprintf(string,"RTi %i %i %i\n",right->x+right->y+right->z, up->x+up->y+up->z, in->x+in->y+in->z); debugstring();

	sprintf(string,"m00 %f %f %i %f\n",mtemp2[0][0],fx0,ix0,127*fx0); debugstring();
	sprintf(string,"m01 %f %f %i %f\n",mtemp2[0][1],fy0,iy0,127*fy0); debugstring();
	sprintf(string,"m02 %f %f %i %f\n",mtemp2[0][2],fz0,iz0,127*fz0); debugstring();
	sprintf(string,"m10 %f %f %i %f\n",mtemp2[1][0],fx1,ix1,127*fx1); debugstring();
	sprintf(string,"m11 %f %f %i %f\n",mtemp2[1][1],fy1,iy1,127*fy1); debugstring();
	sprintf(string,"m12 %f %f %i %f\n",mtemp2[1][2],fz1,iz1,127*fz1); debugstring();
	sprintf(string,"m20 %f %f %i %f\n",mtemp2[2][0],fx2,ix2,127*fx2); debugstring();
	sprintf(string,"m21 %f %f %i %f\n",mtemp2[2][1],fy2,iy2,127*fy2); debugstring();
	sprintf(string,"m22 %f %f %i %f\n",mtemp2[2][2],fz2,iz2,127*fz2); debugstring();
}

const void UpdateViewPos()
{
	const point_t xaxis = {1, 0, 0};
	const point_t yaxis = {0, 1, 0};
  	const point_t zaxis = {0, 0, 1};

  	const point_t motionvec={DotProduct(&vpn, &xaxis), Fly==TRUE ? DotProduct(&vpn, &yaxis) : 0.0f, DotProduct(&vpn, &zaxis)}; // Move in the view direction, across the x-y plane, as if walking. This approach moves slower when looking up or down at more of an angle

  	const point_t temp_pos={ motionvec.x * currentspeed,  motionvec.y * currentspeed, motionvec.z * currentspeed};
  	currentpos=PointAdd(&temp_pos, &currentpos);
  	currentdir=PointAdd(&currentdir, &currentturnspeed);

  	Rotate(&currentdir, &vright, &vup, &vpn);

  	currentspeed = fabs(currentspeed) > EPSILON ? currentspeed *= FRICTION : 0.0f;
  	currentdir.x = fabs(currentdir.x) > EPSILON ? currentdir.x *= FRICTION : 0.0f;
  	currentturnspeed = PointSet(fabs(currentturnspeed.x) > EPSILON ? currentturnspeed.x *= FRICTION : 0.0f, fabs(currentturnspeed.y) > EPSILON ? currentturnspeed.y *= FRICTION : 0.0f, fabs(currentturnspeed.z) > EPSILON ? currentturnspeed.z *= FRICTION : 0.0f);
}

const point_t BackRotateVector(const point_t *pin) // Rotate a vector from viewspace to worldspace. // Rotate into the world orientation
{
	const point_t p = { (pin->x * vright.x) + (pin->y * vup.x) + (pin->z * vpn.x), (pin->x * vright.y) + (pin->y * vup.y) + (pin->z * vpn.y), (pin->x * vright.z) + (pin->y * vup.z) + (pin->z * vpn.z) };
	return p;
}

const point_t TransformPoint(const point_t *pin) // Transform a point from worldspace to viewspace. // Translate into a viewpoint-relative coordinate
{
	const point_t tvert = PointMinus(pin, &currentpos);
	const point_t p={DotProduct(&tvert, &vright), DotProduct(&tvert, &vup), DotProduct(&tvert, &vpn) };
  	return p;
}

const float PolyFacesViewer(const BYTE i0, const BYTE i1, const BYTE i2) // Returns true if polygon faces the viewpoint, assuming a clockwise winding of vertices as seen from the front.
{
	const point_t *p0=&Vertex[i0].Point;
	const point_t *p1=&Vertex[i1].Point;
	const point_t *p2=&Vertex[i2].Point;

	const point_t edge1=PointMinus(p0, p1);
  	const point_t edge2=PointMinus(p2, p1);
	const point_t normal=CrossProduct(&edge1, &edge2);
  	const point_t viewvec=PointMinus(p0, &currentpos);

  	return DotProduct(&viewvec, &normal);
}

const plane_t SetWorldspaceClipPlane(const point_t *normal) // Set up a clip plane with the specified normal.
{
	plane_t	plane;
	plane.normal=BackRotateVector(normal); // Rotate the plane normal into worldspace
	plane.distance = DotProduct(&currentpos, &plane.normal) + EPSILON;
	return plane;
}

const void SetUpFrustum() // Set up the planes of the frustum, in worldspace coordinates.
{
	frustumplanes[0]=SetWorldspaceClipPlane(&frustum_normal[0]);
	frustumplanes[1]=SetWorldspaceClipPlane(&frustum_normal[1]);
	frustumplanes[2]=SetWorldspaceClipPlane(&frustum_normal[2]);
	frustumplanes[3]=SetWorldspaceClipPlane(&frustum_normal[3]);
	frustumplanes[4]=SetWorldspaceClipPlane(&frustum_normal[4]);
}

const BYTE ClassifyPoint(const point_t *point)
{
  BYTE clipcodeor=0;
  for (int f=0; f!=NUM_FRUSTUM_PLANES; ++f)
  {
    if (DotProduct(point, &frustumplanes[f].normal) <frustumplanes[f].distance)
    {
		clipcodeor |= ClipCodeList[f];
    }
  }
  return clipcodeor;
}

const BYTE MovePointsOffScreen(const point_t *point, const point_t *centre, const BYTE points, BYTE *ClipCodesOr)
{
	*ClipCodesOr=0;
	BYTE clipcodesand=255;

  	for (int p=0; p!=points; ++p)
    {
		Vertex[p].Point=PointSet(point[p].x+centre->x, point[p].y+centre->y, point[p].z+centre->z);
  		Vertex[p].ClipCode=ClassifyPoint(&Vertex[p].Point);
  		clipcodesand &= Vertex[p].ClipCode;
  		*ClipCodesOr |= Vertex[p].ClipCode;
  	}
	return clipcodesand;
}

const void MovePointsOnScreen(const point_t *point, const point_t *centre, const BYTE points)
{
  	for (int p=0; p!=points; ++p)
  	{
  		Vertex[p].Point=PointSet(point[p].x+centre->x, point[p].y+centre->y, point[p].z+centre->z);
  	}
}

const void DrawPolygonObject(const BYTE *visibleface, const int faces, const BYTE Colour)
{
	++curr_frame;
  	for (int pos=0, f=0, points; f!=faces; ++f)
  	{
		points=visibleface[pos++];
		for (int fp=0; fp!=points; ++fp)
		{
			const BYTE index=visibleface[pos++];
			if (frame[index]!=curr_frame)
			{
 				frame[index]=curr_frame;
				const point_t transformed_point=TransformPoint(&Vertex[index].Point);
        		project[index]=ProjectPoint(&transformed_point);
      		}
    		polygon[fp].x=project[index].x;
			polygon[fp].y=project[index].y;
		}
		Fill(&polygon[0], points, Colour);
  	}
}

const BYTE ClassifyFace(const BYTE *faceindex, const BYTE points, BYTE *ClipCodesOr)
{
  	*ClipCodesOr = 0;
  	BYTE ClipCodesAnd = 255;
  	for (int p=0; p!=points; ++p)
  	{
    	const BYTE vertex=faceindex[p];
    	const BYTE clipflags=Vertex[vertex].ClipCode;
    	ClipCodesAnd &= clipflags;
    	*ClipCodesOr |= clipflags;
  	}
  	return ClipCodesAnd;
}

const BYTE AddPoint(const Point3D *in, const Point3D *out, const float scale)
{
  	Vertex[Points].Point.x = in->Point.x + ((out->Point.x - in->Point.x) * (scale-EPSILON));
  	Vertex[Points].Point.y = in->Point.y + ((out->Point.y - in->Point.y) * (scale-EPSILON));
  	Vertex[Points].Point.z = in->Point.z + ((out->Point.z - in->Point.z) * (scale-EPSILON));
  	for (int i=0; i!=Points; ++i)
  	{
    	if ((fabs(Vertex[i].Point.x-Vertex[Points].Point.x)<EPSILON /*1.0f 2.0f 3.0f*/) && (fabs(Vertex[i].Point.y-Vertex[Points].Point.y)<EPSILON) && (fabs(Vertex[i].Point.z-Vertex[Points].Point.z)<EPSILON))
    	{
    		return i;
    	}
  	}
  	Vertex[Points].ClipCode=ClassifyPoint(&Vertex[Points].Point);
  	++Points;
  	return Points-1;
}

BYTE ClipToPlane(const BYTE *vertex_list_in, const int *vertex_count, const plane_t *pplane, BYTE *vertex_list_out, BYTE clip_code)
{
  	BYTE vertex_out=0;
  	float cur_dot = DotProduct(&Vertex[vertex_list_in[0]].Point, &pplane->normal);
  	int cur_in = (cur_dot >= pplane->distance);
//int cur_in = Vertex[vertex_list_in[0]].ClipCode & clip_code;
  	for (int i=0; i!=*vertex_count; ++i)
  	{
    	if (cur_in)
    	{
    		vertex_list_out[vertex_out++] = vertex_list_in[i];
    	}
    	const int nextvert = (i + 1) % *vertex_count;
    	const float next_dot = DotProduct(&Vertex[vertex_list_in[nextvert]].Point, &pplane->normal);
    	const int next_in = (next_dot >= pplane->distance);

    	if (cur_in != next_in)
    	{
      		vertex_list_out[vertex_out++]=AddPoint(&Vertex[vertex_list_in[i]], &Vertex[vertex_list_in[nextvert]], (pplane->distance - cur_dot) / (next_dot - cur_dot));
    	}
    	cur_dot = next_dot;
    	cur_in = next_in;
  	}
  	if (vertex_out <3) return 0;
  	if (ClassifyFace(&vertex_list_out[0], vertex_out, &clip_code)!=0) return 0; // if all off one side
  	return vertex_out;
}

BYTE *ClipToFrustum(BYTE *pin, int *vertex_count, const BYTE clip_codes_or)
{
  	BYTE *ppoly = pin;
  	for (int f=0; f!=NUM_FRUSTUM_PLANES; ++f)
  	{
    	if ((clip_codes_or & ClipCodeList[f])!=0)
    	{
      		*vertex_count=ClipToPlane(ppoly, vertex_count, &frustumplanes[f], &ClippedPolygon[f][0], ClipCodeList[f]);
      		if (*vertex_count==0) return NULL;
      		ppoly = &ClippedPolygon[f][0];
    	}
	}
  	return ppoly;
}

const BOOL ClipLineToFrustum(BYTE *line_start, BYTE *line_end)
{
  	for (int f=0; f!=NUM_FRUSTUM_PLANES; ++f)
  	{
    	const float start_dot = DotProduct(&Vertex[*line_start].Point, &frustumplanes[f].normal);
    	const float end_dot = DotProduct(&Vertex[*line_end].Point, &frustumplanes[f].normal);
    	const int start_in = (start_dot >= frustumplanes[f].distance);
    	const int end_in = (end_dot >= frustumplanes[f].distance);

    	if (start_in!=end_in) // either is off edge
    	{
      		if (start_in==FALSE) // start is off edge
      		{
        		*line_start=AddPoint(&Vertex[*line_start], &Vertex[*line_end], (frustumplanes[f].distance - start_dot) / (end_dot - start_dot));
      		}
      		else // end is off edge
      		{
        		*line_end=AddPoint(&Vertex[*line_start], &Vertex[*line_end], (frustumplanes[f].distance - start_dot) / (end_dot - start_dot));
      		}
    	}
    	else //  both the same
    	{
      		if (start_in==FALSE) // start is off edge
      		{
        		return FALSE;
      		}
    	}
  	}
  	return TRUE;
}

const void DrawPolygonObjects(const BYTE type, const point_t center, float boundingsphere, const BYTE Colour)
{
	if (boundingsphere==INTERSECTING)
    {
      BYTE ClipCodesOr;
      if (MovePointsOffScreen(object[type].Point, &center, object[type].Points, &ClipCodesOr)!=0) return; // all off one side
      if (ClipCodesOr==0) boundingsphere=ALL_ON; // nothing needs clipped
    }
    else
    {
    	MovePointsOnScreen(object[type].Point, &center, object[type].Points); // all on screen
    }

    int Pos=0;
    int faces=0;
    Points=object[type].Points; // set initial number of points in object
    if (boundingsphere==ALL_ON) // clip not needed
    {
		for (int pos=0, f=0; f!=object[type].Faces; ++f)
		{
			const BYTE facepoints=object[type].Face[pos];
			++faces;					// add this face
			memcpy(&VisibleFace[Pos], &object[type].Face[pos], (facepoints+1)*sizeof(VisibleFace[0])); // copy face indexes across
			Pos+=(facepoints+1);
			pos+=(facepoints+1);
		}
    }
    else // clip needed
    {
		for (int pos=0, f=0, facepoints, newverts; f!=object[type].Faces; ++f, pos+=(facepoints+1))
		{
			  facepoints=newverts=object[type].Face[pos];

				BYTE* objectfacepos=&object[type].Face[pos]+1; // store position of face
				BYTE PolygonClipCodesOr;
				if ((ClassifyFace(objectfacepos, facepoints, &PolygonClipCodesOr)!=0)) continue; // if all off one side
				if (PolygonClipCodesOr!=0) // if face needs clipped
				{
					objectfacepos=ClipToFrustum(objectfacepos, &newverts, PolygonClipCodesOr); // clip face
					if (objectfacepos==NULL) continue; // if face fully clipped
				}
				++faces;          // add this face
				VisibleFace[Pos++]=newverts; // copy new verts
				memcpy(&VisibleFace[Pos], objectfacepos, newverts*sizeof(VisibleFace[0])); // copy face indexes
				Pos+=newverts; // inc pos by verts
		}
    }
	DrawPolygonObject(&VisibleFace[0], faces, Colour);
}

const BYTE ObjectVisible(const BYTE type, const point_t *centre, const point_t *dist)
{
  	const float vanishingpoint=object[type].VanishingPoint;
  	if ( (fabs(dist->x)>vanishingpoint) || (fabs(dist->y)>vanishingpoint) || (fabs(dist->z)>vanishingpoint) ) { return ALL_OFF; }

  	return BoundingSphere(centre, object[type].BoundingSphere);
}

const void DrawLineObject(const BYTE *visibleface, const int faces, const BYTE colour)
{
  	++curr_frame;
  	for (int p=0, l=0; l!=faces; ++l, p+=2)
  	{
    	for (int i=0; i!=2; ++i)
    	{
    		const BYTE idx=visibleface[p+i];
      		if (frame[idx]!=curr_frame)
      		{
        		frame[idx]=curr_frame;
        		const point_t transformed_point=TransformPoint(&Vertex[idx].Point);
        		project[idx]=ProjectPoint(&transformed_point);
      		}
    	}
    	Line(project[visibleface[p]].x, project[visibleface[p]].y, project[visibleface[p+1]].x, project[visibleface[p+1]].y, colour);
  	}
}

const void DrawLineObjects(const BYTE Type, const point_t *Centre, BYTE Boundingsphere, const BYTE Colour)
{
  	if (Boundingsphere==INTERSECTING)
  	{
    	BYTE ClipCodesOr;
    	if (MovePointsOffScreen(object[Type].Point, Centre, object[Type].Points, &ClipCodesOr)!=0) return; // all off one side
    	if (ClipCodesOr==0) Boundingsphere=ALL_ON; // nothing needs clipped
  	}
  	else
  	{
    	MovePointsOnScreen(object[Type].Point, Centre, object[Type].Points); // all on screen
  	}

  	int faces=0;
  	Points=object[Type].Points; // set initial number of points in object
  	if (Boundingsphere==ALL_ON) // clip not needed
  	{
    	faces=object[Type].Faces;
    	memcpy(&VisibleFace[0], &object[Type].Face[0], sizeof(VisibleFace[0])*faces*2);
  	}
  	else // clip needed
  	{
    	for (int pos=0, p=0, l=0; l!=object[Type].Faces; ++l, p+=2)
    	{
    		BYTE line_s=object[Type].Face[p];
      		BYTE line_e=object[Type].Face[p+1];
      		const BYTE clip_s=Vertex[line_s].ClipCode;
      		const BYTE clip_e=Vertex[line_e].ClipCode;
      		if ((clip_s & clip_e)==0) // both not off same side
      		{
        		if ((clip_s | clip_e)==0) // both on
        		{
          			++faces;
          			memcpy(&VisibleFace[pos], &object[Type].Face[p], sizeof(VisibleFace[0])*2);
          			pos+=2;
        		}
        		else
        		{
          			if (ClipLineToFrustum(&line_s, &line_e)==TRUE)
          			{
            			++faces;
            			VisibleFace[pos]=line_s;
            			VisibleFace[pos+1]=line_e;
            			pos+=2;
          			}
        		}
      		}
    	}
  }
  DrawLineObject(&VisibleFace[0], faces, Colour);
}

/*const void DrawVisibleLineObjects(const OBJECTS *objectlist, const int numobjects, const BYTE Colour)
{
  	for (int o=0; o!=numobjects; ++o)
  	{
    	const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    	const BYTE boundingsphere=ObjectVisible(objectlist[o].Type, &objectlist[o].Location, &dist);
    	if (boundingsphere==ALL_OFF) continue;
    	DrawLineObjects(objectlist[o].Type, &objectlist[o].Location, boundingsphere, Colour);
  	}
}*/

const void DrawPlotVisibleLineObjects(const OBJECTS *objectlist, const int numobjects, const BYTE Colour)
{
  	for (int o=0; o!=numobjects; ++o)
  	{

    	const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    	const BYTE boundingsphere=ObjectVisible(objectlist[o].Type, &objectlist[o].Location, &dist);
// sprintf(string,"dpvlo %i %i %i %i %f %f %f\n",o,numobjects,boundingsphere,ALL_OFF,dist.x,dist.y,dist.z); debugstring();
    	if (boundingsphere==ALL_OFF)
		{
			if (BoundingSphere(&objectlist[o].Location, EPSILON)==ALL_ON)
			{
		        const point_t transformed_point=TransformPoint(&objectlist[o].Location);
				const point2D_t project_point=ProjectPoint(&transformed_point);
				Plot(project_point.x, project_point.y, Colour);
			}
			continue;
		}
    	DrawLineObjects(objectlist[o].Type, &objectlist[o].Location, boundingsphere, Colour);
  	}
}

const void DrawVisiblePolygonObjects(const OBJECTS *objectlist, const int numobjects, const BYTE Colour)
{
  	for (int o=0; o!=numobjects; ++o)
  	{
    	const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    	const float boundingsphere=ObjectVisible(objectlist[o].Type, &objectlist[o].Location, &dist);
    	if (boundingsphere==ALL_OFF) continue;
		DrawPolygonObjects(objectlist[o].Type, PointSet( objectlist[o].Location.x, objectlist[o].Location.y, objectlist[o].Location.z ), boundingsphere, Colour);
	}
}

const void Collision()
{
 	if (currentpos.y <1.0f)
 	{
  		currentspeed=currentturnspeed.x=0.0f;
  		currentpos.y=1.0f;
 	}
}

const void Input()
{
	if (GetAsyncKeyState(VK_UP)) currentturnspeed.y+=TURN_SPEED;    
	if (GetAsyncKeyState(VK_DOWN)) currentturnspeed.y-=TURN_SPEED;

	if (GetAsyncKeyState(VK_RIGHT))
	{
		currentturnspeed.z+=TURN_SPEED;
	    if (Flying()==TRUE) if (currentturnspeed.x > -TILT_LIMIT) currentturnspeed.x-=TILT_FRICTION;
  	}
  	if (GetAsyncKeyState(VK_LEFT))
  	{
    	currentturnspeed.z-=TURN_SPEED;
    	if (Flying()==TRUE) if (currentturnspeed.x < TILT_LIMIT) currentturnspeed.x+=TILT_FRICTION;
  	}
  	if (GetAsyncKeyState('A')) currentspeed+=ACCEL;
  	if (GetAsyncKeyState('Z')) currentspeed-=ACCEL;

  	if (GetAsyncKeyState('W')) if (Flying()!=TRUE) Fly=FALSE;
  	if (GetAsyncKeyState('F')) Fly=TRUE;
}

const void UpdateWorld() // Render the current state of the world to the screen.
{
  	Input();
  	Collision();
  	UpdateViewPos();
  	SetUpFrustum();

  	memset(pDIBBase, SKY, DIBWidth*DIBHeight);    // clear frame

  	const OBJECTS horizon = { {currentpos.x,currentpos.y-400,currentpos.z}, 0};
  	DrawVisiblePolygonObjects(&horizon, 1, GND); // horizon (green)

	DrawPlotVisibleLineObjects(&groundobjects[0], 1, ROD); // city (black) // DrawPlotVisibleLineObjects

  	DrawVisiblePolygonObjects(&groundobjects[1], sizeof(groundobjects) / sizeof(groundobjects[0])-1, ROD); // ground objects (black)

  	DrawPlotVisibleLineObjects(&cityobjects[0], 2, BLD); // city buildings (white)

  	DrawPlotVisibleLineObjects(&movingobjects[0], 2, BLD); // movable objects (white)

  	const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  	const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  	const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  	BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);
	
//	StretchBlt
  	SelectObject(hdcDIBSection, holdbitmap);
  	DeleteDC(hdcDIBSection);
  	DeleteObject(holdbitmap);
  	ReleaseDC(hwndOutput,hdcScreen);

  	Sleep(1); // 20ms = 1/50s
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  	if (!InitApp(hInstance)) // Initialize shared things
  	{ 
    	return FALSE;     // Exits if unable to initialize
  	}
  	if (!InitInst(hInstance, nCmdShow)) // Perform initializations that apply to a specific instance
  	{
    	return FALSE;
  	}

  	MSG msg;
  	for (;;) // Acquire and dispatch messages until a WM_QUIT message is received
  	{
    	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
    	{
      		if (msg.message == WM_QUIT) 
      		{
        		return (msg.wParam);
      		}
      		TranslateMessage(&msg);// xlates virt keycodes
      		DispatchMessage(&msg); // Dispatches msg to window
    	}
    	UpdateWorld(); // Update the world
  	}
  	return (msg.wParam); // Returns the value from PostQuitMessage
}

