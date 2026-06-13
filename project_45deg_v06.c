#define WIN32_LEAN_AND_MEAN
#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define IDM_EXIT           106

#define INITIAL_DIB_WIDTH   512   // initial dimensions of DIB
#define INITIAL_DIB_HEIGHT  512   //  into which we'll draw

#define LT <
#define LE <=
#define GT >
#define GE >=
#define EQ ==
#define NE !=

#define CC_BEHIND 128
#define CC_OFF_LEFT 1
#define CC_OFF_RIGHT 2
#define CC_OFF_TOP 4
#define CC_OFF_BOTTOM 8
#define CC_ALL_ON 0
#define CC_ALL_OFF 255

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
  long x;
  long y;
  long z;
} xyz;

typedef struct
{
  long x;
  long y;
} xy;

const int scale=1024;

const int width=INITIAL_DIB_WIDTH/2;
const int height=INITIAL_DIB_HEIGHT/2;

xyz start={20, 0, 30}; //{0x0fc20,0x0fa7d};
xyz end={100, 0, 150}; //{0x0f20,0x07683};
xyz point={100,100,1000};
float rotation=0.0f;

const void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

const void debug(const char* str) { sprintf(string,"%s\n",str); debugstring(); }

const xyz XYZ(const long x, const long y, const long z)
{
  xyz rc;
  rc.x=x;
  rc.y=y;
  rc.z=z;
  return rc;
}

const xy XY(const long x, const long y)
{
  xy rc;
  rc.x=x;
  rc.y=y;
  return rc;
}

const void Plot(const xyz pos, const unsigned char colour)
{
	unsigned char *screen_line=(char *)pDIB + (pos.z*DIBPitch);
	*(screen_line + (pos.x))=colour;
}

