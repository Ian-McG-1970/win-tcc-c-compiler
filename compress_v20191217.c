#include <windows.h>   	// required for all Windows applications
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

FILE *file;
char string[65535];

typedef struct
{
	unsigned char value;
	int count;
} COMPRESS_BIG;

typedef struct
{
	int value;
	int count;
} COMPRESS_MEDIUM;

typedef struct
{
	unsigned char type_count;
} COMPRESS_SMALL;

const unsigned char array[]=
"ABCDEF\
GHIJKL";

const unsigned char uncompressed1[]= 
"11111111111111111111111111111111\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
10000000000000000000000000000001\
11111111111111111111111111111111";
const unsigned char uncompressed2[]= 
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W...... ..d.r .....r.r....... ....r....W\
W.rXr...... .........rd..r.... ..... ..W\
W.......... ..r.....r.r..r........r....W\
Wr.rr.........r......r..r....r...r.....W\
Wr. r......... r..r........r......r.rr.W\
W... ..r........r.....r. r........r.rr.W\
Wwwwwwwwwwwwwwwwwwwwwwwwwwwwwww...r..r.W\
W. ...r..d. ..r.r..........d.rd...... .W\
W..d.....r..... ........rr r..r....r...W\
W...r..r.r..............r .r..r........W\
W.r.....r........rrr.......r.. .d....r.W\
W.d..  ..r. .....r.rd..d....r...r..d. .W\
W. r..............r r..r........d.....rW\
W........wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwW\
W r.........r...d....r.....r...r.......W\
W r......... r..r........r......r.rr..PW\
W.  ..r........r.....r. ....d...r.rr...W\
W....rd..r........r......r.rd......r...W\
W... ..r. ..r.rr.........r.rd...... ..rW\
W.d.... ..... ......... .r..r....r...r.W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWr";

