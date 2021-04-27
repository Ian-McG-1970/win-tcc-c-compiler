#define WIN32_LEAN_AND_MEAN
#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define IDM_EXIT           106

#define INITIAL_DIB_WIDTH   256   // initial dimensions of DIB
#define INITIAL_DIB_HEIGHT  192   //  into which we'll draw
#define ISOTOP INITIAL_DIB_WIDTH /2
#define ISOLEFT INITIAL_DIB_HEIGHT /2

typedef struct
{
  unsigned short int X,Y;
} POS;

typedef struct
{
  POS ST;
  POS SZ;
  BYTE COLOUR;
} OBJECT;

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

char *pDIB, *pDIBBase; // pointers to DIB section we'll draw into
HBITMAP hDIBSection;   // handle of DIB section
HWND hwndOutput;
int DIBWidth=INITIAL_DIB_WIDTH, DIBHeight=INITIAL_DIB_HEIGHT, DIBPitch;

FILE *file;

char string[256];

typedef struct
{
  short int x;
  short int y;
} xy;

typedef struct
{
  short int x;
  short int y;
  short int z;
} xyz;

typedef struct
{
  xyz centre;
  xyz min;
  xyz max;
  xy overlap_x;
  xy overlap_y;
  xy overlap_h;
  short int shape;
  xy pos;
  BOOL behind;
} obj;

const xyz shape_dimension[]={ {4,4,4}, {8,8,8}, {4,4,8}, {8,4,4}, {4,8,4}, {8,4,8}, {4,8,8}, {8,2,8}, {2,8,8}, {2,2,2}, {1,1,1} };
const xy shape_offset[]={ {8,8}, {16,16}, {12,8}, {10,12}, {10,12}, {14,12}, {14,12}, {13,10}, {13,10} , {4,4}, {0,0} };
//const xy shape_size[]={ {16,16}, {32,32}, {24,16}, {20,24}, {20,24}, {28,24}, {28,24}, {26,20}, {26,20} };
const BYTE shape_colour[][3]={ {1,2,3}, {5,3,1}, {4,1,2}, {2,3,4}, {3,5,1}, {1,3,5}, {5,4,2}, {4,3,2}, {2,5,3}, {3,4,1}, {5,2,3} };

#define numobjects sizeof(shape_dimension)/sizeof(shape_dimension[0])

int objects;
int drawn;
obj object[256];
int object_pos[256];

int px, py, pz=0;

#define LT <
#define GT >
#define LE =<
#define GE >=

const void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

const void debug(const char* str) { sprintf(string,"%s\n",str); debugstring(); }

const xy XY(const int x, const int y)
{
  xy rc;
  rc.x=x;
  rc.y=y;
  return rc;
}

const xyz XYZ(const int x, const int y, const int z)
{
  xyz rc;
  rc.x=x;
  rc.y=y;
  rc.z=z;
  return rc;
}

const xy IsometricPoint(const xyz pos)
{
  return XY(ISOLEFT + ( (pos.x + pos.y) >>1) -pos.z, ISOTOP + pos.x - pos.y);
}

const BOOL Collision(const obj o1, const obj o2)
{
  if (o1.min.x < (o2.max.x) && (o1.max.x) > o2.min.x && o1.min.y < (o2.max.y) && (o1.max.y) > o2.min.y && o1.min.z < (o2.max.z) && (o1.max.z) > o2.min.z)
  {
    return TRUE;
  }
  return FALSE;
}

const BOOL Movement(const xyz centre, const int pos)
{
  obj item2;
  item2.centre=XYZ(object[pos].centre.x+centre.x, object[pos].centre.y+centre.y, object[pos].centre.z+centre.z);
  item2.min=XYZ(item2.centre.x-shape_dimension[object[pos].shape].x, item2.centre.y-shape_dimension[object[pos].shape].y, item2.centre.z-shape_dimension[object[pos].shape].z);
  item2.max=XYZ(item2.centre.x+shape_dimension[object[pos].shape].x, item2.centre.y+shape_dimension[object[pos].shape].y, item2.centre.z+shape_dimension[object[pos].shape].z);

  for (int o=0; o!=objects; ++o)
  {
    if (o!=pos)
    {
      if (Collision(item2, object[o])==TRUE)
      {
        return FALSE;
      }
    }
  }
  memcpy(&object[pos].min,&item2.min,sizeof(object[pos].min));
  memcpy(&object[pos].max,&item2.max,sizeof(object[pos].max));
  object[pos].centre=item2.centre;
  xy loc=IsometricPoint(centre);
  object[pos].pos=XY(loc.x-shape_offset[object[pos].shape].x,loc.y-shape_offset[object[pos].shape].y);
  object[pos].overlap_x=XY(object[pos].min.x-object[pos].max.z,object[pos].max.x-object[pos].min.z);
  object[pos].overlap_y=XY(object[pos].min.y-object[pos].max.z,object[pos].max.y-object[pos].min.z);
  const xy fs=IsometricPoint(XYZ(object[pos].min.x,object[pos].max.y,object[pos].min.z));
  const xy fe=IsometricPoint(XYZ(object[pos].max.x,object[pos].min.y,object[pos].min.z));
  object[pos].overlap_h=XY(fs.y,fe.y);

  return TRUE;
}

