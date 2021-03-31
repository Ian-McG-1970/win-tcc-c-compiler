#define WIN32_LEAN_AND_MEAN
#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define IDM_EXIT           106

#define INITIAL_DIB_WIDTH  	160		// initial dimensions of DIB
#define INITIAL_DIB_HEIGHT	128		//  into which we'll draw
#define MAX_POLY_VERTS      256
#define MAX_OBJECTS 				256
#define MAX_OBJECT_POINTS   4096
#define MAX_SCREEN_HEIGHT   2048
#define PI                  3.141592f
#define NUM_FRUSTUM_PLANES  4
#define EPSILON  0.0001f
#define FIELD_OF_VIEW 2.0f //4.0 //2.0

#define SCALE 1

#define LEFT 0
#define RIGHT 1

#define ALL_ON 0
#define ALL_OFF 1
#define INTERSECTING 2

#define CLIP_LEFT 1
#define CLIP_RIGHT 2
#define CLIP_BOTTOM 4
#define CLIP_TOP 8

#define GND 14 //10
#define SKY 18 //10
#define ROD 15
#define WHITE 10
#define BLACK 0

#define HR_AG 0.7071067812f
#define HR_SZ 8192.0f

#define DEBUG

#define FILL_TYPE 0

#define TURN_SPEED PI / 4096.0f //1024.0f //4096 //512.0 //2048.0 //1024.0
#define TILT_LIMIT TURN_SPEED*16.0f
#define TILT_FRICTION TURN_SPEED*0.4f
#define ACCEL 1.0f
#define FRICTION 0.97f //0.95f

#define MS 4096
#define MO MS/2
#define RD 8

typedef struct 
{
	float v[3];
} point_t;

typedef struct 
{
  signed short int x, y;
} point2D_t;

typedef struct  
{
  point_t center;
  float boundingsphere;
  float vdist;
  BYTE type;
} visibleobject;

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
  BOOL Filled; // is object filled or just lines
  BYTE Colour; // line colour
  float BoundingSphere; // object bounding sphere
  float VanishingPoint; // point where object isnt drawn
} OBJECT;

typedef struct 
{
  point_t Location;
  BYTE Type;    // object type
} OBJECTS;

BITMAPINFO *pbmiDIB;		// pointer to the BITMAPINFO
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
point_t vpn, vright, vup;
Point3D Vertex[MAX_POLY_VERTS];