const unsigned char uncompressed3[]= 
"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W...... ..d.r .....r.r....... ....r....W\
W.rXr...... .........rd..r.... ..... ..W\
W.......... ..r.....r.r..r........r....W\
Wr.rr.........r......r..r....r...r.....W\
Wr. r......... r..r........r......r.rr.W\
W... ..r........r.....r. r........r.rr.W\
Wwwwwwwwwwwwwwwwwwwwwwwwwwwwwww...r..r.W\
W. ...r..d. ..r.r..........d.rd...... .W\
W..d.....r..... ........rr r..r....r...W\
W...r..r.r..............r .r..r........W\
W.r.....r........rrr.......r.. .d....r.W\
W.d..  ..r. .....r.rd..d....r...r..d. .W\
W. r..............r r..r........d.....rW\
W........wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwW\
W r.........r...d....r.....r...r.......W\
W r......... r..r........r......r.rr..PW\
W.  ..r........r.....r. ....d...r.rr...W\
W....rd..r........r......r.rd......r...W\
W... ..r. ..r.rr.........r.rd...... ..rW\
W.d.... ..... ......... .r..r....r...r.W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W.r..r..w.r...d.w... .r.wr......w..rr..W\
W.......w......rwrr. ...w ..d...w....r.W\
W                                      W\
Wd......w.r....rw.r. .. w..r..d.w..r.r.W\
W.......w.r....rw.r. r..w.....r.w... ..W\
Wwwwwwwwwwwwwwwwwwww wwwwwwwwwwwwwwwwwwW\
W....rr.w..r....w... ..rw....r..w.....rW\
W.......w.. ....w... ...w....r. w.....rW\
W                                      W\
Wr..r...w....r..w..r ...w......dwr.....W\
Wr....r.w..r..r.w... . rw.......wr...r.W\
W.r.....w...r...w... . rw.......w r..r.W\
Wwwwwwwwwwwwwwwwwwww wwwwwwwwwwwwwwwwwwW\
Wr.  q..w....r.rw... ...w.rd..r.w......W\
W.....r.wr......w..d ...w ..r...w.r.rr.W\
W                                      W\
Wd.. .r.wr....r.w.r. ..rw.r.r...w......W\
W.....r.wr..d...w... r..w..r....w...rr W\
W.d... rw..r....w.Xd r..w. .....w...rr W\
W.r.... w.. ..r.w.P. ...w....r.rw.... .W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wr.ww.wrr.w...rwr..r....w...r.....rw.d.W\
W..Xw.d.r.w...www..w.r....r..r.r...w.wrW\
W....w..rd..r....w.....r.wwr.......w.wwW\
Wd.w..wrwr..r....w...r......r.rr......wW\
Wr.w...w..r.ww..r.wwd.......r.rr......wW\
Wrr..r....w...r......r.rr......r..dww..W\
W..r.ww..r.rr...w....r.rr......w..r.w.rW\
W..w...d......d.r..wwr..r.w.wr..wr..d.rW\
Wr.r....w.ww..d.r..wwr..r..d.w...w..r.wW\
W.r.ww.....rrwr..d.w.wr..wr...wr..d.r..W\
Ww.ww......rrwr..r.w.ww...w..r.ww..r.wwW\
W.w.r.r.w...wwr..r....w...r.....ww.r.wwW\
W.w.r.r.w.d.w.wr..wr....r..r.rr....w...W\
Ww..wrwr..r....w...d...w.rw......w.ww.dW\
Ww...wwr..w.d...wr..r.r...r.wr......w..W\
Ww.d....r.ww..r.wwr.......r.wr......w..W\
W..r....w...r......r.rr......w..r.w...wW\
Wr.ww..r.ww...w....r.rr......w..rd..r..W\
Ww...r......r.rd......r...ww..wr..d.w..W\
Wrr...w.....r.rd......w..r.wd.d.rw.r...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WX.....r....................r........r.W\
W.....r..............r.................W\
W........r..r..........................W\
Wr.....................................W\
W...................r..................W\
W.r.....................r.........r....W\
W..r.....r...........r..r.............rW\
W......r......r.....................r..W\
W.......  B ..r.  B ....  B ....  B ...W\
W.......    ..r.    ....    ....    r..W\
W......................................W\
W...r..............................r...W\
W...r.....r............................W\
W......r...........r..................rW\
W...........r.......r..................W\
W..r..............r....................W\
W.....................r.........r......W\
W................................r..r..W\
W....r......r.rr..................r....W\
W...........r.rr.........r..r.r.......PW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WX.....................................W\
W......................................W\
W......................................W\
W......................................W\
W......................................W\
W......................................W\
W......................................W\
W.......  q.....  q.....  q.....  q....W\
W.......   .....   .....   .....   ....W\
W....... d ..... d ..... d ..... d ....W\
W......................................W\
W......................................W\
W......................................W\
W.......  q.....  q.....  q.....  q....W\
W.......   .....   .....   .....   ....W\
W....... d ..... d ..... d ..... d ....W\
W......................................W\
W......................................W\
W......................................W\
W......................................W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wwwwwwwwww....r.r..r........r.wwwwwwwwwW\
W         ...........r....r...         W\
W  dq     ..r..........r...r..  qd     W\
Wwwwwwwwww..r........r......r.wwwwwwwwwW\
W         ......r...r.......r.         W\
W  dq     ....r......r.rr.....  qd     W\
Wwwwwwwwww.rr........r.rr.....wwwwwwwwwW\
W         ....r.r....r..r.....         W\
W  dq     ....r.r....r..r..r..  qd     W\
Wwwwwwwwww.rr.r..r....r...r...wwwwwwwwwW\
W         .rr.r..r............         W\
W  dq     ....r..r........r...  qd     W\
Wwwwwwwwww.....r...r....r..r..wwwwwwwwwW\
W....r.r..r........r.....r............rW\
W......r....r....r..r.r...r..r.........W\
W..r....r.....r...r.......r..r.........W\
W..r........r......r.rr.........r......W\
Wr.X...r...........r.rr.........rr..r.PW\
W....r......r.rr......r........r..r....W\
Wrr.........r.rr.........r..r.r.r..r...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W. .. .rr..... ..r. X.... rr r..r. .  .W\
W ..r. .. .  .... .r.r. ...  r..r.d.. .W\
Wr.....  .q.  ... .r.r. ... wwwwwwwwwwwW\
W.r.d... .  ...... ..rr..r.... . ... . W\
Wwwwwwwwwwwww.r. ..   r.. .... ...r....W\
Wr. r...... ..r. ... ..r.  ..r.  q.....W\
Wr. r...... .. r..r.... ...r......r.rr.W\
W... ..r  ... ..r.  ..r.  ... ....r.rr.W\
W... ..r. .r.... ...q......r.r..  r..r.W\
W  .. r.... ..r.r.... .  .......  d.. .W\
W. ... .. .  .. .  .....rr r..r. . r.. W\
W.. d..r.r.... .  ......r  r..r. .  ...W\
W.r.  ..r.  ... .r.r. ...  r.. .... ...W\
W....  .r.  ... .r.r. .r. . r.. r.... .W\
W.  .... ....  .. r r..r.... ...r... .rW\
W..... .  .rr. ...  r.. .r... r..r.r...W\
W r...... ..r. .r.... .  ..r.  r.......W\
W r...... .. r..r.... ...r......r.rr...W\
W. ..r. ... ..r.  .aa.  ... ....r.rr...W\
W. .drq..r.... ...r......r.rq.....dr...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W . r.. . .. ..r. ..X ..r.  ..r. r... .W\
W.r.rr...... ..r...r.... ...r.....dr.r.W\
W r..r...  ...r..r. ..r.r...wwwwwwwwwwwW\
W...d ..r. q.....r..... ........rr r..rW\
Wwwwwwwwwwwww..r.r.... .  ......r  r..rW\
W.  ... ..r.  ..r.  .... rrr.....  r.. W\
W... r... q.. ..r.  .....r.rr..r. . r..W\
W..r. ..r. r.... ..... ...r r..r.... ..W\
W.....r ...... .  qrr. ...  r.. .r....rW\
Wr.r... . r...... ..r...r....r....dr.  W\
W......r. r......... r..r...wwwwwwwwwwwW\
W.rr...... ..r. ... ..r.  ..r.  ... r..W\
Wwwwwwwwwwwwwr........ ...r......r.rr..W\
W..r...  ...d..r. ..r.rr.........r.rr..W\
W.. ..r. .r...mmmmmmmm.........  r..r..W\
Wr.. r....r..r r...d .. .......  r..r..W\
W ... ..r. ...r.  .....rrrr..r. . r.. rW\
W. r..q.r.... .  ......rr r..r...  ...rW\
Wr.  ..r.  .....r.r. ...  r..r.... ...rW\
W...  .r.r .....r.r.....   .. .r....r..W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wdddrrddrddr.rrrrdrdd.ddrddrddddrrdrdrrW\
Wdrrdddrrrdrddrrrrrrdrrd.drdrrrrdrddrrdW\
Wddrrrrrrrdrddrr.rrrdrrdddrdr.rrdrrrddrW\
Wrrdrddrrrrrrdrrddd..ddrrdrddrrdrdd.rrdW\
Wrrdrddrrrrrrdrrd.drdrrrrdrdrdrrddrrdrdW\
Wdddrrdrd.ddrrddrrdddrrdrdrrr.drddrrdrdW\
Wrrrrrdrrdddd..rrrdrdd.rdrddr.rrddddddrW\
Wdrddwwwwwww.wwwwwdrrrrdrwwwwww.wwwwwwrW\
Wd.dd             rddrrrd             rW\
Wdrdr  XP         rddrrrd             rW\
Wdrrd             r.rrddr             rW\
Wdrrd             ddddrdr             dW\
Wrddd             drrd.dr             dW\
Wrrrr             drrddrr             rW\
Wdrdd             .rdrrdr             rW\
Wdrdd            wwwwwwwww            rW\
Wrrrd                                 rW\
Wrrrd             dd.rdrd             rW\
Wddrr             rrrdrdd             rW\
Wdd..wwwwwwwwwwwwwdrrrdddwwwwwwwwwwwwwdW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W............X.........................W\
Wwwwwwwwwwwww wwwwwwww.................W\
Ww....d.............dw.................W\
Ww.w w.wwwwww wwwwww.w.................W\
Ww.qqq.wd.........dw.w.................W\
Ww.qqq.w.wwww wwww.w.w.................W\
Ww.qqq.w.wd.....dw.w.w.................W\
Ww.qqq.w.w.ww ww.w.w.w.................W\
Ww.qqq w w w   w w w w.................W\
Ww.qqqqwqwqwqqqwqwqwqw.................W\
Ww.qqq w w w   w w w w.................W\
Ww.qqq.w.w.wwwww.w.w.w.................W\
Ww.qqq.w.wd.....dw.w.w.................W\
Ww.qdq.w.wwwwwwwww.w.w.................W\
Ww.qdq.wd.........dw.w.................W\
Ww.qdq.wwwwwwwwwwwww.w.................W\
Ww.qdqd.............dw.................W\
Wwwwwwwwwwwwwwwwwwwwww.................W\
W......................................W\
W......................................W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wr.rd.rrr.w...drr..rw...d...r.w...dr.r.W\
W... .r.r.w...r r..rwr....r..rwr...r.rrW\
W.... ..rrw.r....r..w..r. rr..w....r.rrW\
Wr.r.. rrrw.r.... ..wr......r.wr......rW\
Wr. ...r..w.  ..r.rrw.......r.wr...... W\
Wrr..r....w...r.....wr.rr.....wr..r r..W\
W..r.rr..rwrr...r...wr.rr.....wr..r. .wW\
W..r...r..w...r.r..rwr..r. .rrw. r..qwrW\
Wr.r.wwwwwwwwwqwwwwwwwwwrwwwwwwwww..w. W\
W.r.  .....rrrr..r.r.rr..rr... r..rwr..W\
Wr.rr......rrrr..r. . r...r..r.rr.wr.rrW\
W. .r.r. w..rrr..r.... ...r.....rw.r.rrW\
W. .r.r. wr.wwwwwwwwwwwwwwwwwrr.w..r...W\
Wr.. rrr.wr....r...r... .rr....w.r.rr.rW\
Wr...rrr.wr.r... r..r.r...r.rrw.....r.PW\
W .r....rw  ..r.rrr.......r.rw...... ..W\
W..r.... w..r......r.rr.....wr..r.r...rW\
Wr.rr..r.wr...r....rXrr......r..rq..r..W\
Wr...r...w..r.rq......r... r.. r..rdr..W\
Wrr.d. ..w..r.rr......r..r. r.q.rr.r...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wr. ...rr.....r.r..r........r.....r..d.W\
W.....d.r....... ....r....r..r..... ..rW\
W.......rdw.r.w.. w...wr...r..  q . .. W\
Wdwwwwwwwwwww.w...w..rw.....r.    .....W\
Wr........w...w.r.w d.w.....r..........W\
Wrr..r....w...w...w..rwrr......r..d....W\
W..r.....rwrr.w...w..rwrr.........r...rW\
W.wwwwwwwwwww.w.r.w .rw.r....r  q ..d.rW\
Wr.r......w...w.r.w..rw.r..d..    ..r..W\
W.r.......wrr w..dw.. w...r.......d.r..W\
W ........wrr w..rw...w... ..r.....r...W\
W.wwwwwwwwwwwwwwwrw...w...r........r.  W\
W...r.r...w...wr..wr..w.r..r.r  q .....W\
W....r r..w...w...wd..w..r ...    ....dW\
W.... .r..w.d.w..rw.r.w...r. r.........W\
W.wwwwwwwwwww.w...w...w...r. r.........W\
W..r......w.r.w...wr.rw...... ..r......W\
Wr.X...r. w...w...wr.rw.........rd..r..W\
W....r....w.r.wd..w...w.... ...r..d. ..W\
Wrr.......w.r.wd..w...w..r..d.d.r..r...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wr.....rr.P.....r.Xra.......r........r.W\
W.....r.r............r....r..r.r.......W\
W........r..r..............r...........W\
Wr.......r...........r......r..r.......W\
Wr........r.....r...r.......r..r.......W\
W.r..r........r......r.rr.........r....W\
W..r.....r...........r.rr.........r...rW\
W......r......r.r....r..r........r..r..W\
Wr.r..........r.r..........r...........W\
W..........rr.r..r....r...r....r..r.r..W\
W..........r..r..r...........r.....r...W\
W...r.r.......r...........r........r...W\
W...r.r...r....r...r.......r...........W\
W....r.r..r........r.....r............rW\
W......r....r....r..r.r......r.........W\
W..r.wwwwwwwwwwwwwwwwwwwwwwwwwwwwww....W\
W..r.BBBBBBBBBBBBBBBBBBBBBBBBBBBBBB....W\
Wr...rrrrrrrrrrrrrrrrrrrrrrrrrrrrrr.r..W\
W......................................W\
W.r................................r...W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W..X...................................W\
W......................................W\
W......................................W\
W......................................W\
W.....................q.q.q.q.q.q......W\
W.....................r.r.r.r.r.r......W\
W......................................W\
W......................................W\
W......... B. . . . . .................W\
W.........  . . . . . .................W\
W.........  .B. . . . .................W\
W.........  . . . . . .................W\
W.........  . .B. . . .................W\
W.........  . . . . . .................W\
W.........  . . .B. . .................W\
W.........  . . . . . .................W\
W.........  . . . .B. .................W\
W.........  . . . . . .................W\
W.........  . . . . .B.................W\
W......................................W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wr.rr..  r..r..r.r..Xr..r.rr..r.rr...r.W\
W.w.rr......r..r...r....w...r......r.r.W\
Wrrw.r... r.. r..r.r..rwr.... .. ..r.rqW\
W...wr..r. q.....r. ..wr.  .....rrrr..rW\
W.rr.wrr... r..r.r...wr. r......rrrr..rW\
W. r..wr..r.r ..r.rrw... rrr. ...rrr..rW\
W...rr.w. q..r..r.rw.....r.rr..r.r.rr..W\
W..r.r..w.rr.... .w...r.. rrr..r....r..W\
W... .rr.w....r. wqrr. ...rrr..r.r... rW\
Wr.r...r.rw.....wr..r. .r....r.  ..r.rrW\
W......r.rrw...w. ..rr..r.... ...r.....W\
W.rr......r....r...r..r.r ..r.rr... r..W\
W.rr......r.mmm..r....r...r......r.rr..W\
W..r... r...r..r.r..r.rr... .....r.rr..W\
W..r..r. .r....r.....r.  ......rrr..r. W\
Wr.. r....r..r.r....r.  .......rrr..r..W\
Wr...r..r.  ..r.  .... rrrr..r.r.rr..rrW\
W. r..q r....r.rr......rrrr..r. .rr.. rW\
Wr.rr..r.rr... .r.r. ...rrr..r.... ...rW\
W...rr.r.rr... .r.r.P...r r..r.r....r..W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WX..r..........r........r.....r..r.....W\
W.r.rr.........r...r........r......r.r.W\
W.r..r........r..r....r.r..........r.rrW\
W.......r..r.....r..............rr.r..rW\
W..r...r....r..r.r..............r..r..rW\
W.........r.....r........rrr.......r...W\
W....r....r.....r........r.rr..r....r..W\
W..r...mmmmmm..mmmmmm.....r.r..r.......W\
W.....rw....w..w..rrw.......r....r....rW\
Wr.r...w..r.w..w....w...r....r.....r...W\
W......w..r.w..w....wr..r........r.....W\
W.rr...w....wr.w....w.r.....r.......r..W\
W.rr...w....wrrw.r..w.....r......r.rr..W\
W..r...w....w..w....w.rr.........r.rr..W\
W.....rwwwwww..wwwwww............r..r..W\
Wr...r....r..r.r.................r..r..W\
W.............r........r.....r........rW\
W..r..r.  q ....  q ...r  q .r..  q ..rW\
Wr.....r    ....    ....    .r..    ..rW\
W......r.......................r....r..W\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W         r         WWWWWWWWWWWWWWWWWWWW\
W  X      .         WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W         b       P WWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
Wrq...............r.WWWWWWWWWWWWWWWWWWWW\
WXrq.............rP.WWWWWWWWWWWWWWWWWWWW\
Wd.rq...........r.d.WWWWWWWWWWWWWWWWWWWW\
Wrd.rq.........r.dr.WWWWWWWWWWWWWWWWWWWW\
W.rd.rq.......r.dr..WWWWWWWWWWWWWWWWWWWW\
W..rd.rq.....r.dr...WWWWWWWWWWWWWWWWWWWW\
W...rd.rq...r.dr....WWWWWWWWWWWWWWWWWWWW\
W....rd.rq.r.dr.....WWWWWWWWWWWWWWWWWWWW\
W.....rd.rr.dr......WWWWWWWWWWWWWWWWWWWW\
W......rd..dr.......WWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W         X         WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W                 P WWWWWWWWWWWWWWWWWWWW\
W                   WWWWWWWWWWWWWWWWWWWW\
W              qqqq WWWWWWWWWWWWWWWWWWWW\
W              qqqq WWWWWWWWWWWWWWWWWWWW\
W              qqqq WWWWWWWWWWWWWWWWWWWW\
Wddddddddddddddqqqq WWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
W..X.......rrr......WWWWWWWWWWWWWWWWWWWW\
W..........rrr......WWWWWWWWWWWWWWWWWWWW\
W...................WWWWWWWWWWWWWWWWWWWW\
W..........mmm......WWWWWWWWWWWWWWWWWWWW\
W.......r..   ......WWWWWWWWWWWWWWWWWWWW\
W........r.   ......WWWWWWWWWWWWWWWWWWWW\
W.........r   ......WWWWWWWWWWWWWWWWWWWW\
W........P.mmm......WWWWWWWWWWWWWWWWWWWW\
W..........   ......WWWWWWWWWWWWWWWWWWWW\
W..........   ......WWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";

