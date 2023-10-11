#define WIN32_LEAN_AND_MEAN
#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct 
{	
	char Sin;
	char Cos;
} ROTATION;

#define IDM_EXIT           106

#define INITIAL_DIB_WIDTH  	320		// initial dimensions of DIB
#define INITIAL_DIB_HEIGHT	200		//  into which we'll draw

#define LT <
#define GT >
#define LE <= 
#define GE >=

#define DEBUG

BITMAPINFO *pbmiDIB;		// pointer to the BITMAPINFO
char *pDIB, *pDIBBase;		// pointers to DIB section we'll draw into
HBITMAP hDIBSection;        // handle of DIB section
HWND hwndOutput;
int DIBWidth=INITIAL_DIB_WIDTH, DIBHeight=INITIAL_DIB_HEIGHT, DIBPitch;

unsigned int Roll=0;
unsigned int Pitch=0;

FILE *file;

char string[256];

const int Horizon3[]=
{
	100,
101,
102,
103,
104,
105,
106,
107,
108,
109,
110,
111,
112,
113,
114,
115,
116,
117,
118,
119,
120,
121,
122,
123,
124,
125,
126,
127,
128,
129,
130,
131,
132,
133,
134,
135,
136,
137,
138,
139,
140,
141,
142,
143,
144,
145,
146,
147,
148,
149,
150,
151,
152,
153,
154,
155,
156,
157,
158,
159,
160,
161,
162,
163,
164,
165,
166,
167,
168,
169,
170,
171,
172,
173,
174,
175,
176,
177,
178,
179,
180,
181,
182,
183,
184,
185,
186,
187,
188,
189,
190,
191,
192,
193,
194,
195,
196,
197,
198,
199,
200,
201,
202,
203,
204,
205,
206,
207,
208,
209,
210,
211,
212,
213,
214,
215,
216,
217,
218,
219,
220,
221,
222,
223,
224,
225,
226,
227,
228,
229,
230,
231,
232,
233,
234,
235,
236,
237,
238,
239,
240,
241,
242,
243,
244,
245,
246,
247,
248,
249,
250,
251,
252,
253,
254,
255,
256,
257,
258,
259,
260,
261,
262,
263,
264,
265,
266,
267,
268,
269,
270,
271,
272,
273,
274,
275,
276,
277,
278,
279,
280,
281,
282,
283,
284,
285,
286,
287,
288,
289,
290,
291,
292,
293,
294,
295,
296,
297,
298,
299,
300,
301,
302,
303,
304,
305,
306,
307,
308,
309,
310,
311,
312,
313,
314,
315,
316,
317,
318,
319,
320,
321,
322,
323,
324,
325,
326,
327,
328,
329,
330,
331,
332,
333,
334,
335,
336,
337,
338,
339,
340,
341,
342,
343,
344,
345,
346,
347,
348,
349,
350,
351,
352,
353,
354,
355,
-155,
-154,
-153,
-152,
-151,
-150,
-149,
-148,
-147,
-146,
-145,
-144,
-143,
-142,
-141,
-140,
-139,
-138,
-137,
-136,
-135,
-134,
-133,
-132,
-131,
-130,
-129,
-128,
-127,
-126,
-125,
-124,
-123,
-122,
-121,
-120,
-119,
-118,
-117,
-116,
-115,
-114,
-113,
-112,
-111,
-110,
-109,
-108,
-107,
-106,
-105,
-104,
-103,
-102,
-101,
-100,
-99,
-98,
-97,
-96,
-95,
-94,
-93,
-92,
-91,
-90,
-89,
-88,
-87,
-86,
-85,
-84,
-83,
-82,
-81,
-80,
-79,
-78,
-77,
-76,
-75,
-74,
-73,
-72,
-71,
-70,
-69,
-68,
-67,
-66,
-65,
-64,
-63,
-62,
-61,
-60,
-59,
-58,
-57,
-56,
-55,
-54,
-53,
-52,
-51,
-50,
-49,
-48,
-47,
-46,
-45,
-44,
-43,
-42,
-41,
-40,
-39,
-38,
-37,
-36,
-35,
-34,
-33,
-32,
-31,
-30,
-29,
-28,
-27,
-26,
-25,
-24,
-23,
-22,
-21,
-20,
-19,
-18,
-17,
-16,
-15,
-14,
-13,
-12,
-11,
-10,
-9,
-8,
-7,
-6,
-5,
-4,
-3,
-2,
-1,
0,
1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
26,
27,
28,
29,
30,
31,
32,
33,
34,
35,
36,
37,
38,
39,
40,
41,
42,
43,
44,
45,
46,
47,
48,
49,
50,
51,
52,
53,
54,
55,
56,
57,
58,
59,
60,
61,
62,
63,
64,
65,
66,
67,
68,
69,
70,
71,
72,
73,
74,
75,
76,
77,
78,
79,
80,
81,
82,
83,
84,
85,
86,
87,
88,
89,
90,
91,
92,
93,
94,
95,
96,
97,
98,
99,

/*100,
101,
102,
103,
104,
105,
106,
107,
108,
109,
110,
111,
112,
113,
114,
115,
116,
117,
118,
119,
120,
121,
122,
123,
124,
125,
126,
127,
128,
129,
130,
131,
132,
133,
134,
135,
136,
137,
138,
139,
140,
141,
142,
143,
144,
145,
146,
147,
148,
149,
150,
151,
152,
153,
154,
155,
156,
157,
158,
159,
160,
161,
162,
163,
164,
165,
166,
167,
168,
169,
170,
171,
172,
173,
174,
175,
176,
177,
178,
179,
180,
181,
182,
183,
184,
185,
186,
187,
188,
189,
190,
191,
192,
193,
194,
195,
196,
197,
198,
199,
200,
201,
202,
203,
204,
205,
206,
207,
208,
209,
210,
211,
212,
213,
214,
215,
216,
217,
218,
219,
220,
221,
222,
223,
224,
225,
226,
227,
228,
229,
230,
231,
232,
233,
234,
235,
236,
237,
238,
239,
240,
241,
242,
243,
244,
245,
246,
247,
248,
249,
250,
251,
252,
253,
254,
255,
256,
257,
258,
259,
260,
261,
262,
263,
264,
265,
266,
267,
268,
269,
270,
271,
272,
273,
274,
275,
276,
277,
278,
279,
280,
281,
282,
283,
284,
285,
286,
287,
288,
289,
290,
291,
292,
293,
294,
295,
296,
297,
298,
299,
300,
301,
302,
303,
304,
305,
306,
307,
308,
309,
310,
311,
312,
313,
314,
315,
316,
317,
318,
319,
320,
321,
322,
323,
324,
325,
326,
327,
328,
329,
330,
331,
332,
333,
334,
335,
336,
337,
338,
339,
340,
341,
342,
343,
344,
345,
346,
347,
348,
349,
350,
351,
352,
353,
354,
355,
-155,
-154,
-153,
-152,
-151,
-150,
-149,
-148,
-147,
-146,
-145,
-144,
-143,
-142,
-141,
-140,
-139,
-138,
-137,
-136,
-135,
-134,
-133,
-132,
-131,
-130,
-129,
-128,
-127,
-126,
-125,
-124,
-123,
-122,
-121,
-120,
-119,
-118,
-117,
-116,
-115,
-114,
-113,
-112,
-111,
-110,
-109,
-108,
-107,
-106,
-105,
-104,
-103,
-102,
-101,
-100,
-99,
-98,
-97,
-96,
-95,
-94,
-93,
-92,
-91,
-90,
-89,
-88,
-87,
-86,
-85,
-84,
-83,
-82,
-81,
-80,
-79,
-78,
-77,
-76,
-75,
-74,
-73,
-72,
-71,
-70,
-69,
-68,
-67,
-66,
-65,
-64,
-63,
-62,
-61,
-60,
-59,
-58,
-57,
-56,
-55,
-54,
-53,
-52,
-51,
-50,
-49,
-48,
-47,
-46,
-45,
-44,
-43,
-42,
-41,
-40,
-39,
-38,
-37,
-36,
-35,
-34,
-33,
-32,
-31,
-30,
-29,
-28,
-27,
-26,
-25,
-24,
-23,
-22,
-21,
-20,
-19,
-18,
-17,
-16,
-15,
-14,
-13,
-12,
-11,
-10,
-9,
-8,
-7,
-6,
-5,
-4,
-3,
-2,
-1,
0,
1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
26,
27,
28,
29,
30,
31,
32,
33,
34,
35,
36,
37,
38,
39,
40,
41,
42,
43,
44,
45,
46,
47,
48,
49,
50,
51,
52,
53,
54,
55,
56,
57,
58,
59,
60,
61,
62,
63,
64,
65,
66,
67,
68,
69,
70,
71,
72,
73,
74,
75,
76,
77,
78,
79,
80,
81,
82,
83,
84,
85,
86,
87,
88,
89,
90,
91,
92,
93,
94,
95,
96,
97,
98,
99,*/
};

