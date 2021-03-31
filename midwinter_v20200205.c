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
#define MAX_OBJECT_POINTS   65535
#define MAX_SCREEN_HEIGHT   2048
#define PI                  3.141592f
#define NUM_FRUSTUM_PLANES  4
#define EPSILON  0.0001f
#define FIELD_OF_VIEW 2.0f //3.0 //4.0 //2.0

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
#define ROD 12 //20 //20

#define HR_AG 0.7071067812f
#define HR_SZ 8192.0f

#define DEBUG

#define FILL_TYPE 0

#define TURN_SPEED PI / 4096.0f //1024.0f //4096 //512.0 //2048.0 //1024.0
#define TILT_LIMIT TURN_SPEED*16.0f
#define TILT_FRICTION TURN_SPEED*0.4f
#define ACCEL 1.0f
#define FRICTION 0.97f //0.95f

#define PS 2048.0f
#define MS PS * 8.0f

typedef struct 
{
	float x;
  float y;
  float z;
} point_t;

typedef struct 
{
  signed short int x, y;
  float z;
} POINT_P;

typedef struct 
{
  signed short int x, y;
} point2D_t;

typedef struct  
{
  point_t center;
  point_t angle;
  float boundingsphere;
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
  BYTE Points; // points in face
  BYTE Colour; // face colour
  float NearestZ; // nearest Z 
  int PointPos; // point list pos 
} FACE;

FACE FaceList[MAX_OBJECT_POINTS];
point2D_t FacePointList[MAX_OBJECT_POINTS];
int FaceListPos=0;
int FacePointListPos=0;

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
  point_t Angle;
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
point_t vpn, vright, vup;
Point3D Vertex[MAX_POLY_VERTS];

point_t ovpn, ovright, ovup;

OBJECTS groundobjects[] = {

{ {-MS*3,0,-MS*3}, {0,0,0}, 13},
{ {-MS*2,0,-MS*3}, {0,0,0}, 13},
{ {-MS*1,0,-MS*3}, {0,0,0}, 13},
{ {+MS*0,0,-MS*3}, {0,0,0}, 13},
{ {+MS*1,0,-MS*3}, {0,0,0}, 13},
{ {+MS*2,0,-MS*3}, {0,0,0}, 13},
{ {+MS*3,0,-MS*3}, {0,0,0}, 13},

{ {-MS*3,0,-MS*2}, {0,0,0}, 13},
{ {-MS*2,0,-MS*2}, {0,0,0}, 13},
{ {-MS*1,0,-MS*2}, {0,0,0}, 13},
{ {+MS*0,0,-MS*2}, {0,0,0}, 13},
{ {+MS*1,0,-MS*2}, {0,0,0}, 13},
{ {+MS*2,0,-MS*2}, {0,0,0}, 13},
{ {+MS*3,0,-MS*2}, {0,0,0}, 13},

{ {-MS*3,0,-MS*1}, {0,0,0}, 13},
{ {-MS*2,0,-MS*1}, {0,0,0}, 13},
{ {-MS*1,0,-MS*1}, {0,0,0}, 13},
{ {+MS*0,0,-MS*1}, {0,0,0}, 13},
{ {+MS*1,0,-MS*1}, {0,0,0}, 13},
{ {+MS*2,0,-MS*1}, {0,0,0}, 13},
{ {+MS*3,0,-MS*1}, {0,0,0}, 13},

{ {-MS*3,0,+MS*0}, {0,0,0}, 13},
{ {-MS*2,0,+MS*0}, {0,0,0}, 13},
{ {-MS*1,0,+MS*0}, {0,0,0}, 13},
{ {+MS*0,0,+MS*0}, {0,0,0}, 13},
{ {+MS*1,0,+MS*0}, {0,0,0}, 13},
{ {+MS*2,0,+MS*0}, {0,0,0}, 13},
{ {+MS*3,0,+MS*0}, {0,0,0}, 13},

{ {-MS*3,0,+MS*1}, {0,0,0}, 13},
{ {-MS*2,0,+MS*1}, {0,0,0}, 13},
{ {-MS*1,0,+MS*1}, {0,0,0}, 13},
{ {+MS*0,0,+MS*1}, {0,0,0}, 13},
{ {+MS*1,0,+MS*1}, {0,0,0}, 13},
{ {+MS*2,0,+MS*1}, {0,0,0}, 13},
{ {+MS*3,0,+MS*1}, {0,0,0}, 13},

{ {-MS*3,0,+MS*2}, {0,0,0}, 13},
{ {-MS*2,0,+MS*2}, {0,0,0}, 13},
{ {-MS*1,0,+MS*2}, {0,0,0}, 13},
{ {+MS*0,0,+MS*2}, {0,0,0}, 13},
{ {+MS*1,0,+MS*2}, {0,0,0}, 13},
{ {+MS*2,0,+MS*2}, {0,0,0}, 13},
{ {+MS*3,0,+MS*2}, {0,0,0}, 13},

{ {-MS*3,0,+MS*3}, {0,0,0}, 13},
{ {-MS*2,0,+MS*3}, {0,0,0}, 13},
{ {-MS*1,0,+MS*3}, {0,0,0}, 13},
{ {+MS*0,0,+MS*3}, {0,0,0}, 13},
{ {+MS*1,0,+MS*3}, {0,0,0}, 13},
{ {+MS*2,0,+MS*3}, {0,0,0}, 13},
{ {+MS*3,0,+MS*3}, {0,0,0}, 13},

{ {+400,0,+400}, {0,0,0}, 6}, };