const void AddObject(const int shape)
{
  object[objects].centre=XYZ(0,0,0);
  object[objects].shape=shape;
  ++objects;
}

const void Setup()
{
  srand(GetTickCount());
  objects=0;
  for (int i=0; i!=7;)
  {
    AddObject(rand()%numobjects);
    do
    {
    }
    while (Movement(XYZ(rand()&31, rand()&31, 0+shape_dimension[object[objects-1].shape].z), objects-1)!=TRUE);
    ++i;
  }
}

const void Draw2D(const int pos, const BYTE colour)
{
  char *screen_line=(char *)pDIB+(DIBPitch*(object[pos].min.x+16))+object[pos].min.y+16;

  for (int x=object[pos].min.x, y=object[pos].max.y-object[pos].min.y; x!=object[pos].max.x; ++x, screen_line+=DIBPitch)
  {
    memset(screen_line, colour, y + 1);
  }
}

const void Fill(const xy *point, const BYTE points, const BYTE colour)
{
  int EdgeList[INITIAL_DIB_HEIGHT][2];

  int xmax=point[0].x;
  int xmin=point[0].x;

  for (int p=0, curr=points-1; p!=points; curr=p, ++p)
  {
    float ys, ye;
    int edge, xs, xe;

    if (point[curr].x>point[p].x)
    {
      edge=0;
      ys=point[p].y;
      ye=point[curr].y;
      xs=point[p].x;
      xe=point[curr].x;
    }
    else
    {
      edge=1;
      ys=point[curr].y;
      ye=point[p].y;
      xs=point[curr].x;
      xe=point[p].x;
    }

    if (xe>xmax) xmax=xe;
    if (xs<xmin) xmin=xs;

    const int xdiff=xe-xs;
    const float ydiff=ye-ys;
    const float yslope=ydiff/xdiff;

    for (; xs<xe; ++xs, ys+=yslope)
    {
      EdgeList[xs][edge]=ys;
    }
  }

  for (char *screen_line=(char *)pDIB+(DIBPitch*xmin); xmin<xmax; ++xmin, screen_line+=DIBPitch)
  {
    memset(screen_line + EdgeList[xmin][0], colour,EdgeList[xmin][1] - EdgeList[xmin][0] + 1);
  }
}

const void DrawFast(const obj o)
{
  const xy tm=IsometricPoint(XYZ(o.min.x,o.min.y,o.max.z));
  const xy bl=IsometricPoint(XYZ(o.min.x,o.max.y,o.min.z)); 
  const xy tl=IsometricPoint(XYZ(o.min.x,o.max.y,o.max.z));
  const xy br=IsometricPoint(XYZ(o.max.x,o.min.y,o.min.z)); 
  const xy tr=IsometricPoint(XYZ(o.max.x,o.min.y,o.max.z)); 
  const xy bm=IsometricPoint(XYZ(o.max.x,o.max.y,o.min.z)); 
  const xy mm=IsometricPoint(XYZ(o.max.x,o.max.y,o.max.z)); 

  const xy topfill[4]={tm,tr,mm,tl};
  const BYTE col1=shape_colour[o.shape][0];
  Fill(&topfill[0],4,col1);

  const xy rightfill[4]={tr,br,bm,mm};
  const BYTE col2=shape_colour[o.shape][1];
  Fill(&rightfill[0],4,col2);

  const xy leftfill[4]={tl,mm,bm,bl};
  const BYTE col3=shape_colour[o.shape][2];
  Fill(&leftfill[0],4,col3);
}

const BOOL IsBehind2(const xy *first, const xy *second)
{
  return ! ( (first->y LT second->x) || (second->y LT first->x) );
}

const BOOL Overlap(const obj *first, const obj *second)
{
  return ( (IsBehind2(&first->overlap_x, &second->overlap_x)==TRUE) && (IsBehind2(&first->overlap_y, &second->overlap_y)==TRUE) && (IsBehind2(&first->overlap_h, &second->overlap_h)==TRUE) );
}

const BOOL IsBehind(const obj *o1, const obj *o2)
{
  return ( (o1->min.z GE o2->max.z) || (o1->min.x GE o2->max.x) || (o1->min.y GE o2->max.y) );
}