const void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

const void debug(const char* str) { sprintf(string,"%s\n",str); debugstring(); }

const void dbg(const char* str)
{
#ifdef DEBUG
 debug(str); 
#endif
}

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

char CosTab[1026];
char SinTab[1026];

const void CalcSinCosTab()
{
	const double double_pi=(22.0f/7.0f)*2.0f;
//	sprintf(string,"double_pi %f\n",double_pi); debugstring();
	const double inc=double_pi/1024.0f; 

	int i=0;
	for (double c=0.0; c<=double_pi; c+=inc, ++i)
	{
		CosTab[i]=cos(c)*127.0;
		SinTab[i]=sin(c)*127.0;
//		sprintf(string,"i %i c %3.5f sf %3.3f si %3i cf %3.3f ci %3i\n",i,c,cos(c),CosTab[i],sin(c),SinTab[i]); debugstring();
	}
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
//  sprintf(string,"bb %f\n",123.45); debugstring();
 
  CalcSinCosTab();
  
  return TRUE;
}

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

const void Input()
{
  if (GetAsyncKeyState(VK_UP))
  {	
	Pitch = (Pitch-1) & 1023;
  }
  if (GetAsyncKeyState(VK_DOWN))
  {	
	Pitch = (Pitch+1) & 1023;
  }
  if (GetAsyncKeyState(VK_RIGHT))
  {	
	Roll = (Roll-1) & 1023;
  }
  if (GetAsyncKeyState(VK_LEFT))
  {	
	Roll = (Roll+1) & 1023;
  }
}

