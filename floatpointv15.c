#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DBG

typedef struct
{
 unsigned char power_sign;
 unsigned char value;
} FP;

typedef struct
{
 unsigned char MSB;
 unsigned char PSB;
 unsigned char LSB;
} MPL;

const FP SetFP(const unsigned char mantissa, const char exponent)
{
	FP rc;
	rc.power_sign=mantissa;
	rc.value=exponent;

// printf("SetFP %x %x %x %x\n",mantissa,exponent,rc.power_sign,rc.value);
	return rc;
}

const MPL SetMPL(const long number)
{
	MPL rc;
	rc.MSB=number >>16;
	rc.PSB=number >>8;
	rc.LSB=number;
	return rc;
}

const FP Bit24ToFP(long number)
{
	unsigned char sign=0;
	if (number <0)
	{
		number = 0-number;
		sign=1;
	}

	const MPL mpl=SetMPL(number);
	unsigned long value;
	unsigned char position;

	if (mpl.MSB !=0)
	{
		position=24;
		value=(mpl.MSB<<8)+(mpl.PSB);
	}
	else if (mpl.PSB !=0) 
	{
		position=16;
		value=(mpl.PSB<<8)+(mpl.LSB);
	}
	else if (mpl.LSB !=0)
	{
		position=8;
		value=(mpl.LSB<<8)+0;
	}
	else
	{
		return SetFP(0,0);
	}

	while (value < 65536)
	{
		--position;
		value = value <<1;
	}
	return SetFP((position <<2) | sign, (value - 65536) >>8);
}

const unsigned char FPToBit8(const FP number)
{
	unsigned char rc=0;
	unsigned char carry=0;

	if (number.power_sign >127)
	{
		if (number.power_sign > 0x0FC)
		{
			carry=1;
		}
	}
	else
	{
		unsigned char mantissa = number.power_sign >>2;
		if (mantissa >7)
		{
			return 0; // error
		}
		rc = (number.value >>1) +128;
		while (mantissa!=7)
		{
			++mantissa;
			carry = rc &1;
			rc = rc >>1;
		}
	}
	rc = rc + carry;
	if (rc >127)
	{
		return 0;
	}
	if ((number.power_sign &1)!=0)
	{
		return 256-rc;
	}
	return rc;
}

const unsigned int FPToBit16(const FP number)
{
	if (number.power_sign >127)
	{
		if ((number.power_sign &1)!=0)
		{
			return 0xffff;
		}
		return 0;
	}
	unsigned char mantissa = number.power_sign >>2;
	if (mantissa >15)
	{
		return 0; // error
	}
	unsigned long rc = ( ( (number.value >>1) +128) <<8) + ((number.value &1) <<7);
	while (mantissa!=15)
	{
		++mantissa;
		rc = rc >>1;
	}
	if ((number.power_sign &1)!=0)
	{
		return 65535-rc;
	}
	return rc;
}

const unsigned char Overflow(const long number)
{
	if ( (number >127) || (number<-128) )
	{
		return 1;
	}
	return 0;
}

const FP FPAdd2(const FP first, const FP second)
{
	FP rc;

	long ab18 = first.power_sign - second.power_sign;
//		TYA					; get high?
//      SEC 
//      SBC 	ab09		; subtract high?
	
	if ( Overflow(ab18) !=0)
	{
		if (first.power_sign >127)
		{
			return first;
		}
		return second;
	}
// 		BVS 	b836C		; if overflow then 

	ab18++;
//      STA 	ab18		; store result
//      ADC 	#$01		; add the carry

	if ( Overflow(ab18) !=0)
	{
		if (first.power_sign >127)
		{
			return first;
		}
		return second;
	}
//        BVS 	b836C		; if overflow then

	return rc;
}