OBJECTS landobjects[] = { 
{ {-400,+50,-400}, {0,0,1}, 4},
{ {-200,+100,-400}, {0,0,2}, 5},
{ {+000,+50,-400}, {0,0,3}, 4},
{ {+200,+100,-400}, {0,0,4}, 5},
{ {+400,+50,-400}, {0,0,5}, 4},

{ {-400,+100,-200}, {0,0,5}, 5},
{ {-200,+50,-200}, {0,0,4}, 4},
{ {+000,+100,-200}, {0,0,3}, 5},
{ {+200,+50,-200}, {0,0,2}, 4},
{ {+400,+100,-200}, {0,0,1}, 5},

{ {-400,+50,-000}, {0,0,0}, 4},
{ {-200,+100,-000}, {0,0,1}, 5},
{ {+000,+50,-000}, {0,0,2}, 4},
{ {+200,+100,-000}, {0,0,3}, 5},
{ {+400,+50,-000}, {0,0,4}, 4},

{ {-400,+100,+200}, {0,0,4}, 5},
{ {-200,+50,+200}, {0,0,3}, 4},
{ {+000,+100,+200}, {0,0,2}, 5},
{ {+200,+50,+200}, {0,0,1}, 4},
{ {+400,+100,+200}, {0,0,0}, 5},

{ {-400,+50,+400}, {0,0,1}, 4},
{ {-200,+100,+400}, {0,0,2}, 5},
{ {+000,+50,+400}, {0,0,3}, 4},
{ {+200,+100,+400}, {0,0,4}, 5},
{ {+400,+50,+400}, {1,3,5}, 4},
 };

point_t objectpoint104[] = {  {-50,-50,-50}, 
                            {-50,-50,+50}, 
                            {-50,+50,-50}, 
                            {-50,+50,+50}, 
                            {+50,-50,-50}, 
                            {+50,-50,+50}, 
                            {+50,+50,-50},  
                            {+50,+50,+50} };
BYTE objectface104[] = {  4, 103, 0,1,3,2, 
                        4, 105, 5,4,6,7 , 
                        4, 107, 0,2,6,4,
                        4,  109, 3,1,5,7, 
                        4, 111, 1,0,4,5, 
                        4, 113, 2,3,7,6, };
point_t objectpoint105[] = {  {-50,-100,-50}, 
                            {-50,-100,+50}, 
                            {-50,+100,-50}, 
                            {-50,+100,+50}, 
                            {+50,-100,-50}, 
                            {+50,-100,+50}, 
                            {+50,+100,-50}, 
                            {+50,+100,+50} };
BYTE objectface105[] = {  4, 217, 0,1,3,2, 
                        4, 218, 5,4,6,7 , 
                        4, 219, 0,2,6,4,
                        4, 220, 3,1,5,7, 
                        4, 221, 1,0,4,5, 
                        4, 223, 2,3,7,6, };
point_t objectpoint206[] = {  {-75,+0,-75}, 
                            {-75,+0,+75}, 
                            {+75,+0,-75}, 
                            {+75,+0,+75} };
BYTE objectface206[] =  { 4, 124, 0,1,3,2 };

point_t objectpoint308[] = {  {0.0*100, -0.71*100, -1*100}, 
                            {0.0*100, -0.71*100, +1*100}, 
                            {-1*100, +0.71*100, 0.0*100}, 
                            {+1*100, +0.71*100, 0.0*100} };
BYTE objectface308[] =  { 3, 160, 0,1,2,
  3, 170, 1,0,3,
  3, 180, 0,2,3,
  3, 190, 1,3,2, };