const ROTATION GetSinCos(const unsigned int angle) 
{
	ROTATION rot;
	rot.Sin=SinTab[angle];
	rot.Cos=CosTab[angle];
//	sprintf(string,"a %5i s %5i c %5i\n",angle,rot.Sin,rot.Cos); debugstring();
	return rot;
}

int dir;
 
const void DrawHorizon(const int pitch, const int roll)
{
	const ROTATION roll_sin_cos = GetSinCos(roll);
	
	const int pitch_sin_ver = roll_sin_cos.Sin + ( (roll_sin_cos.Sin * (DIBWidth >>1)) / roll_sin_cos.Cos);
//	int pitch_cos_hor = (DIBWidth >>1);

//	const int pitch_pos = ( (pitch -256) &1023);
	dir = ( (pitch -256) &1023) >>9;

	const int pitch_height = Horizon3[(Pitch &511)]; //Horizon2[pitch_pos];//Horizon[Pitch];

	int sy = pitch_height-pitch_sin_ver;
	int ey = pitch_height+pitch_sin_ver;
	int sx = 0;
	int ex = DIBWidth;

	if (ey LT sy)
	{
		int temp =sx;
		sx =ex;
		ex =temp;
		temp =sy;
		sy =ey;
		ey =temp;
	}

	if (ey <0)
	{
		return;
	}
	if (sy >(DIBHeight-1))
	{	
		return;
	}
	if (sy <0)
	{
		sx = sx - ( ( (ex-sx) *sy ) / (ey-sy) );
		sy=0;
	}
	if (ey >(DIBHeight-1))
	{
		ex = ex - ( (ex-sx) *(ey-DIBHeight) / (ey-sy) );
		ey = DIBHeight-1;
	}
	Line( sx, sy, ex, ey, 15 );
}

const void UpdateWorld() // Render the current state of the world to the screen.
{
  Input();
  memset(pDIBBase, 0, DIBWidth*DIBHeight);    // clear frame
  DrawHorizon(Pitch, Roll);

  const HDC hdcScreen = GetDC(hwndOutput); // We've drawn the frame; copy it to the screen
  const HDC hdcDIBSection = CreateCompatibleDC(hdcScreen);
  const HBITMAP holdbitmap = SelectObject(hdcDIBSection, hDIBSection);
  BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection, 0, 0, SRCCOPY);

  const HFONT hf=CreateFont(1, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
  sprintf(string,"%4i %4i %4i\n",Roll,Pitch,dir); TextOutA(hdcScreen,1,1,string, strlen(string));

  ReleaseDC(hwndOutput, hdcScreen);
  SelectObject(hdcDIBSection, holdbitmap);
  DeleteDC(hdcDIBSection);

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