const unsigned char LOG_TABLE[]=
		{0x000,0x001,0x003,0x004,0x006,0x007,0x009,0x00A,
         0x00B,0x00D,0x00E,0x010,0x011,0x012,0x014,0x015,
         0x016,0x018,0x019,0x01A,0x01C,0x01D,0x01E,0x020,
         0x021,0x022,0x024,0x025,0x026,0x028,0x029,0x02A,
         0x02C,0x02D,0x02E,0x02F,0x031,0x032,0x033,0x034,
         0x036,0x037,0x038,0x039,0x03B,0x03C,0x03D,0x03E,
         0x03F,0x041,0x042,0x043,0x044,0x045,0x047,0x048,
         0x049,0x04A,0x04B,0x04D,0x04E,0x04F,0x050,0x051,
         0x052,0x054,0x055,0x056,0x057,0x058,0x059,0x05A,
         0x05C,0x05D,0x05E,0x05F,0x060,0x061,0x062,0x063,
         0x064,0x066,0x067,0x068,0x069,0x06A,0x06B,0x06C,
         0x06D,0x06E,0x06F,0x070,0x071,0x072,0x074,0x075,
         0x076,0x077,0x078,0x079,0x07A,0x07B,0x07C,0x07D,
         0x07E,0x07F,0x080,0x081,0x082,0x083,0x084,0x085,
         0x086,0x087,0x088,0x089,0x08A,0x08B,0x08C,0x08D,
         0x08E,0x08F,0x090,0x091,0x092,0x093,0x094,0x095,
         0x096,0x097,0x098,0x099,0x09A,0x09B,0x09B,0x09C,
         0x09D,0x09E,0x09F,0x0A0,0x0A1,0x0A2,0x0A3,0x0A4,
         0x0A5,0x0A6,0x0A7,0x0A8,0x0A9,0x0A9,0x0AA,0x0AB,
         0x0AC,0x0AD,0x0AE,0x0AF,0x0B0,0x0B1,0x0B2,0x0B2,
         0x0B3,0x0B4,0x0B5,0x0B6,0x0B7,0x0B8,0x0B9,0x0B9,
         0x0BA,0x0BB,0x0BC,0x0BD,0x0BE,0x0BF,0x0C0,0x0C0,
         0x0C1,0x0C2,0x0C3,0x0C4,0x0C5,0x0C6,0x0C6,0x0C7,
         0x0C8,0x0C9,0x0CA,0x0CB,0x0CB,0x0CC,0x0CD,0x0CE,
         0x0CF,0x0D0,0x0D0,0x0D1,0x0D2,0x0D3,0x0D4,0x0D4,
         0x0D5,0x0D6,0x0D7,0x0D8,0x0D8,0x0D9,0x0DA,0x0DB,
         0x0DC,0x0DC,0x0DD,0x0DE,0x0DF,0x0E0,0x0E0,0x0E1,
         0x0E2,0x0E3,0x0E4,0x0E4,0x0E5,0x0E6,0x0E7,0x0E7,
         0x0E8,0x0E9,0x0EA,0x0EA,0x0EB,0x0EC,0x0ED,0x0EE,
         0x0EE,0x0EF,0x0F0,0x0F1,0x0F1,0x0F2,0x0F3,0x0F4,
         0x0F4,0x0F5,0x0F6,0x0F7,0x0F7,0x0F8,0x0F9,0x0F9,
         0x0FA,0x0FB,0x0FC,0x0FC,0x0FD,0x0FE,0x0FF,0x0FF};

const unsigned char EXP_TABLE[]=
		{0x000,0x001,0x001,0x002,0x003,0x003,0x004,0x005,
         0x006,0x006,0x007,0x008,0x008,0x009,0x00A,0x00B,
         0x00B,0x00C,0x00D,0x00E,0x00E,0x00F,0x010,0x010,
         0x011,0x012,0x013,0x013,0x014,0x015,0x016,0x016,
         0x017,0x018,0x019,0x019,0x01A,0x01B,0x01C,0x01D,
         0x01D,0x01E,0x01F,0x020,0x020,0x021,0x022,0x023,
         0x024,0x024,0x025,0x026,0x027,0x028,0x028,0x029,
         0x02A,0x02B,0x02C,0x02C,0x02D,0x02E,0x02F,0x030,
         0x030,0x031,0x032,0x033,0x034,0x035,0x035,0x036,
         0x037,0x038,0x039,0x03A,0x03A,0x03B,0x03C,0x03D,
         0x03E,0x03F,0x040,0x041,0x041,0x042,0x043,0x044,
         0x045,0x046,0x047,0x048,0x048,0x049,0x04A,0x04B,
         0x04C,0x04D,0x04E,0x04F,0x050,0x051,0x051,0x052,
         0x053,0x054,0x055,0x056,0x057,0x058,0x059,0x05A,
         0x05B,0x05C,0x05D,0x05E,0x05E,0x05F,0x060,0x061,
         0x062,0x063,0x064,0x065,0x066,0x067,0x068,0x069,
         0x06A,0x06B,0x06C,0x06D,0x06E,0x06F,0x070,0x071,
         0x072,0x073,0x074,0x075,0x076,0x077,0x078,0x079,
         0x07A,0x07B,0x07C,0x07D,0x07E,0x07F,0x080,0x081,
         0x082,0x083,0x084,0x085,0x087,0x088,0x089,0x08A,
         0x08B,0x08C,0x08D,0x08E,0x08F,0x090,0x091,0x092,
         0x093,0x095,0x096,0x097,0x098,0x099,0x09A,0x09B,
         0x09C,0x09D,0x09F,0x0A0,0x0A1,0x0A2,0x0A3,0x0A4,
         0x0A5,0x0A6,0x0A8,0x0A9,0x0AA,0x0AB,0x0AC,0x0AD,
         0x0AF,0x0B0,0x0B1,0x0B2,0x0B3,0x0B4,0x0B6,0x0B7,
         0x0B8,0x0B9,0x0BA,0x0BC,0x0BD,0x0BE,0x0BF,0x0C0,
         0x0C2,0x0C3,0x0C4,0x0C5,0x0C6,0x0C8,0x0C9,0x0CA,
         0x0CB,0x0CD,0x0CE,0x0CF,0x0D0,0x0D2,0x0D3,0x0D4,
         0x0D6,0x0D7,0x0D8,0x0D9,0x0DB,0x0DC,0x0DD,0x0DE,
         0x0E0,0x0E1,0x0E2,0x0E4,0x0E5,0x0E6,0x0E8,0x0E9,
         0x0EA,0x0EC,0x0ED,0x0EE,0x0F0,0x0F1,0x0F2,0x0F4,
         0x0F5,0x0F6,0x0F8,0x0F9,0x0FA,0x0FC,0x0FD,0x0FF};

const FP FPMul(const FP first, const FP second)
{
	printf("fpmul fps %x fv %x sps %x sv %x\n",first.power_sign, first.value, second.power_sign, second.value);
	const unsigned long power_sign = LOG_TABLE[second.value] + LOG_TABLE[first.power_sign];
	const unsigned long value = (power_sign >255) ? second.power_sign +4 : second.power_sign;
	return SetFP( EXP_TABLE[power_sign &255], ( (value &255) +first.value) &0xfd);
}