point_t objectpoint309[] = {  {0.0*10, -0.71*10, -1*10}, 
                            {0.0*10, -0.71*10, +1*10}, 
                            {-1*10, +0.71*10, 0.0*10}, 
                            {+1*10, +0.71*10, 0.0*10} };
BYTE objectface309[] =  { 3, 160, 0,1,2,
  3, 170, 1,0,3,
  3, 180, 0,2,3,
  3, 190, 1,3,2, };

point_t objectpoint310[] = {  {0.0*10, -0.171*10, -1*10}, 
                            {0.0*10, -0.171*10, +1*10}, 
                            {-1*10, +0.71*10, 0.0*10}, 
                            {+1*10, +0.71*10, 0.0*10} };
BYTE objectface310[] =  { 3, 160, 0,1,2,
  3, 170, 1,0,3,
  3, 180, 0,2,3,
  3, 190, 1,3,2, };

point_t objectpoint211[] = { {-40,+0,-160}, 
                            {-40,+0,+160}, 
                            {+40,+0,+160}, 
                            {+40,+0,-160}, };
BYTE objectface211[] = {  4, 100, 0,1,2,3, };

point_t objectpoint106[] = {  {-512,-512,-512}, 
                            {-512,-512,+512}, 
                            {-512,+512,-512}, 
                            {-512,+512,+512}, 
                            {+512,-512,-512}, 
                            {+512,-512,+512}, 
                            {+512,+512,-512},  
                            {+512,+512,+512} };
BYTE objectface106[] = {  4, 203, 0,1,3,2, 
                        4, 205, 5,4,6,7 , 
                        4, 207, 0,2,6,4,
                        4, 209, 3,1,5,7, 
                        4, 211, 1,0,4,5, 
                        4, 213, 2,3,7,6, };

point_t objectpoint107[] = {  
  {-4*PS,100,-4*PS}, 
  {-3*PS,210,-4*PS}, 
  {-2*PS,120,-4*PS}, 
  {-1*PS,030,-4*PS}, 
  {+0*PS,140,-4*PS}, 
  {+1*PS,250,-4*PS}, 
  {+2*PS,160,-4*PS}, 
  {+3*PS,070,-4*PS}, 
  {+4*PS,180,-4*PS},

  {-4*PS,710,-3*PS}, 
  {-3*PS,250,-3*PS}, 
  {-2*PS,140,-3*PS}, 
  {-1*PS,030,-3*PS}, 
  {+0*PS,220,-3*PS}, 
  {+1*PS,110,-3*PS}, 
  {+2*PS,200,-3*PS}, 
  {+3*PS,010,-3*PS}, 
  {+4*PS,120,-3*PS},

  {-4*PS,230,-2*PS}, 
  {-3*PS,140,-2*PS}, 
  {-2*PS,050,-2*PS}, 
  {-1*PS,160,-2*PS}, 
  {+0*PS,200,-2*PS}, 
  {+1*PS,170,-2*PS}, 
  {+2*PS,050,-2*PS}, 
  {+3*PS,140,-2*PS}, 
  {+4*PS,230,-2*PS},

  {-4*PS,120,-1*PS}, 
  {-3*PS,010,-1*PS}, 
  {-2*PS,120,-1*PS}, 
  {-1*PS,230,-1*PS}, 
  {+0*PS,140,-1*PS}, 
  {+1*PS,050,-1*PS}, 
  {+2*PS,160,-1*PS}, 
  {+3*PS,250,-1*PS}, 
  {+4*PS,140,-1*PS},

  {-4*PS,030,+0*PS}, 
  {-3*PS,120,+0*PS}, 
  {-2*PS,210,+0*PS}, 
  {-1*PS,100,+0*PS}, 
  {+0*PS,010,+0*PS}, 
  {+1*PS,120,+0*PS}, 
  {+2*PS,230,+0*PS}, 
  {+3*PS,140,+0*PS}, 
  {+4*PS,050,+0*PS},

  {-4*PS,160,+1*PS}, 
  {-3*PS,200,+1*PS}, 
  {-2*PS,170,+1*PS}, 
  {-1*PS,80,+1*PS}, 
  {+0*PS,190,+1*PS}, 
  {+1*PS,200,+1*PS}, 
  {+2*PS,100,+1*PS}, 
  {+3*PS,010,+1*PS}, 
  {+4*PS,120,+1*PS},

  {-4*PS,230,+2*PS}, 
  {-3*PS,140,+2*PS}, 
  {-2*PS,050,+2*PS}, 
  {-1*PS,160,+2*PS}, 
  {+0*PS,210,+2*PS}, 
  {+1*PS,120,+2*PS}, 
  {+2*PS,030,+2*PS}, 
  {+3*PS,140,+2*PS}, 
  {+4*PS,250,+2*PS},

  {-4*PS,160,+3*PS}, 
  {-3*PS,070,+3*PS}, 
  {-2*PS,180,+3*PS}, 
  {-1*PS,210,+3*PS}, 
  {+0*PS,100,+3*PS}, 
  {+1*PS,90,+3*PS}, 
  {+2*PS,180,+3*PS}, 
  {+3*PS,240,+3*PS}, 
  {+4*PS,150,+3*PS},

  {-4*PS,040,+4*PS}, 
  {-3*PS,130,+4*PS}, 
  {-2*PS,220,+4*PS}, 
  {-1*PS,110,+4*PS}, 
  {+0*PS,000,+4*PS}, 
  {+1*PS,110,+4*PS}, 
  {+2*PS,220,+4*PS}, 
  {+3*PS,130,+4*PS}, 
  {+4*PS,040,+4*PS}, };