OBJECTS groundobjects[] = {
{ 0,0,0, 1},

{ { MS*(0-8), 0, MS*(0-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(1-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(2-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(12-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(13-8)+MO}, 2},
{ { MS*(0-8), 0, MS*(14-8)+MO}, 2},

{ { MS*(1-8), 0, MS*(0-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(1-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(2-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(1-8), 0, MS*(8-8)+MO}, 2},

{ { MS*(2-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(2-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(2-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(2-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(2-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(2-8), 0, MS*(12-8)+MO}, 2},

{ { MS*(3-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(3-8), 0, MS*(11-8)+MO}, 2},

{ { MS*(4-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(12-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(13-8)+MO}, 2},
{ { MS*(4-8), 0, MS*(14-8)+MO}, 2},

{ { MS*(5-8), 0, MS*(0-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(1-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(2-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(12-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(13-8)+MO}, 2},
{ { MS*(5-8), 0, MS*(14-8)+MO}, 2},

{ { MS*(6-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(6-8), 0, MS*(12-8)+MO}, 2},

{ { MS*(7-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(7-8), 0, MS*(12-8)+MO}, 2},

{ { MS*(8-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(8-8), 0, MS*(12-8)+MO}, 2},
{ { MS*(8-8), 0, MS*(13-8)+MO}, 2},
{ { MS*(8-8), 0, MS*(14-8)+MO}, 2},

{ { MS*(9-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(9-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(9-8), 0, MS*(8-8)+MO}, 2},

{ { MS*(11-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(11-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(11-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(11-8), 0, MS*(12-8)+MO}, 2},

{ { MS*(13-8), 0, MS*(0-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(1-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(2-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(13-8), 0, MS*(11-8)+MO}, 2},

{ { MS*(15-8), 0, MS*(0-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(1-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(2-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(3-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(4-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(5-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(6-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(7-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(8-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(9-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(10-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(11-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(12-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(13-8)+MO}, 2},
{ { MS*(15-8), 0, MS*(14-8)+MO}, 2},

{ { MS*(0-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(2-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(3-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(7-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(11-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(12-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(13-8)+MO, 0, MS*(1-8)}, 3},
{ { MS*(14-8)+MO, 0, MS*(1-8)}, 3},

{ { MS*(0-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(2-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(3-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(7-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(11-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(12-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(13-8)+MO, 0, MS*(2-8)}, 3},
{ { MS*(14-8)+MO, 0, MS*(2-8)}, 3},

{ { MS*(3-8)+MO, 0, MS*(3-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(3-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(3-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(3-8)}, 3},

{ { MS*(0-8)+MO, 0, MS*(4-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(4-8)}, 3},
{ { MS*(2-8)+MO, 0, MS*(4-8)}, 3},
{ { MS*(3-8)+MO, 0, MS*(4-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(4-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(4-8)}, 3},

{ { MS*(7-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(11-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(12-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(13-8)+MO, 0, MS*(5-8)}, 3},
{ { MS*(14-8)+MO, 0, MS*(5-8)}, 3},

{ { MS*(1-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(2-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(7-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(11-8)+MO, 0, MS*(6-8)}, 3},
{ { MS*(12-8)+MO, 0, MS*(6-8)}, 3},

{ { MS*(3-8)+MO, 0, MS*(7-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(7-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(7-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(7-8)}, 3},

{ { MS*(3-8)+MO, 0, MS*(8-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(8-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(8-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(8-8)}, 3},

{ { MS*(0-8)+MO, 0, MS*(9-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(9-8)}, 3},
{ { MS*(7-8)+MO, 0, MS*(9-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(9-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(9-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(9-8)}, 3},

{ { MS*(0-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(2-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(3-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(4-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(5-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(6-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(7-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(8-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(9-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(10-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(11-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(12-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(13-8)+MO, 0, MS*(10-8)}, 3},
{ { MS*(14-8)+MO, 0, MS*(10-8)}, 3},

{ { MS*(0-8)+MO, 0, MS*(15-8)}, 3},
{ { MS*(1-8)+MO, 0, MS*(15-8)}, 3},

{ {-400,0,-400}, 6},
{ {+400,0,-400}, 6},
{ {-400,0,+400}, 6},
{ {+400,0,+400}, 6}, };

OBJECTS landobjects[] = { 
{ {-400,+50,-400}, 4},
{ {-200,+100,-400}, 5},
{ {+000,+50,-400}, 4},
{ {+200,+100,-400}, 5},
{ {+400,+50,-400}, 4},

{ {-400,+100,-200}, 5},
{ {-200,+50,-200}, 4},
{ {+000,+100,-200}, 5},
{ {+200,+50,-200}, 4},
{ {+400,+100,-200}, 5},

{ {-400,+50,-000}, 4},
{ {-200,+100,-000}, 5},
{ {+000,+50,-000}, 4},
{ {+200,+100,-000}, 5},
{ {+400,+50,-000}, 4},

{ {-400,+100,+200}, 5},
{ {-200,+50,+200}, 4},
{ {+000,+100,+200}, 5},
{ {+200,+50,+200}, 4},
{ {+400,+100,+200}, 5},

{ {-400,+50,+400}, 4},
{ {-200,+100,+400}, 5},
{ {+000,+50,+400}, 4},
{ {+200,+100,+400}, 5},
{ {+400,+50,+400}, 4}, };

point_t objectpoint002[] = { {-RD,+0,-MO-7}, 
                            {-RD,+0,+MO+7}, 
                            {+RD,+0,+MO+7}, 
                            {+RD,+0,-MO-7}, };
BYTE objectline002[] = { 0,1, 1,2, 2,3, 3,0 };
point_t objectpoint003[] = { {-MO-7,+0,-RD},
                            {-MO-7,+0,+RD}, 
                            {+MO+7,+0,+RD}, 
                            {+MO+7,+0,-RD}, };
BYTE objectline003[] = { 0,1, 1,2, 2,3, 3,0 };

point_t objectpoint000[] = { {HR_SZ*8,0,0}, {HR_AG*(HR_SZ*8),0,-HR_AG*(HR_SZ*8)}, {0,0,-HR_SZ*8}, {-HR_AG*(HR_SZ*8),0,-HR_AG*(HR_SZ*8)}, {-HR_SZ*8,0,0}, {-HR_AG*(HR_SZ*8),0,HR_AG*(HR_SZ*8)}, {0,0,HR_SZ*8},  {HR_AG*(HR_SZ*8),0,HR_AG*(HR_SZ*8)}, };
BYTE objectline000[] =  { 0,1, 1,2, 2,3, 3,4, 4,5, 5,6, 6,7, 7,0 };

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

point_t objectpoint104[] = {  {-50,-50,-50}, 
                            {-50,-50,+50}, 
                            {-50,+50,-50}, 
                            {-50,+50,+50}, 
                            {+50,-50,-50}, 
                            {+50,-50,+50}, 
                            {+50,+50,-50},  
                            {+50,+50,+50} };
BYTE objectface104[] = {  4, 0,1,3,2, 
                        4, 5,4,6,7 , 
                        4, 0,2,6,4,
                        4, 3,1,5,7, 
                        4, 1,0,4,5, 
                        4, 2,3,7,6, };
point_t objectpoint105[] = {  {-50,-100,-50}, 
                            {-50,-100,+50}, 
                            {-50,+100,-50}, 
                            {-50,+100,+50}, 
                            {+50,-100,-50}, 
                            {+50,-100,+50}, 
                            {+50,+100,-50}, 
                            {+50,+100,+50} };
BYTE objectface105[] = {  4, 0,1,3,2, 
                        4, 5,4,6,7 , 
                        4, 0,2,6,4,
                        4, 3,1,5,7, 
                        4, 1,0,4,5, 
                        4, 2,3,7,6, };
point_t objectpoint206[] = {  {-75,+0,-75}, 
                            {-75,+0,+75}, 
                            {+75,+0,-75}, 
                            {+75,+0,+75} };
BYTE objectline206[] =  { 0, 1, 1, 3, 3, 2, 2, 0, };
point_t objectpoint207[] = {  {-75,+0,-75}, 
                            {-75,+0,+75}, 
                            {+75,+0,-75}, 
                            {+75,+0,+75} };
BYTE objectline207[] =  { 0, 1, 1, 3, 3, 2, 2, 0, };

OBJECT object[] = {
                    { 8, &objectpoint000[0], 8, &objectline000[0], FALSE, 3, 65537.0f, 10000000.0f},
                    { 57, &objectpoint001[0], 33, &objectline001[0], FALSE, 5, 46341.0f, 10000000.0f},
                    { 4, &objectpoint002[0], 4, &objectline002[0], FALSE, 9, 2049.0f, 6145.0f},
                    { 4, &objectpoint003[0], 4, &objectline003[0], FALSE, 12, 2049.0f, 6145.0f},
                    { 8, &objectpoint104[0], 6, &objectface104[0], TRUE, 15, 87.0f, 4097.0f},
                    { 8, &objectpoint105[0], 6, &objectface105[0], TRUE, 18, 187.0f, 4097.0f},
                    { 4, &objectpoint206[0], 4, &objectline206[0], FALSE, 21, 107.0f, 4097.0f},
                    { 4, &objectpoint207[0], 4, &objectline207[0], FALSE, 24, 107.0f, 4097.0f},
};

int VisibleObjects=0;
visibleobject VisibleObject[MAX_OBJECTS];

FILE *file;
const BYTE ClipCodeList[NUM_FRUSTUM_PLANES]={CLIP_LEFT, CLIP_RIGHT, CLIP_BOTTOM, CLIP_TOP};
BYTE ClippedPolygon[NUM_FRUSTUM_PLANES][MAX_POLY_VERTS];

BYTE Points; // number of points in object - set initially then increases as clipped points are added

BOOL Fly=FALSE;

char string[256];

const void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

const void debug(const char* str) { sprintf(string,"%s\n",str); debugstring(); }

const void dbg(const char* str)
{
#ifdef DEBUG
 debug(str); 
#endif
}

/*const void bb()
{
	for (int o=0; o!=sizeof(object)/sizeof(object[0]); ++o)
	{
		float sphere=0.0f;
		for (int p=0; p!=object[o].Points; ++p)
		{
			const float bs=sqrt((object[o].Point[p].v[0]*object[o].Point[p].v[0]) + (object[o].Point[p].v[1]*object[o].Point[p].v[1]) + (object[o].Point[p].v[2]*object[o].Point[p].v[2]));
			if (sphere<bs) sphere=bs;
			sprintf(string,"bb %i %i %f %f %f %f %f\n",o,p,object[o].Point[p].v[0],object[o].Point[p].v[1],object[o].Point[p].v[2], bs, sphere); debugstring();
		}
	}
}*/

const void Set_DIB_FOV()
{
  if (pbmiDIB->bmiHeader.biHeight > 0)
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
            fclose(file);
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
            pbmiDIB->bmiHeader.biWidth = (LOWORD(lParam) +3) & ~3;
            pbmiDIB->bmiHeader.biHeight = HIWORD(lParam);;

            hDIBSection = CreateDIBSection(GetDC(hwnd), pbmiDIB, DIB_PAL_COLORS, &pDIBBase, NULL, 0);
            if (hDIBSection) 
            {
              DIBWidth = pbmiDIB->bmiHeader.biWidth;
              DIBHeight = pbmiDIB->bmiHeader.biHeight;
              DeleteObject(holdDIBSection);

              Set_DIB_FOV();
            }
          } 
        }
        break;

    case WM_DESTROY:  // message: window being destroyed
        free(pbmiDIB);
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
  RECT rctmp={0,0,DIBWidth/**SCALE*/,DIBHeight/**SCALE*/};
  AdjustWindowRect(&rctmp, WS_OVERLAPPEDWINDOW, 1);

  const HWND hwnd = CreateWindow("Clip", "Clip", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_DLGFRAME /*WS_OVERLAPPEDWINDOW*/, GetSystemMetrics(SM_CXSCREEN) - (rctmp.right - rctmp.left), GetSystemMetrics(SM_CYSCREEN) - (rctmp.bottom - rctmp.top), rctmp.right - rctmp.left, rctmp.bottom - rctmp.top, NULL, NULL, hInstance, NULL);
  if (!hwnd)
  {
    return FALSE;
  }

  pbmiDIB = malloc(sizeof(BITMAPINFO) - 4 + 256*sizeof(USHORT)); // Finally, set up the DIB section
  if (pbmiDIB == NULL)
  {
    return FALSE;
  }

  pbmiDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  pbmiDIB->bmiHeader.biWidth = DIBWidth;
  pbmiDIB->bmiHeader.biHeight = DIBHeight;
  pbmiDIB->bmiHeader.biPlanes = 1;
  pbmiDIB->bmiHeader.biBitCount = 8;
  pbmiDIB->bmiHeader.biCompression = BI_RGB;
  pbmiDIB->bmiHeader.biSizeImage = pbmiDIB->bmiHeader.biXPelsPerMeter = pbmiDIB->bmiHeader.biYPelsPerMeter = 0;
  pbmiDIB->bmiHeader.biClrUsed = pbmiDIB->bmiHeader.biClrImportant = 256;
  PUSHORT pusTemp = (PUSHORT) pbmiDIB->bmiColors;
  for (int i=0; i!=256; ++i) 
  {
    *pusTemp++ = i;
  }

  const HDC hdc = GetDC(hwnd);
  hDIBSection = CreateDIBSection(hdc, pbmiDIB, DIB_PAL_COLORS, &pDIBBase, NULL, 0);
  if (!hDIBSection) 
  {
    free(pbmiDIB);
    return FALSE;
  }

  Set_DIB_FOV();

  ShowWindow(hwnd, nCmdShow); // Show the window

  ReleaseDC(hwnd, hdc);
  hwndOutput = hwnd;

  file=fopen("log.txt","w");
  sprintf(string,"bb %f\n",123.45); debugstring();
 
//  bb();

  return TRUE;
}

point_t PointSet(const float in1, const float in2, const float in3)
{
	const point_t p = {in1, in2, in3};
	return p;
}
const point_t PointAdd(const point_t *in1, const point_t *in2)
{
  return PointSet(in1->v[0]+in2->v[0], in1->v[1]+in2->v[1], in1->v[2]+in2->v[2]);
}
const point_t PointMinus(const point_t *in1, const point_t *in2)
{
  return PointSet(in1->v[0]-in2->v[0], in1->v[1]-in2->v[1], in1->v[2]-in2->v[2]);
}
const float DotProduct(const point_t *vec1, const point_t *vec2)
{
	return (vec1->v[0] * vec2->v[0]) + (vec1->v[1] * vec2->v[1]) + (vec1->v[2] * vec2->v[2]);
}
const point_t CrossProduct(const point_t *in1, const point_t *in2)
{
  return PointSet((in1->v[1] * in2->v[2]) - (in1->v[2] * in2->v[1]), (in1->v[2] * in2->v[0]) - (in1->v[0] * in2->v[2]), (in1->v[0] * in2->v[1]) - (in1->v[1] * in2->v[0]));
}

//const void Plot(const int x, const int y, const BYTE colour) { *(pDIB + (DIBPitch * y) + x)=colour; }

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
	if (xslope <1.0f)
	{
		for (char *screen_line=(char *)pDIB + (sy*DIBPitch); scy!=ecy; scy+=ydir, sx+=xslope, screen_line+=pitch)
		{
      *(screen_line + (int)sx+1)=WHITE;
      *(screen_line + (int)sx)=colour;
		}
	}
	else
	{
		for (char *screen_line=(char *)pDIB + (sy*DIBPitch); scy!=ecy; scy+=ydir, sx+=xslope, screen_line+=pitch)
		{
      memset(screen_line+(int)sx+pitch,WHITE,xslope+1);
      memset(screen_line+(int)sx,colour,xslope+1);
		}
	}
}

const void Fill(const point2D_t *point, const BYTE points, const BOOL filled, const BYTE colour)
{
  if (filled==TRUE)
  {
  	int EdgeList[MAX_SCREEN_HEIGHT][2];
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

    char *screen_line=(char *)pDIB+(DIBPitch*ymin);
	  for (int ym=ymin; ym<ymax; ++ym, screen_line+=DIBPitch)
	  {
		  if (EdgeList[ym][0] <= EdgeList[ym][1])
		  {
		  	memset(screen_line + EdgeList[ym][0], BLACK,EdgeList[ym][1] - EdgeList[ym][0] + 1);
		  }
		  else
		  {
			  memset(screen_line + EdgeList[ym][1], BLACK,EdgeList[ym][0] - EdgeList[ym][1] + 1);
	 	  }
    }
  }
  for (int p=0, curr=points-1; p!=points; curr=p, ++p)
  {
    Line(point[curr].x, point[curr].y, point[p].x, point[p].y, colour);
  }

}

const point2D_t ProjectPoint(const point_t *pin)
{
  const float zrecip = (1.0f / (pin->v[2]+EPSILON)) * maxscale;
  point2D_t pout;
  pout.x = pin->v[0] * zrecip + xcenter;
  pout.y = DIBHeight - (pin->v[1] * zrecip + ycenter);

  if (pout.x <1) pout.x=1; else if (pout.x >DIBWidth-2) pout.x=DIBWidth-2;
  if (pout.y <1) pout.y=1; else if (pout.y >DIBHeight-2) pout.y=DIBHeight-2;

  return pout;
}

int ZCompare(const void *v1, const void *v2)
{
  const visibleobject *p1 = (visibleobject*)v1;
  const visibleobject *p2 = (visibleobject*)v2;
  if (p1->vdist < p2->vdist)
  {
    return 1;
  }
  else if (p1->vdist > p2->vdist)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

const int BoundingSphere(const point_t location, const float Bounding_Sphere)
{
  int result=ALL_ON;
  for (int f=0; f!=NUM_FRUSTUM_PLANES; ++f)
  {
    const float dot = DotProduct(&location, &frustumplanes[f].normal);
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
  return ( (Fly==TRUE) && (currentpos.v[1]!=1.0f) );
}

const void UpdateViewPos()
{
  const point_t xaxis = {1, 0, 0};
  const point_t yaxis = {0, 1, 0};
  const point_t zaxis = {0, 0, 1};

  const point_t motionvec={DotProduct(&vpn, &xaxis), Fly==TRUE ? DotProduct(&vpn, &yaxis) : 0.0f, DotProduct(&vpn, &zaxis)}; // Move in the view direction, across the x-y plane, as if walking. This approach moves slower when looking up or down at more of an angle

  const point_t temp_pos={ motionvec.v[0] * currentspeed,  motionvec.v[1] * currentspeed, motionvec.v[2] * currentspeed};
  currentpos=PointAdd(&temp_pos, &currentpos);
  currentdir=PointAdd(&currentdir, &currentturnspeed);

  const float rs = sin(currentdir.v[0]); // Set up the world-to-view rotation. Note: much of the work done in concatenating these matrices can be factored out, since it contributes nothing to the final result; multiply the three matrices together on paper to generate a minimum equation for each of the 9 final elements
  const float rc = cos(currentdir.v[0]);
  const float ps = sin(currentdir.v[1]);
  const float pc = cos(currentdir.v[1]);
  const float ys = sin(currentdir.v[2]);
  const float yc = cos(currentdir.v[2]);

  const float mtemp2[3][3] ={ { rc * yc, rs, rc * -ys }, { (pc * -rs * yc) + (ps * ys), pc * rc, (pc * -rs * -ys) + (ps * yc) }, { (-ps * -rs * yc) + (pc * ys), -ps * rc, (-ps * -rs * -ys) + (pc * yc) } };

  vright = PointSet( mtemp2[0][0], mtemp2[0][1], mtemp2[0][2] ); // Break out the rotation matrix into vright, vup, and vpn. We could work directly with the matrix; breaking it out into three vectors is just to make things clearer
  vup = PointSet( mtemp2[1][0],mtemp2[1][1],mtemp2[1][2] );
  vpn = PointSet( mtemp2[2][0], mtemp2[2][1], mtemp2[2][2] );
  
  currentspeed = fabs(currentspeed) > EPSILON ? currentspeed *= FRICTION : 0.0f;
  currentdir.v[0] = fabs(currentdir.v[0]) > EPSILON ? currentdir.v[0] *= FRICTION : 0.0f;
  currentturnspeed = PointSet(fabs(currentturnspeed.v[0]) > EPSILON ? currentturnspeed.v[0] *= FRICTION : 0.0f, fabs(currentturnspeed.v[1]) > EPSILON ? currentturnspeed.v[1] *= FRICTION : 0.0f, fabs(currentturnspeed.v[2]) > EPSILON ? currentturnspeed.v[2] *= FRICTION : 0.0f);
}

const point_t BackRotateVector(const point_t *pin) // Rotate a vector from viewspace to worldspace. // Rotate into the world orientation
{
	const point_t p = { (pin->v[0] * vright.v[0]) + (pin->v[1] * vup.v[0]) + (pin->v[2] * vpn.v[0]), (pin->v[0] * vright.v[1]) + (pin->v[1] * vup.v[1]) + (pin->v[2] * vpn.v[1]), (pin->v[0] * vright.v[2]) + (pin->v[1] * vup.v[2]) + (pin->v[2] * vpn.v[2]) };
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
  float angle = atan(2.0f / FIELD_OF_VIEW * maxscale / xscreenscale);
  float s = sin(angle);
  float c = cos(angle);

  point_t normal= { s, 0.0f, c}; // Left clip plane
  frustumplanes[0]=SetWorldspaceClipPlane(&normal);

  normal.v[0] = -s; // Right clip plane
  frustumplanes[1]=SetWorldspaceClipPlane(&normal);

  angle = atan(2.0f / FIELD_OF_VIEW * maxscale / yscreenscale);
  s = sin(angle);
  c = cos(angle);

	normal= PointSet(0.0f ,s, c); // Bottom clip plane
  frustumplanes[2]=SetWorldspaceClipPlane(&normal);

  normal.v[1] = -s; // Top clip plane
  frustumplanes[3]=SetWorldspaceClipPlane(&normal);
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
  	Vertex[p].Point=PointSet(point[p].v[0]+centre->v[0], point[p].v[1]+centre->v[1], point[p].v[2]+centre->v[2]);
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
		Vertex[p].Point=PointSet(point[p].v[0]+centre->v[0], point[p].v[1]+centre->v[1], point[p].v[2]+centre->v[2]);
	}
}

const void DrawObject(const BYTE *visibleface, const int faces, const BOOL filled, const BYTE Colour)
{
	point2D_t project[MAX_POLY_VERTS];
	BOOL frame[MAX_POLY_VERTS];
  memset(&frame[0],FALSE,sizeof(frame[0])*Points);

    point2D_t polygon[MAX_POLY_VERTS];

	  for (int pos=0, f=0, points; f!=faces; ++f)
	  {
		  points=visibleface[pos++];
		  for (int fp=0; fp!=points; ++fp)
		  {
			  const BYTE index=visibleface[pos++];
        if (frame[index]==FALSE)
        {
          frame[index]=TRUE;
          const point_t transformed_point=TransformPoint(&Vertex[index].Point);
          project[index]=ProjectPoint(&transformed_point);
        }
        polygon[fp].x=project[index].x;
        polygon[fp].y=project[index].y;
		  }
		  Fill(&polygon[0], points, filled, Colour);
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
  Vertex[Points].Point.v[0] = in->Point.v[0] + ((out->Point.v[0] - in->Point.v[0]) * (scale-EPSILON));
  Vertex[Points].Point.v[1] = in->Point.v[1] + ((out->Point.v[1] - in->Point.v[1]) * (scale-EPSILON));
  Vertex[Points].Point.v[2] = in->Point.v[2] + ((out->Point.v[2] - in->Point.v[2]) * (scale-EPSILON));
  for (int i=0; i!=Points; ++i)
  {
    if ((fabs(Vertex[i].Point.v[0]-Vertex[Points].Point.v[0])<EPSILON /*1.0f 2.0f 3.0f*/) && (fabs(Vertex[i].Point.v[1]-Vertex[Points].Point.v[1])<EPSILON) && (fabs(Vertex[i].Point.v[2]-Vertex[Points].Point.v[2])<EPSILON))
    {
      return i;
    }
  }
	Vertex[Points].ClipCode=ClassifyPoint(&Vertex[Points].Point);
  ++Points;
  return Points-1;
}

BYTE ClipToPlane(const BYTE *vertex_list_in, const int *vertex_count, const plane_t *pplane, BYTE *vertex_list_out, const BYTE clip_code)
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

const void DrawObjects()
{
	BYTE VisibleFace[MAX_OBJECT_POINTS]; // index into faces that need to be displayed

  for (int o=0; o!=VisibleObjects; ++o)
  {
    const BYTE type = VisibleObject[o].type;
	  if (VisibleObject[o].boundingsphere==INTERSECTING)
    {
      BYTE ClipCodesOr;
      if (MovePointsOffScreen(object[type].Point, &VisibleObject[o].center, object[type].Points, &ClipCodesOr)!=0) continue; // all off one side
      if (ClipCodesOr==0) VisibleObject[o].boundingsphere=ALL_ON; // nothing needs clipped
    }
    else
    {
    	MovePointsOnScreen(object[type].Point, &VisibleObject[o].center, object[type].Points); // all on screen
    }

    int Pos=0;
    int faces=0;
    Points=object[type].Points; // set initial number of points in object
    if (VisibleObject[o].boundingsphere==ALL_ON) // clip not needed
    {
			  for (int pos=0, f=0; f!=object[type].Faces; ++f)
			  {
				  const BYTE facepoints=object[type].Face[pos];
				  if (PolyFacesViewer(object[type].Face[pos+1], object[type].Face[pos+2], object[type].Face[pos+3])>=0.0f)
				  {
					  ++faces;					// add this face
					  memcpy(&VisibleFace[Pos], &object[type].Face[pos], (facepoints+1)*sizeof(VisibleFace[0])); // copy face indexes across
					  Pos+=(facepoints+1);
				  }
				  pos+=(facepoints+1);
			  }
    }
    else // clip needed
    {
			  for (int pos=0, f=0, facepoints, newverts; f!=object[type].Faces; ++f, pos+=(facepoints+1))
			  {
				  facepoints=newverts=object[type].Face[pos];
				  if (PolyFacesViewer(object[type].Face[pos+1], object[type].Face[pos+2], object[type].Face[pos+3])>=0.0f)
				  {
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
    }
	  DrawObject(&VisibleFace[0], faces, object[type].Filled, object[type].Colour);
  }
  VisibleObjects=0;
}

const BYTE ObjectVisible(const BYTE type, const point_t centre, point_t dist)
{
  const float vanishingpoint=object[type].VanishingPoint;
  if ( (fabs(dist.v[0])>vanishingpoint) || (fabs(dist.v[1])>vanishingpoint) || (fabs(dist.v[2])>vanishingpoint) ) { return ALL_OFF; }

  return BoundingSphere(centre, object[type].BoundingSphere);
}

const void DrawLineObject(const BYTE *visibleface, const int faces, const BYTE colour)
{
  point2D_t project[MAX_POLY_VERTS];
  BOOL frame[MAX_POLY_VERTS];
  memset(&frame[0],FALSE,sizeof(frame[0])*Points);

  for (int p=0, l=0; l!=faces; ++l, p+=2)
  {
    for (int i=0; i!=2; ++i)
    {
      const BYTE idx=visibleface[p+i];
      if (frame[idx]==FALSE)
      {
        frame[idx]=TRUE;
        const point_t transformed_point=TransformPoint(&Vertex[idx].Point);
        project[idx]=ProjectPoint(&transformed_point);
      }
    }
    Line(project[visibleface[p]].x, project[visibleface[p]].y, project[visibleface[p+1]].x, project[visibleface[p+1]].y, colour);
  }
}

const void DrawLineObjects(const BYTE Type, const point_t Centre, BYTE Boundingsphere, const BYTE Colour)
{
  if (Boundingsphere==INTERSECTING)
  {
   BYTE ClipCodesOr;
    if (MovePointsOffScreen(object[Type].Point, &Centre, object[Type].Points, &ClipCodesOr)!=0) return; // all off one side
    if (ClipCodesOr==0) Boundingsphere=ALL_ON; // nothing needs clipped
  }
  else
  {
    MovePointsOnScreen(object[Type].Point, &Centre, object[Type].Points); // all on screen
  }

  int faces=0;
  BYTE VisibleFace[MAX_OBJECT_POINTS]; // index into faces that need to be displayed
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

const void DrawVisibleLineObjects(const OBJECTS *objectlist, const int numobjects)
{
  for (int o=0; o!=numobjects; ++o)
  {
    const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    const BYTE boundingsphere=ObjectVisible(objectlist[o].Type, objectlist[o].Location, dist);
    if (boundingsphere==ALL_OFF) continue;
    DrawLineObjects(objectlist[o].Type, objectlist[o].Location, boundingsphere, object[objectlist[o].Type].Colour);
  }
}

const void DrawVisiblePolygonObjects(const OBJECTS *objectlist, const int numobjects)
{
  for (int o=0; o!=numobjects; ++o)
  {
    const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    VisibleObject[VisibleObjects].boundingsphere=ObjectVisible(objectlist[o].Type, objectlist[o].Location, dist);
    if (VisibleObject[VisibleObjects].boundingsphere==ALL_OFF) continue;

    VisibleObject[VisibleObjects].type = objectlist[o].Type;
    VisibleObject[VisibleObjects].vdist = (dist.v[0] * dist.v[0]) + (dist.v[1] * dist.v[1]) + (dist.v[2] * dist.v[2]);
    VisibleObject[VisibleObjects].center = PointSet( objectlist[o].Location.v[0], objectlist[o].Location.v[1], objectlist[o].Location.v[2] );      
    ++VisibleObjects;
  }
//  sprintf(string,"dvpo %i %i\n",numobjects,VisibleObjects); debugstring();
}

const void Collision()
{
 if (currentpos.v[1] <1.0f)
 {
  currentspeed=currentturnspeed.v[0]=0.0f;
  currentpos.v[1]=1.0f;
 }
}

const void Input()
{
  if (GetAsyncKeyState(VK_UP)) currentturnspeed.v[1]+=TURN_SPEED;    
  if (GetAsyncKeyState(VK_DOWN)) currentturnspeed.v[1]-=TURN_SPEED;

  if (GetAsyncKeyState(VK_RIGHT))
  {
    currentturnspeed.v[2]+=TURN_SPEED;
    if (Flying()==TRUE) if (currentturnspeed.v[0] > -TILT_LIMIT) currentturnspeed.v[0]-=TILT_FRICTION;
  }
  if (GetAsyncKeyState(VK_LEFT))
  {
    currentturnspeed.v[2]-=TURN_SPEED;
    if (Flying()==TRUE) if (currentturnspeed.v[0] < TILT_LIMIT) currentturnspeed.v[0]+=TILT_FRICTION;
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

  memset(pDIBBase, 0, DIBWidth*DIBHeight);    // clear frame
  OBJECTS horizon; horizon.Type=0; horizon.Location=PointSet(currentpos.v[0],currentpos.v[1]-100,currentpos.v[2]); // horizon
  DrawVisibleLineObjects(&horizon, 1); // horizon 
  DrawVisibleLineObjects(&groundobjects[0], sizeof(groundobjects) / sizeof(groundobjects[0])); // ground objects
  DrawObjects(); // horizon and ground object

  DrawVisiblePolygonObjects(&landobjects[0], sizeof(landobjects) / sizeof(landobjects[0])); // pass in all other objects
  qsort(&VisibleObject[0], VisibleObjects, sizeof(VisibleObject[0]), ZCompare);
  DrawObjects();

  const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);
  ReleaseDC(hwndOutput, hdcScreen);
  SelectObject(hdcDIBSection, holdbitmap);
  DeleteDC(hdcDIBSection);

//sprintf(string,"%5.1f %5.1f %5.1f\n",currentpos.v[0],currentpos.v[1],currentpos.v[2]);
//const HFONT hf=CreateFont(1, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
//TextOutA(hdcScreen,1,1,string, strlen(string));

  Sleep(9); // 20ms = 1/50s
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