const FP FPDiv(const FP first, const FP second)
{
	printf("fpdiv fps %x fv %x sps %x sv %x\n",first.power_sign, first.value, second.power_sign, second.value);
	const long power_sign = LOG_TABLE[second.power_sign] - LOG_TABLE[first.power_sign];
	const long value = (power_sign <0) ? (second.value | 2) -4 : second.value | 2;
	return SetFP( EXP_TABLE[power_sign &255], ( (value &255) -first.value) &0xfd);
}

/*
; in
; A = first value
; FP_MULTIPLY_POWER_SIGN = first  power sign
; X = second value
; Y = SECOND power sign
; out
; Y = power sign
; A = value

FP_MULTIPLY	STA		FP_MULTIPLY_VALUE 		; store first value
			LDA 	LOG_TABLE,X				; get second value
			LDX 	FP_MULTIPLY_POWER_SIGN	; get first power sign
			CLC 
			ADC 	LOG_TABLE,X				; second value + first power sign ?
			TAX 							; store in x
			TYA 							; get second power sign
			LDY 	EXP_TABLE,X				; get ouput power sign
			BCC 	_FP_MUL_CNT
					ADC 	#$03			; add 4 (00000011) + (00000001) = 
					CLC 
_FP_MUL_CNT	ADC 	FP_MULTIPLY_VALUE		; add first value
			AND 	#$FD					; get output value
			RTS 
*/

unsigned char A;
unsigned char X;
unsigned char Y;
unsigned char C;
unsigned char S;
unsigned char Z;
unsigned char V;
unsigned char ab08;
unsigned char ab09;
unsigned char ab18;
unsigned char ab06;
const unsigned char v01 = 1;
const unsigned char v09 = 0x09;
const unsigned char v0f = 0x0f;
const unsigned char v00 = 0x00;
const unsigned char vff = 0xff;
const unsigned char v24 = 0x24;
const unsigned char ve0 = 0xe0;
const unsigned char v07 = 0x07;
unsigned char ab83AB;
unsigned char ab8408;

const void SetFlags(const unsigned char number)
{
// #ifdef DBG 
// printf("SF-I number %x / ",number);
// #endif
	S=0;
	if (number >127) // GT
	{
		S=1;
	}
	Z=0;
	if (number ==0)
	{
		Z=1;
	}
// #ifdef DBG 
// printf("SF-O S %x Z %x / ",S,Z);
// #endif
}

const void TYA()
{
 #ifdef DBG 
 printf("TYA-I A %x Y %x / ",A,Y);
 #endif
	A = Y;
	SetFlags(A);
 #ifdef DBG 
 printf("TYA-O A %x Y %x\n",A,Y);
 #endif
}

const void LSR()
{
 #ifdef DBG 
 printf("LSR-I A %x / ",A); 
 #endif
	C = A &1;
	A = A >>1;
	SetFlags(A);
 #ifdef DBG 
 printf("LSR-O A %x C %x\n",A,C);
 #endif
}

const void ROR()
{
 #ifdef DBG 
 printf("ROR-I A %x C %x / ",A,C);
 #endif
	const unsigned long tempC= C<<7;
	C = A &1;
	A = A >>1;
	A = A | tempC;
	SetFlags(A);
 #ifdef DBG 
 printf("ROR-O A %x C %x\n",A,C);
 #endif
}

const void TXA()
{
 #ifdef DBG 
 printf("TXA-I A %x X %x / ",A,X);
 #endif
	A = X;
	SetFlags(X);
 #ifdef DBG 
 printf("TXA-O A %x X %x\n",A,X);
 #endif
}

const void TAX()
{
 #ifdef DBG 
 printf("TAX-I A %x X %x / ",A,X);
 #endif
	X = A;
	SetFlags(A);
 #ifdef DBG 
 printf("TAX-O A %x X %x\n",A,X);
 #endif
}

const void TAY()
{
 #ifdef DBG 
 printf("TAY-I A %x Y %x / ",A,Y);
 #endif
	Y = A;
	SetFlags(Y);
 #ifdef DBG 
 printf("TAY-O A %x Y %x\n",A,Y);
 #endif
}

const void SEC()
{
	C =1;
// #ifdef DBG 
// printf("SEC-IO C %x / ",C);
// #endif
}

const void CLC()
{
	C =0;
// #ifdef DBG 
// printf("CLC-IO C %x\n",C);
// #endif
}

const void NOP()
{
 #ifdef DBG 
 printf("NOP\n");
 #endif
}

const void INY()
{
 #ifdef DBG 
 printf("INY-I Y %x / ",Y);
 #endif
	++Y;
	Y = Y &255;
	SetFlags(Y);
 #ifdef DBG 
 printf("INY-O Y %x\n",Y);
 #endif
}

const void DEY()
{
 #ifdef DBG 
 printf("DEY-I Y %x / ",Y);
 #endif
	--Y;
	Y = Y &255;
	SetFlags(Y);
 #ifdef DBG 
 printf("DEY-O Y %x / ",Y);
 #endif
}

const void SetFlagV(const long oldA, const long newA)
{
	V=0;
	if ( (oldA <0) && (newA >0) )
	{
		V=1;
		return;
	}
	if ( (oldA >0) && (newA <0) )
	{
		V=1;
		return;
	}
// #ifdef DBG 
// printf("SetFlagV-IO OA %x NA %x V %x / ",oldA,newA,V);
// #endif
}