BYTE objectface107[] = {
3, 1, 1,0,10,
3, 3, 0,9,10,
3, 5, 10,2,1,
3, 7, 10,11,2,
3, 9, 3,2,12,
3, 11, 2,11,12,
3, 13, 12,4,3,
3, 15, 12,13,4,
3, 17, 5,4,14,
3, 19, 4,13,14,
3, 21, 14,6,5,
3, 23, 14,15,6,
3, 25, 7,6,16,
3, 27, 6,15,16,
3, 29, 16,8,7,
3, 31, 16,17,8,

3, 33, 10,9,18,
3, 35, 10,18,19,
3, 37, 11,10,20,
3, 39, 10,19,20,
3, 41, 12,11,20,
3, 43, 12,20,21,
3, 45, 13,12,22,
3, 47, 12,21,22,
3, 49, 14,13,22,
3, 51, 14,22,23,
3, 53, 15,14,24,
3, 55, 14,23,24,
3, 55, 16,15,24,
3, 57, 16,24,25,
3, 59, 17,16,26,
3, 61, 16,25,26,

3, 65, 1+18,0+18,10+18,
3, 67, 0+18,9+18,10+18,
3, 69, 10+18,2+18,1+18,
3, 71, 10+18,11+18,2+18,
3, 73, 3+18,2+18,12+18,
3, 75, 2+18,11+18,12+18,
3, 77, 12+18,4+18,3+18,
3, 79, 12+18,13+18,4+18,
3, 81, 5+18,4+18,14+18,
3, 83, 4+18,13+18,14+18,
3, 85, 14+18,6+18,5+18,
3, 87, 14+18,15+18,6+18,
3, 89, 7+18,6+18,16+18,
3, 91, 6+18,15+18,16+18,
3, 93, 16+18,8+18,7+18,
3, 95, 16+18,17+18,8+18,

3, 97, 10+18,9+18,18+18,
3, 99, 10+18,18+18,19+18,
3, 101, 11+18,10+18,20+18,
3, 103, 10+18,19+18,20+18,
3, 105, 12+18,11+18,20+18,
3, 107, 12+18,20+18,21+18,
3, 109, 13+18,12+18,22+18,
3, 111, 12+18,21+18,22+18,
3, 113, 14+18,13+18,22+18,
3, 115, 14+18,22+18,23+18,
3, 117, 15+18,14+18,24+18,
3, 119, 14+18,23+18,24+18,
3, 121, 16+18,15+18,24+18,
3, 123, 16+18,24+18,25+18,
3, 125, 17+18,16+18,26+18,
3, 127, 16+18,25+18,26+18,

3, 129, 1+36,0+36,10+36,
3, 131, 0+36,9+36,10+36,
3, 133, 10+36,2+36,1+36,
3, 135, 10+36,11+36,2+36,
3, 137, 3+36,2+36,12+36,
3, 139, 2+36,11+36,12+36,
3, 141, 12+36,4+36,3+36,
3, 143, 12+36,13+36,4+36,
3, 145, 5+36,4+36,14+36,
3, 147, 4+36,13+36,14+36,
3, 149, 14+36,6+36,5+36,
3, 151, 14+36,15+36,6+36,
3, 153, 7+36,6+36,16+36,
3, 155, 6+36,15+36,16+36,
3, 157, 16+36,8+36,7+36,
3, 159, 16+36,17+36,8+36,

3, 161, 10+36,9+36,18+36,
3, 163, 10+36,18+36,19+36,
3, 165, 11+36,10+36,20+36,
3, 167, 10+36,19+36,20+36,
3, 169, 12+36,11+36,20+36,
3, 171, 12+36,20+36,21+36,
3, 173, 13+36,12+36,22+36,
3, 175, 12+36,21+36,22+36,
3, 177, 14+36,13+36,22+36,
3, 179, 14+36,22+36,23+36,
3, 181, 15+36,14+36,24+36,
3, 183, 14+36,23+36,24+36,
3, 185, 16+36,15+36,24+36,
3, 187, 16+36,24+36,25+36,
3, 189, 17+36,16+36,26+36,
3, 191, 16+36,25+36,26+36,

3, 199, 1+54,0+54,10+54,
3, 201, 0+54,9+54,10+54,
3, 203, 10+54,2+54,1+54,
3, 205, 10+54,11+54,2+54,
3, 207, 3+54,2+54,12+54,
3, 209, 2+54,11+54,12+54,
3, 211, 12+54,4+54,3+54,
3, 213, 12+54,13+54,4+54,
3, 215, 5+54,4+54,14+54,
3, 217, 4+54,13+54,14+54,
3, 219, 14+54,6+54,5+54,
3, 221, 14+54,15+54,6+54,
3, 223, 7+54,6+54,16+54,
3, 225, 6+54,15+54,16+54,
3, 227, 16+54,8+54,7+54,
3, 229, 16+54,17+54,8+54,

3, 231, 10+54,9+54,18+54,
3, 233, 10+54,18+54,19+54,
3, 235, 11+54,10+54,20+54,
3, 237, 10+54,19+54,20+54,
3, 239, 12+54,11+54,20+54,
3, 241, 12+54,20+54,21+54,
3, 243, 13+54,12+54,22+54,
3, 245, 12+54,21+54,22+54,
3, 247, 14+54,13+54,22+54,
3, 249, 14+54,22+54,23+54,
3, 251, 15+54,14+54,24+54,
3, 253, 14+54,23+54,24+54,
3, 255, 16+54,15+54,24+54,
3, 2, 16+54,24+54,25+54,
3, 4, 17+54,16+54,26+54,
3, 6, 16+54,25+54,26+54, };