const unsigned char uncompressed4[]= 
"1111\
1AB4\
5670\
7C77\
1111";

const unsigned char uncompressed5[]= 
"1111\
1234\
5660\
7777\
1111";

const unsigned char uncompressed6[]= 
"1234567\
1111111\
7777777\
1234567";
;

const unsigned char uncompressed7[]= 
"1234A67\
1111111\
7777777\
1234A67";

#define NOT_FOUND 255
#define MAX_NO 15
#define VALUE_SHIFT 4
#define MAX_SIZE 65535

const void debugstring() { file=fopen("log.txt","a"); fprintf(file,string); fclose(file); }

const BOOL compare(const unsigned char* test1, const unsigned char* test2, const int count)
{
	for (int c=0; c!=count; ++c)
	{
		if (test1[c]!=test2[c])
		{ 
			return FALSE;
		}
	}
	return TRUE;
}

unsigned char array_type[255]; // list of different types
int array_types; // count of different types

const int find_type_in_list(const unsigned char type)
{
	for (int t=0; t!=array_types; ++t) // read through all current types
	{
		if (type==array_type[t])
		{
			return t; // type found in list
		}
	}
	return NOT_FOUND;
}

const void decompress(const unsigned char* input, const int input_count, unsigned char* output)
{
	unsigned char type[255];

	const int types=input[0];
	memcpy(&type[0], &input[1], types*sizeof(type[0]));
	for (int c=0, pos=types+1; pos!=input_count; ++pos)
	{
		const unsigned char value=input[pos] >>VALUE_SHIFT;
		const int count=input[pos] &MAX_NO;
		memset(&output[c], type[value], count);
		c+=count;
 }
}