const void SBC(const unsigned char number)
{
 #ifdef DBG 
 printf("SBC-I number %x A %x C %x / ",number,A,C);
 #endif
	if (C==0)
	{
		SEC();
	}
	else 
	{
		CLC();
	}
	const long oldA =A;
	const long tempA = A -number -C;
	SEC();
	A = tempA;
//	printf("\ntempA %i\n",tempA);
	if (tempA <0) // LT
	{
//		printf("\ntempA wraparound %i\n",tempA);
		A = A +256;
		CLC();
	}
	SetFlags(A);
	SetFlagV(oldA, A);
 #ifdef DBG 
 printf("SBC-O A %x C %x\n",A,C);
 #endif
}

const void ADC(const unsigned char number)
{
 #ifdef DBG 
 printf("ADC-I number %x A %x C %x / ",number,A,C);
 #endif
	const long oldA =A;
	const long tempA = A +number +C;
	CLC();
	A = tempA;
	if (tempA >255) // GT
	{
		A = A -256;
		SEC();
	}
	SetFlags(A);
	SetFlagV(oldA, A);
 #ifdef DBG 
 printf("ADC-O A %x C %x\n",A,C);
 #endif
}

const void CMP(const unsigned char number)
{
 #ifdef DBG 
 printf("CMP-I number %x A %x / ",number,A);
 #endif
	Z=0;
	CLC();
	if (number==A)
	{
		Z=1;
 #ifdef DBG 
 printf("CMP-O ZS C %x S %x Z %x\n",C,S,Z);
 #endif
		return;
	}
	if (A >number) // GT
	{
		SEC();
 #ifdef DBG 
 printf("CMP-O CS C %x S %x Z %x\n",C,S,Z);
 #endif
		return;
	}
 #ifdef DBG 
 printf("CMP-O C %x S %x Z %x\n",C,S,Z);
 #endif
}

const void STA(unsigned char *number)
{
 #ifdef DBG 
 printf("STA-I A %x / ",A);
 #endif
	*number = A;
	SetFlags(A);
 #ifdef DBG 
 printf("STA-O A %x\n",A);
 #endif
}

const void STX(unsigned char *number)
{
 #ifdef DBG 
 printf("STX-I X %x / ",X);
 #endif
	*number = X;
	SetFlags(X);
 #ifdef DBG 
 printf("STX-O X %x\n",X);
 #endif
}

const void STY(unsigned char *number)
{
 #ifdef DBG 
 printf("STY-I Y %x / ",Y);
 #endif
	*number = Y;
	SetFlags(Y);
 #ifdef DBG 
 printf("STY-O Y %x\n",Y);
 #endif
}

const void LDA(const unsigned char number)
{
 #ifdef DBG 
 printf("LDA-I number %x A %x / ",number,A);
 #endif
	A = number;
	SetFlags(A);
 #ifdef DBG 
 printf("LDA-O number %x A %x\n",number,A);
 #endif
}

const void LDY(const unsigned char number)
{
 #ifdef DBG 
 printf("LDY-I number %x Y %x / ",number,Y);
 #endif
	Y = number;
	SetFlags(Y);
 #ifdef DBG 
 printf("LDY-O number %x Y %x\n",number,Y);
 #endif
}

const void LDX(const unsigned char number)
{
 #ifdef DBG 
 printf("LDX-I number %x X %x / ",number,X);
 #endif
	X = number;
	SetFlags(X);
 #ifdef DBG 
 printf("LDX-O number %x X %x\n",number,X);
 #endif
}

const void EOR(const unsigned char number)
{
 #ifdef DBG 
 printf("EOR-I A %x / ",A);
 #endif
	A= A^number;
	SetFlags(A);
 #ifdef DBG 
 printf("EOR-O A %x\n",A);
 #endif
}

const void AND(const unsigned char number)
{
 #ifdef DBG 
 printf("AND-I A %x / ",A);
 #endif
	A= A&number;
	SetFlags(A);
 #ifdef DBG 
 printf("AND-O A %x\n",A);
 #endif
}

const void ASL()
{
 #ifdef DBG 
 printf("ASL-I A %x C %x / ",A,C);
 #endif
	CLC();
	if (A >127) // GT
	{
		SEC();
	}
	A= A+A;
	SetFlags(A);
 #ifdef DBG 
 printf("ASL-O A %x C %x\n",A,C);
 #endif
}