OBJECT object[] = { { 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 8, &objectpoint104[0], 6, &objectface104[0], 87.0f, 4097.0f},
                    { 8, &objectpoint105[0], 6, &objectface105[0], 187.0f, 4097.0f},
                    { 4, &objectpoint206[0], 1, &objectface206[0], 107.0f, 4097.0f},
                    { 0, NULL, 0, NULL, 0.0f, 0.0f},
                    { 4, &objectpoint308[0], 4, &objectface308[0], 200.0f, 32767.0f},
                    { 4, &objectpoint309[0], 4, &objectface309[0], 20.0f, 32767.0f},
                    { 4, &objectpoint310[0], 4, &objectface310[0], 50.0f, 32767.0f},
                    { 4, &objectpoint211[0], 1, &objectface211[0], 300.0f, 32767.0f},
                    { 8, &objectpoint106[0], 6, &objectface106[0], 900.0f, 65535.0f},
                    { 81, &objectpoint107[0], 128, &objectface107[0], 5794.0f, 65535.0f}, };

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

const void bb()
{
	for (int o=0; o!=sizeof(object)/sizeof(object[0]); ++o)
	{
		float sphere=0.0f;
		for (int p=0; p!=object[o].Points; ++p)
		{
			const float bs=sqrt((object[o].Point[p].x*object[o].Point[p].x) + (object[o].Point[p].y*object[o].Point[p].y) + (object[o].Point[p].z*object[o].Point[p].z));
			if (sphere<bs) sphere=bs;
			sprintf(string,"bb %i %i %f %f %f %f %f\n",o,p,object[o].Point[p].x,object[o].Point[p].y,object[o].Point[p].z, bs, sphere); debugstring();
		}
	}
}