const void SortObjectList(const int objects)
{
  for (int o=0; o!=objects; ++o)
  {
    object_pos[o]=o;
  }

  for (int objects_left=objects, drawn=0; objects_left!=0; )
  {

    for (int o=0; o!=objects_left; ++o)
    {
      object[object_pos[o]].behind=TRUE;
    }

    for (int f=0; f!=objects_left-1; ++f)
    {
      for (int b=f+1; b!=objects_left; ++b)
      {
        if (Overlap(&object[object_pos[f]], &object[object_pos[b]])==TRUE)
        {
          if (IsBehind(&object[object_pos[f]], &object[object_pos[b]])==TRUE)
          {
            object[object_pos[f]].behind=FALSE;
          }
          else
          {
            object[object_pos[b]].behind=FALSE;
          }
        }
      }
    }

    for (int o=0; o!=objects_left; ++o)
    {
      if (object[object_pos[o]].behind==TRUE)
      {
        DrawFast((object[object_pos[o]]));
        Draw2D(object_pos[o],5);

        ++drawn;
        --objects_left;
        object_pos[o]=object_pos[objects_left];
        --o;
      }
    }
  }
}

const void Set_DIB()
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
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM uParam, LPARAM lParam)
{
    switch (message) 
    {
    case WM_COMMAND:  // message: command from application menu
        switch (LOWORD(uParam)) 
        {
        case IDM_EXIT:
            DestroyWindow(hwnd);
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
              Set_DIB();
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

  const HWND hwnd = CreateWindow("Clip", "Clip", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_DLGFRAME, GetSystemMetrics(SM_CXSCREEN) - (rctmp.right - rctmp.left), GetSystemMetrics(SM_CYSCREEN) - (rctmp.bottom - rctmp.top), rctmp.right - rctmp.left, rctmp.bottom - rctmp.top, NULL, NULL, hInstance, NULL);
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

  Set_DIB();

  ShowWindow(hwnd, nCmdShow); // Show the window

  ReleaseDC(hwnd, hdc);
  hwndOutput = hwnd;

  file=fopen("log.txt","w");
  sprintf(string,"bb %f\n",123.45); debugstring();
 
  return TRUE;
}

#define KEYA 0x41
#define KEYS 0x53
#define KEYD 0x44
#define KEYW 0x57
#define KEYQ 0x51
#define KEYE 0x45

const void Input()
{
  px=py=pz=0;
  if (GetAsyncKeyState(KEYW)) --py;    
  if (GetAsyncKeyState(KEYS)) ++py;
  if (GetAsyncKeyState(KEYA)) --px;
  if (GetAsyncKeyState(KEYD)) ++px;
  if (GetAsyncKeyState(KEYQ)) --pz;
  if (GetAsyncKeyState(KEYE)) ++pz;
}

/*const xy CalcOffset(const int o)
{
  const xyz s=shape_dimension[o]; 
  sprintf(string,"s %i %i %i %i\n",o,s.x,s.y,s.z); debugstring();

  const xyz sa[]={ { -s.x, -s.y, -s.z }, { -s.x, -s.y, +s.z }, { -s.x, +s.y, -s.z }, { -s.x, +s.y, +s.z }, { +s.x, -s.y, -s.z }, { +s.x, -s.y, +s.z }, { +s.x, +s.y, -s.z }, { +s.x, +s.y, +s.z }, };
  xy mn={ INITIAL_DIB_WIDTH, INITIAL_DIB_HEIGHT};
  xy mx={ 0, 0};
  for (int i=0; i!=8; ++i)
  {
    const xy pos=IsometricPoint(sa[i]);
    mn.x=min(pos.x,mn.x);
    mx.x=max(pos.x,mx.x);
    mn.y=min(pos.y,mn.y);
    mx.y=max(pos.y,mx.y);
  }
  const xy sz={ (mx.x-mn.x)/2, (mx.y-mn.y)/2 };
  sprintf(string,"sz %i %i %i\n",o,sz.x,sz.y); debugstring();

  return sz;
}*/

const void UpdateWorld() // Render the current state of the world to the screen.
{
/*  const xyz tstxyz[]= { {-63,-63,-31}, {-63,-63,+31}, {-63,+63,-31}, {-63,+63,+31}, {+63,-63,-31}, {+63,-63,+31}, {+63,+63,-31}, {+63,+63,+31} };
  for (int i=0; i!=8; ++i)
    {
      const xy tstxy=IsometricPoint(tstxyz[i]);
//      sprintf(string,"ip %i %i %i %i %i %i\n",i, tstxyz[i].x,tstxyz[i].y,tstxyz[i].z, tstxy.x, tstxy.y); debugstring();
    }*/

//    for (int o=0; o!=numobjects; ++o) { const xy sz=CalcOffset(o);}

  Input();
  Movement(XYZ(px,py,pz),objects-1);
  memset(pDIBBase, 0, DIBWidth*DIBHeight);    // clear frame
  SortObjectList(objects);

  const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);
  SelectObject(hdcDIBSection, holdbitmap);
  DeleteDC(hdcDIBSection);
  DeleteObject(holdbitmap);
  ReleaseDC(hwndOutput,hdcScreen);

  Sleep(19); // 20ms = 1/50s
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
  Setup();

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