const int compress(const unsigned char* input, const int input_count, unsigned char *output)
{
	unsigned char array_value[MAX_SIZE]; // this value 
	int array_count[MAX_SIZE]; // number of this consequitive value

	int array_pos=0;
	unsigned char prev_value=255;

	for (int p=0; p!=input_count; ++p) // create list of duplicate values
	{
		if (prev_value!=input[p])
		{
			array_value[array_pos]=prev_value=input[p];
			array_count[array_pos++]=0;
		}
		++array_count[array_pos-1];
	}

	array_types=0;
	for (int a=0; a!=array_pos; ++a) // create list of unique types
	{
		if (find_type_in_list(array_value[a])==NOT_FOUND)
		{
			array_type[array_types++]=array_value[a]; // add type to list
		}
	}
	
	output[0]=array_types;
	memcpy(&output[1], &array_type[0], array_types*sizeof(array_type[0]));
	int compressed_count=array_types+1;

	for (int a=0; a!=array_pos; ++a)
	{
		const unsigned char value=find_type_in_list(array_value[a]);
		int count=array_count[a];
		while (count!=0)
		{
			const unsigned char count_dec=(count>=MAX_NO) ? MAX_NO : count;
			output[compressed_count++]= (value <<VALUE_SHIFT) | (count_dec);
			count-=count_dec;
		}
	}
	return compressed_count;
}