const void Set_DIB_FOV()
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

  BMInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  BMInfo.bmiHeader.biWidth = DIBWidth;
  BMInfo.bmiHeader.biHeight = DIBHeight;
  BMInfo.bmiHeader.biPlanes = 1;
  BMInfo.bmiHeader.biBitCount = 8;
  BMInfo.bmiHeader.biCompression = BI_RGB;
  BMInfo.bmiHeader.biSizeImage = BMInfo.bmiHeader.biXPelsPerMeter = BMInfo.bmiHeader.biYPelsPerMeter = 0;
  BMInfo.bmiHeader.biClrUsed = BMInfo.bmiHeader.biClrImportant = 256;

  RGBQUAD palette[256];
  for (int i=0; i!=256; ++i)
  {
    palette[i].rgbRed = rand(); palette[i].rgbGreen = rand(); palette[i].rgbBlue = rand(); palette[i].rgbReserved = 0;
  }
  memcpy(&BMInfo.bmiColors[0], &palette[0], sizeof(palette[0])*256);

  PalInfo.palVersion = 0x300;
  PalInfo.palNumEntries = 256;
  for (int i=0; i!=256; ++i)
  {
    PalInfo.palPalEntry[i].peRed = palette[i].rgbRed;
    PalInfo.palPalEntry[i].peGreen = palette[i].rgbGreen;
    PalInfo.palPalEntry[i].peBlue = palette[i].rgbBlue;
    PalInfo.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
  }

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
  sprintf(string,"bb %f\n",123.45); debugstring();
 
  bb();

  return TRUE;
}