const FP FPAdd(const FP first, const FP second)
{
	ab08=first.value;
	ab09=first.power_sign;
	X=second.value;
	Y=second.power_sign;
	
	printf("t0 ab08 %x ab09 %x X %x Y %x\n",ab08,ab09,X,Y);
	goto FPAddStart;
	
FPADD_SAME_VALUE:
			LDA(ab18); // ($838B entry point)
			LSR(); 
			if (C!=0) //			BCS 	FPADD_CONT2 - same value but negative
			{
				#ifdef DBG 
				printf("BCS 	FPADD_CONT2\n");
				#endif
				goto	FPADD_SAME_VALUE_NEG;
			}				
        TXA(); 
        ADC(ab08);
        ROR();
        INY();	// is add 4 to y the same as lsr #2 ?
        INY();
        INY();
        INY(); 
        TAX(); 			// redundant?
        STX(&ab08);
        STY(&ab09);
//		printf("test1 %x %x\n",ab09,ab08);
		return SetFP(ab09,ab08); // RTS 

FP_ADD_ERR:
			TYA(); 
			ASL(); 
			if (C==0) goto FP_ADD_ERR_1ST;		//			BCC 	FP_ADD_ERR_1ST
			goto	FP_ADD_ERR_2ND;				//				JMP 	FP_ADD_ERR_2ND

FPADD_SAME_VALUE_NEG:
		TXA(); 
        SBC(ab08);
			if (Z!=0) 					 		//        BEQ 	FPADD_CONT3
			{
				#ifdef DBG 
				printf("BEQ 	FPADD_CONT3\n");
				#endif
				goto	FPADD_CONT3;
			}
			if (C!=0)  							//			BCS 	FPADD_CONT6
			{
				#ifdef DBG 
				printf("BCS 	FPADD_CONT6\n");
				#endif
				goto FPADD_CONT6;
			}
			LDY(ab09);
			EOR(vff);
			ADC(v01);
			#ifdef DBG 
			printf("JMP 	FPADD_CONT6\n");
			#endif
			goto FPADD_CONT6;				//			JMP 	FPADD_CONT6

FPADD_CONT3:
				TYA(); 
				SEC(); 
				SBC(v24);
//				if (V!=0) goto _MATH_LIMIT_POS;		//				BVS 	_MATH_LIMIT_POS
//				if (V==0) 
				goto  MATH_LIMIT_EXIT; 	//					BVC 	MATH_LIMIT_EXIT ; jmp

FPAddStart:		TYA();						// transfer num1_hi to a

				#ifdef DBG 
				printf("test1 A %x X %x Y %x\n",A,X,Y);
				#endif					

				SEC();						// set carry
				SBC(ab09);					// sub num2_hi from a
				#ifdef DBG 
				printf("test2 A %x X %x Y %x\n",A,X,Y);
				#endif					

//				if (V!=0) goto FP_ADD_ERR;	//        BVS 	FP_ADD_ERR	// overflow so exit
				STA(&ab18);					// ab18 = num1_hi - num2_hi
				ADC(v01);					// add 1 + carry?
				#ifdef DBG 
				printf("test3 A %x X %x Y %x\n",A,X,Y);
				#endif					

//				if (V!=0) goto FP_ADD_ERR;	//        BVS 	FP_ADD_ERR	// overflow so exit
				#ifdef DBG 
				printf("test4 A %x X %x Y %x\n",A,X,Y);
				#endif					

				if (S!=0)
				{
					#ifdef DBG 
					printf("BMI FPADD_CONT8\n");
					#endif					
					goto FPADD_CONT8;	//        BMI 	FPADD_CONT8		; if result minus - start - end = negative?
				}
				LSR(); 						// a divide by 2 - remove bit 0 (sign)
				LSR(); 						// a divide by 2 - remove bit 1 
				if (Z!=0)
				{
					#ifdef DBG 
					printf("BEQ FPADD_SAME_VALUE\n");
					#endif					
					goto FPADD_SAME_VALUE;		//        BEQ 	FPADD_CONT	// if 0 then the number is the value with the sign removed is the same
				}
				CMP(v09);						// gt 9
				if (C!=0) goto FP_ADD_ERR_1ST;	//        BCS 	FP_ADD_ERR_1ST		; yes so exit
        EOR(v0f);			// reverse 15
		STA(&ab83AB);	//        STA 	ab83AB +1	; jump forward that far
        LDA(ab08);			// get num_hi
        SEC();				// set carry
        ROR ();				// a divide by 2 and move carry into top bit

//	printf("test 1 a %x x %x y %x\n",A,X,Y);
	#ifdef DBG 
	printf("BNE		b83B3 %x %i\n",ab83AB,ab83AB);
	#endif					
	switch (ab83AB)			//ab83AB: 	BNE		b83B3 	; self modifiction code shift
	{
		default:
		case 0:
	#ifdef DBG 
	printf("case 0 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 1:
	#ifdef DBG 
	printf("case 1 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 2:
	#ifdef DBG 
	printf("case 2 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 3:
	#ifdef DBG 
	printf("case 3 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 4:
	#ifdef DBG 
	printf("case 4 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 5:
	#ifdef DBG 
	printf("case 5 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 6:
	#ifdef DBG 
	printf("case 6 %x %i\n",ab83AB,ab83AB);
	#endif					
			NOP(); 				// NA
		case 7:
	#ifdef DBG 
	printf("case 7 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR(); 				// divide by 2
		case 8:
	#ifdef DBG 
	printf("case 8 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR(); 				// divide by 2
		case 9:
	#ifdef DBG 
	printf("case 9 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR(); 				// divide by 2
		case 10:
	#ifdef DBG 
	printf("case 10 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR(); 				// divide by 2
		case 11:
	#ifdef DBG 
	printf("case 11 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR(); 				// divide by 2
		case 12:
	#ifdef DBG 
	printf("case 12 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR();  			// divide by 2
		case 13:
	#ifdef DBG 
	printf("case 13 %x %i\n",ab83AB,ab83AB);
	#endif					
			LSR();  			// divide by 2
		case 14:
	#ifdef DBG 
	printf("case 14 %x %i\n",ab83AB,ab83AB);
	#endif					
        STA (&ab06);		// 
		case 15:
	#ifdef DBG 
	printf("case 15 %x %i\n",ab83AB,ab83AB);
	#endif					
        LDA (ab18);			// 
	}
        LSR();				// divide by 2 - setting carry?
        TXA();				// transfer num1_lo to a
		if (C!=0) goto FPADD_CONT5; 		//        BCS 	FPADD_CONT5			; carry set
FPADD_CONT4:
		ADC(ab06);
		if (C==0) goto FPADD_CONT7; 		//			BCC 	FPADD_CONT7
				LSR(); 				// divide by 2
				INY(); 				// y++
				INY(); 				// y++
				INY(); 				// y++
				INY(); 				// y++
FPADD_CONT7:
				TAX();
FP_ADD_ERR_1ST: 
			STX(&ab08);
			STY(&ab09);
			printf("test3 %x %x\n",ab09,ab08);

			return SetFP(ab09,ab08); // RTS 

FPADD_CONT5:
			SBC(ab06);
			if (C!=0) goto FPADD_CONT7; 		//			BCS FPADD_CONT7
FPADD_CONT6:
			STY(&ab06);
			LDY(v00);
FPADD_LOOP1:
				ASL(); 
				DEY(); 
				if (C==0) goto FPADD_LOOP1;		//				BCC		FPADD_LOOP1
        TAX(); 
        TYA(); 
        ASL(); 
        ASL(); 
        CLC(); 
        ADC(ab06);
//        BVS 	FPADD_ERR3
			TAY(); 
			STX(&ab08);
			STY(&ab09);
			return SetFP(ab09,ab08); //RTS 
			
FPADD_CONT8:
			CMP(ve0);
			if (C==0) goto FP_ADD_ERR_2ND;		//			BCC 	FP_ADD_ERR_2ND
        LSR(); 
        LSR(); 
        AND(v07);
		STA(&ab8408);	//        STA FP_ADD_JMP2 +1
		TXA(); 
        SEC(); 
        ROR();
		
	switch (ab8408)			////	BNE b8408	; self modifiction code shift
	{
		default:
		case 0:
			LSR(); 
		case 1:
			LSR(); 
		case 2:
			LSR(); 
		case 3:
			LSR(); 
		case 4:
			LSR(); 
		case 5:
			LSR(); 
		case 6:
			LSR(); 
		case 7:
        STA(&ab06);
	}		
//        STA(&ab06);
        LDY(ab09);
        LDA(ab18);
        LSR(); 
        LDA(ab08);
			if (C==0) goto FPADD_CONT4;		//        BCC FPADD_CONT4
			if (C!=0) 
				goto FPADD_CONT5;		//        BCS FPADD_CONT5	; jmp
		
FP_ADD_ERR_2ND:
				LDX(ab08);
				LDY(ab09);
				printf("new path - result is %x %x\n",ab08,ab09);
				return;	 // RTS 

MATH_LIMIT_EXIT: 
				TAY();
				LDX(v00);
				STX(&ab08);
				STY(&ab09);
				return SetFP(ab09,ab08); //RTS 

//switch (jmp1)
//{
//	case 0:
//	case 1:
//	case 2:
//	case 3:
//	case 4:
//	case 5:
//	case 6:
//	case 7:
//	case 8:
//	case 9:
//	case 10:
//	case 11:
//	case 12:
//	case 13:
//	case 14:
//	case 15:
//	default:
//}	

}

const int inc =123;
const long end = ((128*256)-1);
const long start = -((128*256)-1);

int main(int argc, char *argv[])
{
	unsigned long num1, num2;
	scanf("%x",&num1);
	scanf("%x",&num2);
	const FP rc5 = Bit24ToFP(num1);
	const FP rc6 = Bit24ToFP(num2);
	printf("num1 %x\nbit24tofp ps %x v %x\nfptobit8 %x\nfptobit16 %x\n",num1,rc5.power_sign,rc5.value,FPToBit8(rc5),FPToBit16(rc5));
	printf("num2 %x\nbit24tofp ps %x v %x\nfptobit8 %x\nfptobit16 %x\n",num2,rc6.power_sign,rc6.value,FPToBit8(rc6),FPToBit16(rc6));
	const FP rc7 = FPAdd(rc5,rc6);
	printf("fpadd ps %x v %x\n",rc7.power_sign,rc7.value);

	const FP rcmul=FPMul(rc5,rc6);
	printf("fpmul ps %x v %x\n",rcmul.power_sign,rcmul.value);
	const FP rcdiv1=FPDiv(rc5,rcmul);
	printf("fpdiv1 ps %x v %x\n",rcdiv1.power_sign,rcdiv1.value);

	return;
	

	scanf("%x",&num1);
////	scanf("%x",&num2);

	const FP rc3 = Bit24ToFP(num1);
	printf("num3 %x\nbit24tofp ps %x v %x\nfptobit8 %x\nfptobit16 %x\n",num1,rc3.power_sign,rc3.value,FPToBit8(rc3),FPToBit16(rc3));
////	const FP rc4 = Bit24ToFP(num2);
////	printf("num2 %x\nbit24tofp ps %x v %x\nfptobit8 %x\nfptobit16 %x\n",num2,rc4.power_sign,rc4.value,FPToBit8(rc4),FPToBit16(rc4));
////	const FP rc5 = FPAdd(rc3,rc4);
////	printf("fpadd %x %x\n",rc5.power_sign,rc5.value);
////	const FP rcmul=FPMul(rc3,rc4);
////	printf("fpmul ps %x v %x\n",rcmul.power_sign,rcmul.value);
////	const FP rcdiv1=FPDiv(rc3,rcmul);
////	printf("fpdiv1 ps %x v %x\n",rcdiv1.power_sign,rcdiv1.value);
//	const FP rcdiv2=FPDiv(rc4,rcmul);
//	printf("fpdiv2 ps %x v %x\n",rcdiv2.power_sign,rcdiv2.value);
	return;

    FILE *outfile = fopen("temp.txt", "w");

	fprintf(outfile,"num1,num2,");
	fprintf(outfile,"num1 bit24tofp ps,v,");
	fprintf(outfile,"num1 fptobit8,num1 fptobit16,");
	fprintf(outfile,"num2 bit24tofp ps,v,");
	fprintf(outfile,"num2 fptobit8,num2 fptobit16,");
	fprintf(outfile,"fpadd ps,v,");
	fprintf(outfile,"fpmul ps,v,");
	fprintf(outfile,"fpdiv ps,v\n");

	for (long num1=start; num1<end; num1+=inc)
	{
		for (long num2=start; num2<end; num2+=inc)
		{
			fprintf(outfile,"%x, %x,",num1,num2);

	const FP rc3 = Bit24ToFP(num1);
	fprintf(outfile,"%x,%x,",rc3.power_sign,rc3.value);
	fprintf(outfile,"%x,%x,",FPToBit8(rc3),FPToBit16(rc3));
	const FP rc4 = Bit24ToFP(num2);
	fprintf(outfile,"%x,%x,",rc4.power_sign,rc4.value);
	fprintf(outfile,"%x,%x,",FPToBit8(rc4),FPToBit16(rc4));
	const FP rc5 = FPAdd(rc3,rc4);
	fprintf(outfile,"%x,%x,",rc5.power_sign,rc5.value);
	const FP rcmul=FPMul(rc3,rc4);
	fprintf(outfile,"%x,%x,",rcmul.power_sign,rcmul.value);
	const FP rcdiv1=FPDiv(rc3,rcmul);
	fprintf(outfile,"%x,%x\n",rcdiv1.power_sign,rcdiv1.value);

//	const FP rc3 = Bit24ToFP(num1);
//	fprintf(outfile,"num3, %x, bit24tofp ps, %x, v, %x, fptobit8, %x, fptobit16, %x, ",num1,rc3.power_sign,rc3.value,FPToBit8(rc3),FPToBit16(rc3));
//	const FP rc4 = Bit24ToFP(num2);
//	fprintf(outfile,"num2, %x, bit24tofp ps, %x, v, %x, fptobit8, %x, fptobit16, %x, ",num2,rc4.power_sign,rc4.value,FPToBit8(rc4),FPToBit16(rc4));
//	const FP rc5 = FPAdd(rc3,rc4);
//	fprintf(outfile,"fpadd, %x, %x, ",rc5.power_sign,rc5.value);
//	const FP rcmul=FPMul(rc3,rc4);
//	fprintf(outfile,"fpmul ps, %x, v, %x, ",rcmul.power_sign,rcmul.value);
//	const FP rcdiv1=FPDiv(rc3,rcmul);
//	fprintf(outfile,"fpdiv1 ps, %x, v, %x\n",rcdiv1.power_sign,rcdiv1.value);
			
//			printf("x %x y %x\n",x,y);
		}
	}

    fclose(outfile);
	return;
	
//	for (long x=-((256*256*128)-1); x<((256*256*128)-1); x+=inc)
//	{
//		for (long y=-((256*256*128)-1); y<((256*256*128)-1); y+=inc)
//		{
//			printf("x %x y %x\n",x,y);
//		}
//	}
	return;


	for (long num1=start; num1<end; num1+=inc)
	{
		for (long num2=start; num2<end; num2+=inc)
		{
			printf("num1 %x num2 %x\n",num1,num2);
	const FP rc1 = Bit24ToFP(num1);
	printf("num1 %x bit24tofp %x %x fptobit8 %x fptobit16 %x\n",num1,rc1.power_sign,rc1.value,FPToBit8(rc1),FPToBit16(rc1));
//
	const FP rc2 = Bit24ToFP(num2);
	printf("num2 %x bit24tofp %x %x fptobit8 %x fptobit16 %x\n",num2,rc2.power_sign,rc2.value,FPToBit8(rc2),FPToBit16(rc2));

	const FP rcadd = FPAdd(rc1,rc2);
//	printf("fpadd %x %x\n",rcadd.power_sign,rcadd.value);
			
//			printf("x %x y %x\n",x,y);
		}
	}
	return;
	
//	for (long x=-((256*256*128)-1); x<((256*256*128)-1); x+=inc)
//	{
//		for (long y=-((256*256*128)-1); y<((256*256*128)-1); y+=inc)
//		{
//			printf("x %x y %x\n",x,y);
//		}
//	}
	return;
	
	
//	scanf("%x",&num1);
//	scanf("%x",&num2);
//	const FP rc1 = Bit24ToFP(num1);
//	const FP rc2 = Bit24ToFP(num2);
//	const FP rcadd = FPAdd(rc1,rc2);
	
//	printf("num1 %x bit24tofp %x %x fptobit8 %x fptobit16 %x\n",num1,rc1.power_sign,rc1.value,FPToBit8(rc1),FPToBit16(rc1));
//	printf("num2 %x bit24tofp %x %x fptobit8 %x fptobit16 %x\n",num2,rc2.power_sign,rc2.value,FPToBit8(rc2),FPToBit16(rc2));
//	printf("fpadd %x %x\n",rcadd.power_sign,rcadd.value);
	
//	long a = (256*256*256)-1;
//	long b = -a;
//	long test1 =-123456;
//	unsigned long test2=test1;
//	printf("t1 %x t2 %x a %x b %x\n",test1,test2,a,b);
	
//	const FP rcmul=FPMul(rc1,rc2);
//	printf("fpmul %x %x\n",rcmul.power_sign,rcmul.value);
//	const FP rcdiv=FPDiv(rcmul,rc2);	
//	printf("fpdiv %x %x\n",rcdiv.power_sign,rcdiv.value);
}
 // 7fffff ff800001
/*
switch (jmp1)
{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	default:
}	
	
case  1 || 2:
    cout << "You're not a wizard larry";
    break;
default:
    cout << "You're a wizard harry";
}
*/

/*
FPADD_CONT	LDA		ab18	; ($838B entry point)  
			LSR 
			BCS 	FPADD_CONT2
        TXA 
        ADC 	ab08
        ROR 
        INY 
        INY 
        INY 
        INY 
        TAX 			; redundant?
        STA 	ab08
        STY 	ab09
        RTS 

PC_ADD_ERR	TYA 
			ASL 
			BCC 	FP_ADD_ERR_1ST
				JMP 	FP_ADD_ERR_2ND
FPADD_CONT2
		TXA 
        SBC 	ab08
        BEQ 	FPADD_CONT3
			BCS 	FPADD_CONT6
			LDY 	ab09
			EOR 	#$FF
			ADC 	#$01
			JMP 	FPADD_CONT6

FPADD_CONT3		TYA 
				SEC 
				SBC 	#$24
				BVS 	_MATH_LIMIT_POS
					BVC 	MATH_LIMIT_EXIT
	
FP_ADD  TYA			; XY + 89	; transfer num1_hi to a
        SEC 					; set carry
        SBC 	ab09			; sub num2_hi from a
        BVS 	PC_ADD_ERR		; overflow so exit
        STA 	ab18			; ab18 = num1_hi - num2_hi
        ADC 	#$01			; add 1 + carry?
        BVS 	PC_ADD_ERR		; overflow so exit
        BMI 	FPADD_CONT8			
        LSR 					; divide by 2
        LSR 					; divide by 2
        BEQ 	FPADD_CONT		; if 0
        CMP 	#$09			; gt 9
        BCS 	FP_ADD_ERR_1ST	; yes
        EOR 	#$0F			; reverse 
        STA 	FP_ADD_JMP1 +1		; jump forward that amount
        LDA		ab08			; get num_hi
        SEC						; set carry 
        ROR 					; /2 and move carry into first bit

FP_ADD_JMP1 	BNE		b83B3 	; self modifiction code shift
        NOP 					; NA
        NOP 					; NA
        NOP 					; NA
        NOP 					; NA
        NOP 					; NA
        NOP 					; NA
        NOP 					; NA
b83B3   LSR 					; divide by 2
        LSR  					; divide by 2
        LSR  					; divide by 2
        LSR  					; divide by 2
        LSR  					; divide by 2
        LSR  					; divide by 2
        LSR  					; divide by 2
        STA 	ab06			; 
        LDA 	ab18
        LSR 					; divide by 2 - setting carry?
        TXA 					; transfer num1_lo to a
        BCS 	FPADD_CONT5			; carry set
FPADD_CONT4	ADC 	ab06
			BCC 	FPADD_CONT7
				LSR 				; divide by 2
				INY 				; y++
				INY 				; y++
				INY 				; y++
				INY 				; y++
FPADD_CONT7   	TAX
FP_ADD_ERR_1ST 
			STX 	ab08
			STY 	ab09
			RTS 

FPADD_CONT5	SBC ab06
			BCS FPADD_CONT7
FPADD_CONT6	STY ab06
        LDY #$00
FPADD_LOOP1		ASL 
				DEY 
				BCC		FPADD_LOOP1
        TAX 
        TYA 
        ASL 
        ASL 
        CLC 
        ADC 	ab06
        BVS 	FPADD_ERR3
			TAY 
			STX 	ab08
			STY 	ab09
			RTS 

FPADD_ERR3	LDX 	#<8400
			LDY 	#>8400
			STX 	ab08
			STY		ab09
			RTS 

FPADD_CONT8	CMP 	#$E0
			BCC 	FP_ADD_ERR_2ND
        LSR 
        LSR 
        AND #$07
        STA FP_ADD_JMP2 +1
		TXA 
        SEC 
        ROR
FP_ADD_JMP2	BNE b8408
        LSR 
        LSR 
        LSR 
b8408   LSR 
        LSR 
        LSR 
        LSR 
        STA ab06
        LDY ab09
        LDA ab18
        LSR 
        LDA ab08
        BCC FPADD_CONT4
        BCS FPADD_CONT5	; jmp
		
FP_ADD_ERR_2ND	LDX		ab08
				LDY 	ab09
				RTS 
*/

/*
D:\Temp\C\tcc\examples>floatpointv10
ffff807c
7f9d
num3 ffff807c bit24tofp 39 fe fptobit8 0 fptobit16 807f
num2 7f9d bit24tofp 38 fe fptobit8 0 fptobit16 7f80
t0 ab08 fe ab09 39 X fe Y 38
*/