int main()
{
	file=fopen("log.txt","w");

	unsigned char uncompressed[MAX_SIZE], compressed[MAX_SIZE];;

	int compressed_count=compress(uncompressed2, sizeof(uncompressed2)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed2)-1,compressed_count, compare(uncompressed2, uncompressed, sizeof(uncompressed2)-1)); debugstring();

	compressed_count=compress(uncompressed1, sizeof(uncompressed1)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed1)-1,compressed_count, compare(uncompressed1, uncompressed, sizeof(uncompressed1)-1)); debugstring();

	compressed_count=compress(uncompressed3, sizeof(uncompressed3)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed3)-1,compressed_count, compare(uncompressed3, uncompressed, sizeof(uncompressed3)-1)); debugstring();

	compressed_count=compress(uncompressed4, sizeof(uncompressed4)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed4)-1,compressed_count, compare(uncompressed4, uncompressed, sizeof(uncompressed4)-1)); debugstring();

	compressed_count=compress(uncompressed5, sizeof(uncompressed5)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed5)-1,compressed_count, compare(uncompressed5, uncompressed, sizeof(uncompressed5)-1)); debugstring();

	compressed_count=compress(uncompressed6, sizeof(uncompressed6)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed6)-1,compressed_count, compare(uncompressed6, uncompressed, sizeof(uncompressed6)-1)); debugstring();

	compressed_count=compress(uncompressed7, sizeof(uncompressed7)-1, compressed);
	decompress(compressed, compressed_count, uncompressed);
	sprintf(string,"cc uc cp %i %i %i\n",sizeof(uncompressed7)-1,compressed_count, compare(uncompressed7, uncompressed, sizeof(uncompressed7)-1)); debugstring();
}