point_t PointSet(const float in1, const float in2, const float in3)
{
	const point_t p = {in1, in2, in3};
	return p;
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

int EdgeList[MAX_SCREEN_HEIGHT][2];

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

const POINT_P ProjectPoint(const point_t *pin)
{
  const float zrecip = (1.0f / (pin->z+EPSILON)) * maxscale;
  POINT_P pout;
  pout.x = pin->x * zrecip + xcenter;
  pout.y = DIBHeight - (pin->y * zrecip + ycenter);
  pout.z=pin->z;

  if (pout.x <0) pout.x=0; else if (pout.x >DIBWidth-1) pout.x=DIBWidth-1;
  if (pout.y <0) pout.y=0; else if (pout.y >DIBHeight-1) pout.y=DIBHeight-1;

//sprintf(string,"pp %f\n",pin->z); debugstring();
  return pout;
}

int NearestZCompare(const void *v1, const void *v2)
{
  const FACE *p1 = (FACE*)v1;
  const FACE *p2 = (FACE*)v2;
  if (p1->NearestZ < p2->NearestZ)
  {
    return +1;
  }
  else if (p1->NearestZ > p2->NearestZ)
  {
    return -1;
  }
  else
  {
    return 0;
  }
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

  const float mtemp2[3][3] ={ { rc * yc, rs, rc * -ys }, { (pc * -rs * yc) + (ps * ys), pc * rc, (pc * -rs * -ys) + (ps * yc) }, { (-ps * -rs * yc) + (pc * ys), -ps * rc, (-ps * -rs * -ys) + (pc * yc) } };

  *right = PointSet( mtemp2[0][0], mtemp2[0][1], mtemp2[0][2] ); // Break out the rotation matrix into vright, vup, and vpn. We could work directly with the matrix; breaking it out into three vectors is just to make things clearer
  *up = PointSet( mtemp2[1][0],mtemp2[1][1],mtemp2[1][2] );
  *in = PointSet( mtemp2[2][0], mtemp2[2][1], mtemp2[2][2] );
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
  float angle = atan(2.0f / FIELD_OF_VIEW * maxscale / xscreenscale);
  float s = sin(angle);
  float c = cos(angle);

  point_t normal= { s, 0.0f, c}; // Left clip plane
  frustumplanes[0]=SetWorldspaceClipPlane(&normal);

  normal.x = -s; // Right clip plane
  frustumplanes[1]=SetWorldspaceClipPlane(&normal);

  angle = atan(2.0f / FIELD_OF_VIEW * maxscale / yscreenscale);
  s = sin(angle);
  c = cos(angle);

	normal= PointSet(0.0f ,s, c); // Bottom clip plane
  frustumplanes[2]=SetWorldspaceClipPlane(&normal);

  normal.y = -s; // Top clip plane
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

const BYTE MovePointsOffScreen(const point_t *point, const point_t *centre, const point_t *angle, const BYTE points, BYTE *ClipCodesOr)
{
	*ClipCodesOr=0;
	BYTE clipcodesand=255;

  if ( (angle->x!=0) || (angle->y!=0) || (angle->z!=0) ) 
  {
    Rotate(angle, &ovright, &ovup, &ovpn);
    for (int p=0; p!=points; ++p)
    {
      Vertex[p].Point=PointSet(DotProduct(&point[p], &ovright) +centre->x, DotProduct(&point[p], &ovup) +centre->y, DotProduct(&point[p], &ovpn) +centre->z);
      Vertex[p].ClipCode=ClassifyPoint(&Vertex[p].Point);
      clipcodesand &= Vertex[p].ClipCode;
      *ClipCodesOr |= Vertex[p].ClipCode;
    }
  }
  else
  {
  	for (int p=0; p!=points; ++p)
    {
    	Vertex[p].Point=PointSet(point[p].x+centre->x, point[p].y+centre->y, point[p].z+centre->z);
  	  Vertex[p].ClipCode=ClassifyPoint(&Vertex[p].Point);
  	  clipcodesand &= Vertex[p].ClipCode;
  	  *ClipCodesOr |= Vertex[p].ClipCode;
  	}
  }
	return clipcodesand;
}

const void MovePointsOnScreen(const point_t *point, const point_t *centre, const point_t *angle, const BYTE points)
{
  if ( (angle->x!=0) || (angle->y!=0) || (angle->z!=0) ) 
  {
    Rotate(angle, &ovright, &ovup, &ovpn);
    for (int p=0; p!=points; ++p)
    {
      Vertex[p].Point=PointSet(DotProduct(&point[p], &ovright) +centre->x, DotProduct(&point[p], &ovup) +centre->y, DotProduct(&point[p], &ovpn) +centre->z);
    }
  }
  else
  {
  	for (int p=0; p!=points; ++p)
  	{
  		Vertex[p].Point=PointSet(point[p].x+centre->x, point[p].y+centre->y, point[p].z+centre->z);
  	}
  }
}

const void PolygonDraw()
{
  for (int p=0; p!=FaceListPos; ++p)
  {
//sprintf(string,"pd %i %i %f\n",FaceListPos,p,FaceList[p].NearestZ); debugstring();

    Fill(&FacePointList[FaceList[p].PointPos], FaceList[p].Points, FaceList[p].Colour);
  }
  FaceListPos=FacePointListPos=0;
}

const void PolygonAdd(const point2D_t *point, const BYTE points, const BYTE colour, const float nearestz)
{
  FaceList[FaceListPos].Points=points;
  FaceList[FaceListPos].Colour=colour;
  FaceList[FaceListPos].NearestZ=nearestz;
  FaceList[FaceListPos].PointPos=FacePointListPos;
  memcpy(&FacePointList[FacePointListPos], &point[0], sizeof(FacePointList[0])*points);
  FacePointListPos+=points;
  ++FaceListPos;
}

POINT_P project[MAX_POLY_VERTS];
BOOL frame[MAX_POLY_VERTS];
  point2D_t polygon[MAX_POLY_VERTS];

const void DrawObject(const BYTE *visibleface, const int faces)
{
  memset(&frame[0],FALSE,sizeof(frame[0])*Points);

  for (int pos=0, f=0, points; f!=faces; ++f)
	{
    points=visibleface[pos++];
	  const BYTE colour=visibleface[pos++];
    float nearestZ=+900000;
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

      nearestZ=min(nearestZ, project[index].z);
		}
    PolygonAdd(&polygon[0], points, colour, nearestZ);
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
      if (MovePointsOffScreen(object[type].Point, &VisibleObject[o].center, &VisibleObject[o].angle, object[type].Points, &ClipCodesOr)!=0) continue; // all off one side
      if (ClipCodesOr==0) VisibleObject[o].boundingsphere=ALL_ON; // nothing needs clipped
    }
    else
    {
    	MovePointsOnScreen(object[type].Point, &VisibleObject[o].center, &VisibleObject[o].angle, object[type].Points); // all on screen
    }

    int Pos=0;
    int faces=0;
    Points=object[type].Points; // set initial number of points in object
    if (VisibleObject[o].boundingsphere==ALL_ON) // clip not needed
    {
      for (int pos=0, f=0; f!=object[type].Faces; ++f)
			{
			  const BYTE facepoints=object[type].Face[pos];
			  if (PolyFacesViewer(object[type].Face[pos+2], object[type].Face[pos+3], object[type].Face[pos+4])>=0.0f)
			  {
				  ++faces;					// add this face
				  memcpy(&VisibleFace[Pos], &object[type].Face[pos], (facepoints+2)*sizeof(VisibleFace[0])); // copy face indexes across
				  Pos+=(facepoints+2);
			  }
			  pos+=(facepoints+2);
			 }
    }
    else // clip needed
    {
      for (int pos=0, f=0, facepoints, newverts; f!=object[type].Faces; ++f, pos+=(facepoints+2))
			{
			  facepoints=newverts=object[type].Face[pos];
			  if (PolyFacesViewer(object[type].Face[pos+2], object[type].Face[pos+3], object[type].Face[pos+4])>=0.0f)
			  {
          BYTE* objectfacepos=&object[type].Face[pos]+2; // store position of face
          BYTE PolygonClipCodesOr;
          if ((ClassifyFace(objectfacepos, facepoints, &PolygonClipCodesOr)!=0)) continue; // if all off one side
          if (PolygonClipCodesOr!=0) // if face needs clipped
          {
            objectfacepos=ClipToFrustum(objectfacepos, &newverts, PolygonClipCodesOr); // clip face
            if (objectfacepos==NULL) continue; // if face fully clipped
          }
          ++faces;          // add this face
          VisibleFace[Pos++]=newverts; // copy new verts
          VisibleFace[Pos++]=object[type].Face[pos+1];  // copy colour
          memcpy(&VisibleFace[Pos], objectfacepos, newverts*sizeof(VisibleFace[0])); // copy face indexes
          Pos+=newverts; // inc pos by verts
				}
      }
    }
	  DrawObject(&VisibleFace[0], faces);
  }
  VisibleObjects=0;
}