const void DrawLine(const xy start, const xy end, const char colour)
{
	int	dx, dy, incx, incy;

	if (end.x >= start.x)
	{
		dx = end.x - start.x;
		incx = 1;
	}
	else
	{
		dx = start.x - end.x;
		incx = -1;
	}

	if (end.y >= start.y)
	{
		dy = end.y - start.y;
		incy = 1;
	}
	else
	{
		dy = start.y - end.y;
		incy = -1;
	}

	int x = start.x;
	int y = start.y;

	if (dx >= dy)
	{
		dy <<= 1;
		int balance = dy - dx;
		dx <<= 1;

		while (x != end.x)
		{
			Plot(XYZ(x,0,y),colour);
			if (balance >= 0)
			{
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		} 
	}
	else
	{
		dx <<= 1;
		int balance = dx - dy;
		dy <<= 1;

		while (y != end.y)
		{
			Plot(XYZ(x,0,y),colour);
			if (balance >= 0)
			{
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
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

const void Input()
{
  if (GetAsyncKeyState('A')) point.x-=200;
  if (GetAsyncKeyState('D')) point.x+=200;
  if (GetAsyncKeyState('W')) point.y-=200;
  if (GetAsyncKeyState('S')) point.y+=200;
  if (GetAsyncKeyState('Q')) point.z+=200;
  if (GetAsyncKeyState('Z')) point.z-=200;

  if (GetAsyncKeyState('C')) rotation+=0.01f;
  if (GetAsyncKeyState('X')) rotation-=0.01f;
}

const xyz Clip3DLeft(const xyz Start, const xyz End)
{
	const float D_x = End.x - Start.x;
	const float D_y = End.y - Start.y;
	const float D_z = End.z - Start.z;
	const float D_Diff = -D_z - D_x;
	const int S_Diff = Start.x + Start.z;
	const float Scale = S_Diff / D_Diff;
// left ³ (Ax + Az) / (-dz - dx)

	xyz rc;
	rc.x = Start.x + (D_x *Scale);
	rc.y = Start.y + (D_y *Scale);
	rc.z = Start.z + (D_z *Scale);
	return rc;
}

const xyz Clip3DRight(const xyz Start, const xyz End)
{
	const float D_x = End.x - Start.x;
	const float D_y = End.y - Start.y;
	const float D_z = End.z - Start.z;
	const float D_Diff = -D_x + D_z;
	const int S_Diff = Start.x - Start.z;
	const float Scale = S_Diff / D_Diff;
// right ³ (Ax - Az) / (-dx + dz)

	xyz rc;
	rc.x = Start.x + (D_x *Scale);
	rc.y = Start.y + (D_y *Scale);
	rc.z = Start.z + (D_z *Scale);
	return rc;
}

const xyz Clip3DTop(const xyz Start, const xyz End)
{
	const float D_x = End.x - Start.x;
	const float D_y = End.y - Start.y;
	const float D_z = End.z - Start.z;
	const float D_Diff = -D_z - D_y;
	const int S_Diff = Start.y + Start.z;
	const float Scale = S_Diff / D_Diff;
// top ³ (Ay - Az) / (-dy + dz)

	xyz rc;
	rc.x = Start.x + (D_x *Scale);
	rc.y = Start.y + (D_y *Scale);
	rc.z = Start.z + (D_z *Scale);
	return rc;
}

const xyz Clip3DBottom(const xyz Start, const xyz End)
{
	const float D_x = End.x - Start.x;
	const float D_y = End.y - Start.y;
	const float D_z = End.z - Start.z;
	const float D_Diff = -D_y + D_z;
	const int S_Diff = Start.y - Start.z;
	const float Scale = S_Diff / D_Diff;
// bottom ³ (Ay + Az) / (-dz - dy)*/

	xyz rc;
	rc.x = Start.x + (D_x *Scale);
	rc.y = Start.y + (D_y *Scale);
	rc.z = Start.z + (D_z *Scale);
	return rc;
}

const unsigned char ClipCode(const xyz Point)
{
	if (Point.z LT 0)
	{
		return CC_BEHIND;
	}
	const long minus_point_z = -Point.z;
	unsigned char clip_code=CC_ALL_ON;
	if (Point.x GT Point.z)
	{
		clip_code=CC_OFF_RIGHT;
	}
	else if (Point.x LT minus_point_z)
	{
		clip_code=CC_OFF_LEFT;
	}
	if (Point.y GT Point.z)
	{
		return clip_code | CC_OFF_BOTTOM;
	}
	else if (Point.y LT minus_point_z)
	{
		return clip_code | CC_OFF_TOP;
	}
	return clip_code;
}

const xy Project(const xyz Point)
{
	unsigned long x=abs(Point.x);
	unsigned long y=abs(Point.y);
	unsigned long z=Point.z;
	while (z < 0x80000000)
	{
		z+=z;//z<<1;//+=z;
		x+=x;//x<<1;//+=x;
		y+=y;//y<<1;//+=y;
	}
	z=z>>9;
	x=x>>1;
	y=y>>1;
	x=x/z;
	y=y/z;
	if (Point.x<0)
	{
		x=256-x;
	}
	else
	{
		x=256+x;
	}	
	if (Point.y<0)
	{
		y=256-y;
	}
	else
	{
		y=256+y;
	}	
	return XY(x,y);
}

const xyz Rotate(const xyz Point)
{
	const float s=sin(rotation);
	const float c=cos(rotation);

	const int xs=Point.x*s;
	const int zs=Point.z*s;

	const int xc=Point.x*c;
	const int zc=Point.z*c;

	Point.x=xc+zs;
	Point.z=zc-xs;
	
	return XYZ(Point.x,Point.y,Point.z);
}

xyz ObjectRotatePoint[256];
xy ObjectProjectPoint[256];
unsigned char ObjectClipCode[256];

const xyz ObjectPoints0000[]={ {-100,-100,-100}, {100,-100,-100}, {-100,100,-100}, {100,100,-100}, {-100,-100,100}, {100,-100,100}, {-100,100,100}, {100,100,100} }; 
const unsigned char ObjectLines0000[]={ 0,1, 1,3, 3,2, 2,0, 4,5, 5,7, 7,6, 6,4, 0,4, 1,5, 2,6, 3,7}; 

const void ClipLine(const xyz start_point, const xyz end_point, unsigned char start_cc, unsigned char end_cc, xy start_project, xy end_project)
{
	unsigned char cc = start_cc | end_cc;

//	sprintf(string,"cl %3i %3i %3i\n",cc,start_cc,end_cc); debugstring();
	sprintf(string,"cl %5i %5i %5i %5i %5i %5i\n",start_point.x, start_point.y, start_point.z, end_point.x, end_point.y, end_point.z); debugstring();

	if ( (cc & CC_OFF_RIGHT) NE CC_ALL_ON)
	{
		if ( (start_cc & CC_OFF_RIGHT) NE CC_ALL_ON)
		{
			start_point=Clip3DRight(start_point,end_point);
	++start_point.z;
			start_cc=ClipCode(start_point);
 			if (start_cc EQ CC_ALL_ON)
			{
				start_project=Project(start_point);
			}
		}
		else
		{
			end_point=Clip3DRight(end_point,start_point);
	++end_point.z;
			end_cc=ClipCode(end_point);
 			if (end_cc EQ CC_ALL_ON)
			{
				end_project=Project(end_point);
			}
		}
		if ( (start_cc & end_cc) NE CC_ALL_ON)
		{
			return;
		}
		if ( (start_cc | end_cc) EQ CC_ALL_ON)
		{ 
			DrawLine(start_project, end_project, 25);
			return;
		}
	}

	cc = start_cc | end_cc;
	if ( (cc & CC_OFF_LEFT) NE CC_ALL_ON)
	{
		if ( (start_cc & CC_OFF_LEFT) NE CC_ALL_ON)
		{
			start_point=Clip3DLeft(end_point,start_point);
	++start_point.z;
			start_cc=ClipCode(start_point);
 			if (start_cc EQ CC_ALL_ON)
			{
				start_project=Project(start_point);
			}
		}
		else
		{
			end_point=Clip3DLeft(start_point,end_point);
	++end_point.z;
			end_cc=ClipCode(end_point);
 			if (end_cc EQ CC_ALL_ON)
			{
				end_project=Project(end_point);
			}
		}
		if ( (start_cc & end_cc) NE CC_ALL_ON)
		{
			return;
		}
		if ( (start_cc | end_cc) EQ CC_ALL_ON)
		{ 
			DrawLine(start_project, end_project, 25);
			return;
		}
	}

	cc = start_cc | end_cc;
	if ( (cc & CC_OFF_BOTTOM) NE CC_ALL_ON)
	{
		if ( (start_cc & CC_OFF_BOTTOM) NE CC_ALL_ON)
		{
			start_point=Clip3DBottom(start_point,end_point);
	++start_point.z;
			start_cc=ClipCode(start_point);
 			if (start_cc EQ CC_ALL_ON)
			{
				start_project=Project(start_point);
			}
		}
		else
		{
			end_point=Clip3DBottom(end_point,start_point);
	++end_point.z;
			end_cc=ClipCode(end_point);
 			if (end_cc EQ CC_ALL_ON)
			{
				end_project=Project(end_point);
			}
		}
		if ( (start_cc & end_cc) NE CC_ALL_ON)
		{
			return;
		}
		if ( (start_cc | end_cc) EQ CC_ALL_ON)
		{ 
			DrawLine(start_project, end_project, 25);
			return;
		}
	}

	cc = start_cc | end_cc;
	if ( (cc & CC_OFF_TOP) NE CC_ALL_ON)
	{
		if ( (start_cc & CC_OFF_TOP) NE CC_ALL_ON)
		{
			start_point=Clip3DTop(end_point,start_point);
	++start_point.z;
			start_cc=ClipCode(start_point);
 			if (start_cc EQ CC_ALL_ON)
			{
				start_project=Project(start_point);
			}
		}
		else
		{
			end_point=Clip3DTop(start_point,end_point);
	++end_point.z;
			end_cc=ClipCode(end_point);
 			if (end_cc EQ CC_ALL_ON)
			{
				end_project=Project(end_point);
			}
		}
		if ( (start_cc & end_cc) NE CC_ALL_ON)
		{
			return;
		}
		if ( (start_cc | end_cc) EQ CC_ALL_ON)
		{ 
			DrawLine(start_project, end_project, 25);
			return;
		}
	}
	return;
}

const void DrawObject(const xyz *Point, const unsigned char *Line, const unsigned char Points, const unsigned char Lines, const xyz Offset)
{
	unsigned char cc_and=CC_ALL_OFF;
	unsigned char cc_or=CC_ALL_ON;
	for (int p=0; p NE Points; ++p)
	{
		const xyz rotated_point=Rotate(Point[p]);
		ObjectRotatePoint[p]=XYZ(rotated_point.x+Offset.x, rotated_point.y+Offset.y, rotated_point.z+Offset.z);
//		sprintf(string,"do %3i %3i %7i %7i %7i\n",p,Points,ObjectRotatePoint[p].x,ObjectRotatePoint[p].y,ObjectRotatePoint[p].z); debugstring();

		const unsigned char cc=ClipCode(ObjectRotatePoint[p]);
		ObjectClipCode[p]=cc;
//		sprintf(string,"do %3i %3i %7i %7i %7i %3i\n",p,Points,ObjectRotatePoint[p].x,ObjectRotatePoint[p].y,ObjectRotatePoint[p].z,cc); debugstring();
		cc_and = cc_and & cc;
		cc_or = cc_or | cc;
		if (cc EQ 0)
		{
			ObjectProjectPoint[p]=Project(ObjectRotatePoint[p]);
//			sprintf(string,"pp %3i %3i %3i %3i %3i\n",p,Points,ObjectProjectPoint[p].x,ObjectProjectPoint[p].y,ObjectProjectPoint[p].z); debugstring();
		}
	}
	if (cc_and NE CC_ALL_ON)
	{
		return;
	}
	if (cc_or EQ CC_ALL_ON)
	{
		for (int l=0, p=0; l NE Lines; ++l, p+=2)
		{
			const unsigned char start=Line[p];
			const unsigned char end=Line[p+1];
			DrawLine(ObjectProjectPoint[start], ObjectProjectPoint[end], 25); 
		}
	}
	else
	{
		for (int l=0, p=0; l NE Lines; ++l, p+=2)
		{
			const unsigned char start=Line[p];
			const unsigned char end=Line[p+1];
			const unsigned char start_cc=ObjectClipCode[start];
			const unsigned char end_cc=ObjectClipCode[end];
			if ( (start_cc | end_cc) EQ CC_ALL_ON)
			{
				DrawLine(ObjectProjectPoint[start], ObjectProjectPoint[end], 25); 
				continue;
			}
			if ( (start_cc & end_cc) NE CC_ALL_ON)
			{
				continue;
			}
			ClipLine(ObjectRotatePoint[start], ObjectRotatePoint[end], start_cc, end_cc, ObjectProjectPoint[start], ObjectProjectPoint[end]);
		}
	}
}

const void UpdateWorld() // Render the current state of the world to the screen.
{
  	Input();
	memset(pDIBBase, 0, DIBWidth*DIBHeight);    // clear frame
  	DrawObject(&ObjectPoints0000[0], &ObjectLines0000[0], 8, 12, point);

  const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);
  
  const HFONT hf=CreateFont(1, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
//  sprintf(string,"s %6x %6x %6x e %6x %6x %6x cs %6x %6x %6x %6x %6x %6x %2x\n",start.x,start.y,start.z,end.x,end.y,end.z,clip.x,clip.y,clip.z,point.x,point.y,point.z,clipcode); TextOutA(hdcScreen,1,1,string, strlen(string));

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