const BYTE ObjectVisible(const BYTE type, const point_t *centre, const point_t *dist)
{
  const float vanishingpoint=object[type].VanishingPoint;
  if ( (fabs(dist->x)>vanishingpoint) || (fabs(dist->y)>vanishingpoint) || (fabs(dist->z)>vanishingpoint) ) { return ALL_OFF; }

  return BoundingSphere(centre, object[type].BoundingSphere);
}

const void DrawVisiblePolygonObjects(const OBJECTS *objectlist, const int numobjects)
{
  for (int o=0; o!=numobjects; ++o)
  {
//sprintf(string,"dvo %i %i %i\n",o,numobjects,objectlist[o].Type); debugstring();

    const point_t dist=PointMinus(&objectlist[o].Location, &currentpos);
    VisibleObject[VisibleObjects].boundingsphere=ObjectVisible(objectlist[o].Type, &objectlist[o].Location, &dist);
// if ( (bounding sphere = intersecting) and (rotation = false) ) bounding sphere = bounding box test;
    if (VisibleObject[VisibleObjects].boundingsphere==ALL_OFF) continue;

    VisibleObject[VisibleObjects].type = objectlist[o].Type;
    VisibleObject[VisibleObjects].center = PointSet( objectlist[o].Location.x, objectlist[o].Location.y, objectlist[o].Location.z );      
    VisibleObject[VisibleObjects].angle = PointSet( objectlist[o].Angle.x, objectlist[o].Angle.y, objectlist[o].Angle.z );      
    ++VisibleObjects;
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

point_t r={0,0,0};

const void UpdateWorld() // Render the current state of the world to the screen.
{
  r=PointSet(r.x+0.01,r.y+0.02,r.z+0.03);

  Input();
  Collision();
  UpdateViewPos();
  SetUpFrustum();

  memset(pDIBBase, SKY, DIBWidth*DIBHeight);    // clear frame

  DrawVisiblePolygonObjects(&groundobjects[0], sizeof(groundobjects) / sizeof(groundobjects[0])); // ground objects
  DrawVisiblePolygonObjects(&landobjects[0], sizeof(landobjects) / sizeof(landobjects[0])); // pass in all other objects

  OBJECTS obj1 = { {1000,1000,1000}, {r.x,r.y,r.z}, 8}; // bullet
  DrawVisiblePolygonObjects(&obj1, 1); // rotating bullet

  OBJECTS obj2 = { {+1000,+5,+1500}, {0,0,0}, 10}; // ship
  DrawVisiblePolygonObjects(&obj2, 1);

  OBJECTS obj4[] = { 
  { {-5096,+0,-5096}, {r.x,r.y,r.z}, 12}, 
  { {+5096,+0,-5096}, {r.x,r.y,r.z}, 12}, 
  { {-5096,+0,+5096}, {r.x,r.y,r.z}, 12}, 
  { {+5096,+0,+5096}, {r.x,r.y,r.z}, 12} }; // boxes
  DrawVisiblePolygonObjects(&obj4[0], 4);


  DrawObjects();

//for (int p=0; p!=FaceListPos; ++p) { sprintf(string,"pd %i %i %f\n",FaceListPos,p,FaceList[p].NearestZ); debugstring(); }


  qsort(&FaceList[0], FaceListPos, sizeof(FaceList[0]), NearestZCompare);

//for (int p=0; p!=FaceListPos; ++p) { sprintf(string,"pd %i %i %f\n",FaceListPos,p,FaceList[p].NearestZ); debugstring(); }

  PolygonDraw();

  const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);
  SelectObject(hdcDIBSection, holdbitmap);
  DeleteDC(hdcDIBSection);
  DeleteObject(holdbitmap);
  ReleaseDC(hwndOutput,hdcScreen);

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
