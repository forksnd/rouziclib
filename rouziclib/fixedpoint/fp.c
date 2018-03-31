/*
	Summary of the main trigonometry functions:

			format	format	new	float	float
			in	out	speed	speed	equivalent
			----------------------------------------------------------------------------------------------------------
	isqrt(x)	u64	u32	~15.5	10-65	sqrt(x)
	fplog2(x)	32.0	s5.26	31.33	66	log2(x), fplog2(0) returns 0 instead of a NaN
	fpexp2(x)	s5.26	32.0	12.66	142	exp2(x) or 2^x, returns 0 is x is negative as if it was 2^-inf
	fppow(x,y,fmt)	?,s5.26	.fmt	59	3.6-12k	pow(x, y) or x^y, x > 0 and in a custom (32-fmt).fmt format
	fpcos(x)	0.32	s1.30	13	94	cos(2.*pi*x), input unit is in turns, not radians. fpsin() takes one extra cycle
	fpwsinc(x)	s2+.24	s1.30	5?	?	Blackman-windowed sinc with a cutoff frequency of 0.5 and rolloff bandwidth of 0.5
	fpatan2(y, x)	s31,s31	s0.32	62	155	atan2(y, x)/2pi (output unit is turns, not radians), fpatan2(y, x) = [-0.5, +0.5[
	fpgauss(x)	s3+.16	0.30	5?	144?	exp(-x*x) for x = [-4.0, 4.0], returns 0.0 outside of that range
	fperfr(x)	s3+.16	0.30	5?	?	0.5erf(x)+0.5 for x = [-4.0, 4.0], returns 0.0 or 1.0 outside of that range
 */

// accurate, ~1.5 cycles instead of ~5, sometimes the compiler already does this
int32_t fastabs(int32_t x)
{
	int32_t y = x >> 31;
	return (x^y) - y;
}

int64_t fastabs64(int64_t x)
{
	int32_t y = x >> 63;
	return (x^y) - y;
}

#ifndef RL_EXCL_APPROX

#include "ffo_lut.h"

int32_t log2_ffo32(uint32_t x)	// returns the number of bits up to the most significant set bit so that 2^return > x >= 2^(return-1)
{
	/*
	   Max iterations vs table size:
	   4 iterations,  8-bit index (0.25 kB)
	   3 iterations, 11-bit index (2 kB)	<---
	   2 iterations, 16-bit index (64 kB)
	*/
	int32_t y;

	y = x>>21;	if (y)	return ffo_lut[y]+21;
	y = x>>10;	if (y)	return ffo_lut[y]+10;
	return ffo_lut[x];
}

int32_t log2_ffo64(uint64_t x)	// returns the number of bits up to the most significant set bit so that 2^return > x >= 2^(return-1)
{
	/*
	   Max iterations vs table size:
	   8 iterations,  8-bit index (0.25 kB)
	   7 iterations, 10-bit index (1 kB)
	   6 iterations, 11-bit index (2 kB)	<---
	   5 iterations, 13-bit index (8 kB)
	   4 iterations, 16-bit index (64 kB)
	*/
	int32_t y;

	y = x>>53;	if (y)	return ffo_lut[y]+53;
	y = x>>42;	if (y)	return ffo_lut[y]+42;
	y = x>>31;	if (y)	return ffo_lut[y]+31;
	y = x>>20;	if (y)	return ffo_lut[y]+20;
	y = x>>9;	if (y)	return ffo_lut[y]+9;
	return ffo_lut[x];
}

//#if defined(__x86_64__) || defined(_M_X64)
#if (1==0)

// compute mul_wide (a, b) >> s, for s in [0,63]
int64_t mulshift (int64_t a, int64_t b, int s)
{
	int64_t res;
	__asm__ volatile (
			"movq  %1, %%rax;\n\t"          // rax = a
			"movq  %2, %%rdx;\n\t"          // rdx = b  
			"movl  %3, %%ecx;\n\t"          // ecx = s
			"imulq %%rdx;\n\t"              // rdx:rax = rax * rdx
			"shrdq %%cl, %%rdx, %%rax;\n\t" // rax = int64_t (rdx:rax >> s)
			"movq  %%rax, %0;\n\t"          // res = rax
			: "=rm" (res)
			: "rm"(a), "rm"(b), "rm"(s));
	return res;
}

#else

void umul64wide (uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
	uint64_t a_lo = (uint64_t)(uint32_t)a;
	uint64_t a_hi = a >> 32;
	uint64_t b_lo = (uint64_t)(uint32_t)b;
	uint64_t b_hi = b >> 32;

	uint64_t p0 = a_lo * b_lo;
	uint64_t p1 = a_lo * b_hi;
	uint64_t p2 = a_hi * b_lo;
	uint64_t p3 = a_hi * b_hi;

	uint32_t cy = (uint32_t)(((p0 >> 32) + (uint32_t)p1 + (uint32_t)p2) >> 32);

	*lo = p0 + (p1 << 32) + (p2 << 32);
	*hi = p3 + (p1 >> 32) + (p2 >> 32) + cy;
}

void mul64wide (int64_t a, int64_t b, int64_t *hi, int64_t *lo)
{
	umul64wide ((uint64_t)a, (uint64_t)b, (uint64_t *)hi, (uint64_t *)lo);
	if (a < 0LL) *hi -= b;
	if (b < 0LL) *hi -= a;
}

// compute mul_wide (a, b) >> s, for s in [0,63]
int64_t mulshift (int64_t a, int64_t b, int s)
{
	int64_t res;
	int64_t hi, lo;
	mul64wide (a, b, &hi, &lo);
	if (s) {
		res = ((uint64_t)hi << (64 - s)) | ((uint64_t)lo >> s);
	} else {
		res = lo;
	}
	return res;
}
#endif

// Usage note: for fixed point inputs make outfmt = desired format + format of x - format of y
// The caller must make sure not to divide by 0. Division by 0 causes a crash by negative index table lookup
int64_t fpdiv_d2(int32_t y, int32_t x, int32_t outfmt)	// ~24.3 cycles, max error (by division) 32.42e-9
{
	#include "fpdiv_d2.h"	// includes the quadratic coefficients LUT (1.5 kB) as generated by tablegen.exe, the format (prec=27) and LUT size power (lutsp)
	int32_t xa, xs, p, sh;
	uint32_t expon, frx, lutind;
	const uint32_t ish = prec-lutsp-1, cfs = 31-prec, half = 1L<<(prec-1);	// the shift for the index, the shift for 31-bit xa, the value of 0.5
	int64_t out;
	const int32_t *c;
	int32_t c0, c1, c2;

	// turn x into xa (|x|) and sign of x (xs)
	xs = x >> 31;
	xa = (x^xs) - xs;

	// decompose |x| into frx * 2^expon
	expon = log2_ffo32(xa);
	frx = (xa << (31-expon)) >> cfs;	// the fractional part is now in 0.27 format

	// lookup the 3 quadratic coefficients for c2*x^2 + c1*x + c0 then compute the result
	lutind = (frx - half) >> ish;		// range becomes [0, 2^26 - 1], in other words 0.26, then >> (26-lutsp) so the index is lutsp bits
	c = &fpdiv_lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	p = ((((int64_t) c2 * frx >> prec) + c1) * frx >> prec) + c0;	// does (c2*x + c1)*x + c0 instead of c2*x^2 + c1*x + c0

	// apply the necessary bit shifts and reapplies the original sign of x to make final result
	sh = expon + prec - outfmt;		// calculates the final needed shift
	out = (int64_t) y * p;			// format is s31 + 1.27 = s32.27
	if (sh >= 0)
		out >>= sh;
	else
		out <<= -sh;
	out = (out^xs) - xs;			// if x was negative then out is negated

	return out;
}

// (for size 10) accurate to either <= 10 integer units or 1 in 1.9 M (sqrtf() is about 18x more accurate, 1 in 34M)
uint32_t isqrt_d1i(uint64_t x)
{
	#include "fracsqrt_d1i.h"	// includes the LUT, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint64_t ish=64-lutsp;
	const uint32_t ip=32-prec, tp=32+abdp-prec, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the precision for t and how to shift from frx to t
	const uint64_t tmask = (1ULL<<ish)-1;
	uint32_t expon, lutind, t, a, b, fs;
	uint64_t frx;

	expon = (log2_ffo64(x)+1) & 0xFFFFFFFE;		// 3 cycles, get the evened exponent
	frx = x << (64-expon);				// 3 cycles, make the fractional part in 0.64 format

	lutind = frx >> ish;				// 0.33 cycles, index for the LUT
	expon = 32 - (expon>>1);			// the rest of the function takes about 9 cycles
	t = (frx & tmask) >> tbd;			// interpolator between two LUT entries

	a = fracsqrt[lutind];
	b = fracsqrt[lutind+1];
	fs = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// 6 cycles, fs = sqrt(frx) by linear interpolation of a and b by t in 0.32 format

	return fs >> expon;
}

// takes 31.33 cycles as opposed to 66 cycles for log2f() or logf()
// treats x as an integer, returns result in s5.26 format within the range [0.0 , 32.0[, except for fplog2(0) = undefined
int32_t fplog2_d1i(uint32_t x)		// input is in 32.0, output is in s5.26
{
	#include "fraclog2_d1i.h"	// includes LUT, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t ish=32-lutsp, ip=31-prec, tp=31+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, expon, frx, t;
	int32_t a, b, fl;

	expon = log2_ffo32(x);			// x = frx * 2^expon	->	fplog2(x) = log2(frx) + expon
	frx = x << (32-expon);			// turns the fractional part of normalised x into a 0.32 fp value
//	frx >>= (32-26);			// turns it into a 0.26 fp value

	lutind = frx >> ish;			// index for the LUT
	t = (frx & tmask) >> tbd;		// interpolator between two LUT entries

	a = fraclog2[lutind];
	b = fraclog2[lutind+1];
	fl = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// fl = log2(frx) by linear interpolation of a and b by t in 0.31 format

	return (expon<<26) + (fl>>5);		// output format is s5.26
}

// takes 12.66 cycles as opposed to 142 cycles for exp2f()
uint32_t fpexp2_d1i(int32_t x)	// input is s5.26, output is in 32.0
{
	#include "fracexp2_d1i.h"	// includes the fractional 2^x table as generated by tablegen.exe, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t ish=26-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, expon, frx, t;
	uint32_t a, b, fe;

	if (x < 0)		// treat negative input as being like -infinity
		return 0;

	expon = x>>26;			// extract the exponent, 5.0 fp value
	frx = x & 0x3FFFFFF;		// the fractional part is the 26 lower bits, 0.26 fp value

	lutind = frx >> ish;			// index for the LUT
	t = (frx & tmask) >> tbd;		// interpolator between two LUT entries

	a = fracexp2[lutind];
	b = fracexp2[lutind+1];
	fe = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);		// fe = exp2(frx) by linear interpolation of a and b by t in 2.30 format

	if (expon > 30)
		return (fe<<(expon-30));
	else
		return (fe>>(30-expon));		// output format is 32.0
}

int32_t fppow(uint32_t x, int32_t y, const uint32_t fmt)	// does pow(x, y) or x^y by doing exp2(log2(x) * y) or 2^(y*log2(x))
{
	int32_t xlog, xyl, r;

	xlog = fplog2(x) - (fmt<<26);			// log2(x) with x in (32-fmt).fmt format (for instance if fmt is 24 then x input format is 8.24, xlog stays in s5.26)
	xyl = ((uint64_t) xlog * (uint64_t) y) >> 26;	// doesn't handle overflows very gracefully as it might go up to s10.26
	r = fpexp2(xyl+(fmt<<26));			// in (32-fmt).fmt format

	return r;
}

#define fpsin(x) fpcos((x)+0xC0000000)
int32_t fpcos_d2(uint32_t x)		// 13 cycles
{
	#include "fpcos_d2.h"	// includes the quadratic coefficients LUT (3 kB) as generated by tablegen.exe, the format (prec) and LUT size power (lutsp)
	int32_t p, lutind;
	const uint32_t fmt = 32, ish = fmt-lutsp, osh = 30-prec;	// the shift for the index, output shift
	const int32_t *c;
	int64_t c0, c1, c2, xl=x;

	// lookup the 3 quadratic coefficients for c2*x^2 + c1*x + c0 then compute the result
	lutind = x >> ish;		// the index is lutsp bits
	c = &fpcos_lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	p = (((c2 * xl >> fmt) + c1) * xl >> fmt) + c0;		// does (c2*x + c1)*x + c0 instead of c2*x^2 + c1*x + c0

	return p << osh;
}

// takes 8.3 cycles compared to 94 cycles for cosf()
int32_t fpcos_d1i(uint32_t x)		// does the equivalent of cos(2.*pi*x), x = [0.0 , 1.0[ = [0 , 2^32[ = 0.32 fp value
{
	#include "fpcos_d1i.h"	// includes the cos LUT as generated by tablegen.exe, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t outfmt = 30;	// final output format in s1.outfmt
	const uint32_t ofs=30-outfmt, ish=32-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator between two LUT entries

	a = fpcos_lut[lutind];
	b = fpcos_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);	// y = cos(x) by linear interpolation of a and b by t in s1.30 format

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

// Caveat: bogus results if x or y == -2147483648
int64_t fpatan2_d2(int32_t y, int32_t x)	// 62 cycles, average error 0.00274", max error 0.00893" (16 metres on the Moon, 27 cm on the surface of the Earth)
{
	#include "fpatan2_d2.h"	// includes the quadratic coefficients LUT (1.5 kB) as generated by tablegen.exe, the format (prec=32) and LUT size power (lutsp)
	int32_t xa, xs, ya, ys, p, t, lutind;
	uint32_t d;
	const uint32_t fmt = 30, ish = fmt-lutsp+1, half = 1L<<(prec-1);	// the shift for the index, the value of 0.5
	const int32_t *c;
	int32_t c0, c1, c2;

	// turn x into xa (|x|) and sign of x (xs)
	xs = x >> 31;
	xa = (x^xs) - xs;
	ys = y >> 31;
	ya = (y^ys) - ys;

	d = ya+xa;
	if (d==0)		// if both |y| and |x| are 0 then they add up to 0 and we must return 0
		return 0;
	if (d >= 0x7FFFFFFF)	// this prevents signed overflow in the division
	{
		ya >>= 1;
		xa >>= 1;
		d = ya+xa;
	}

	t = fpdiv_d2(ya-xa, d, fmt);	// '/d' normalises distance to the unit diamond, immediate result of division is always <= +/-1^ds

	// lookup the 3 quadratic coefficients for c2*x^2 + c1*x + c0 then compute the result
	lutind = t >> ish;		// the index is lutsp bits
	c = &fpatan2_lut[lutind*3];    c0 = c[0];    c1 = c[1];    c2 = c[2];
	p = ((((int64_t) c2 * t >> fmt) + c1) * t >> fmt) + c0;		// does (c2*x + c1)*x + c0 instead of c2*x^2 + c1*x + c0

	// Quadrants
	if (xs)				// if x was negative
		p = half - p;		// p = 0.5 - p

	p = (p^ys) - ys;		// if y was negative then p is negated

	return p;
}

int32_t fpwsinc_d1i(int32_t x)	// Windowed sinc function in the [-2 , 2] range, input is 2+.24, output is s1.30
{
	#include "fpwsinc_d1i.h"	// includes the one-sided windowed sinc LUT as generated by tablegen.exe, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t outfmt = 30;	// final output format in s1.outfmt
	const uint32_t ofs=30-outfmt, ish=25-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	x = fastabs(x);

	if (x >= 2<<24)
		return 0;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator between two LUT entries

	a = fpwsinc_lut[lutind];
	b = fpwsinc_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

int32_t fpgauss_d1i(int32_t x)	// Gaussian function in the [-4 , 4] range, input is 3+.16, output is 1.30
{
	#include "fpgauss_d1i.h"	// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t outfmt = 30;	// final output format in 1.outfmt
	const uint32_t ofs=30-outfmt, ish=18-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	x = fastabs(x);

	if (x >= 4<<16)
		return 0;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator between two LUT entries

	a = fpgauss_lut[lutind];
	b = fpgauss_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

int32_t fpgauss_d0(int32_t x)	// Gaussian function in the [-4 , 4] range, input is 3+.16, output is 1.30
{
	#include "fpgauss_d0.h"			// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;		// final output format in 1.outfmt
	const uint32_t ish=18-lutsp, ip=outfmt-prec;	// the shifts for the index and the output

	x = fastabs(x);

	if (x >= 4<<16)
		return 0;

	return fpgauss_lut[x >> ish] << ip;
}

int32_t fpgauss_d0_nocheck(int32_t x)	// no limits check for speed, caller must make sure |x| <= 4<<16
{
	#include "fpgauss_d0.h"			// includes the one-sided gaussian LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;		// final output format in 1.outfmt
	const uint32_t ish=18-lutsp, ip=outfmt-prec;	// the shifts for the index and the output

	x = fastabs(x);

	return fpgauss_lut[x >> ish] << ip;
}

int32_t fperfr_d1i(int32_t x)	// 0.5erf(x)+0.5 function in the [-4 , 4] range, input is s3+.16, output is 1.30
{
	#include "fperfr_d1i.h"	// includes the two-sided raised erf LUT as generated by tablegen.exe, the entry bit precision (prec), LUT size power (lutsp) and how many max bits |b-a| takes (abdp)
	const uint32_t outfmt = 30;	// final output format in 1.outfmt
	const uint32_t ofs=30-outfmt, ish=19-lutsp, ip=30-prec, tp=30+abdp-prec, tmask = (1<<ish)-1, tbd=(ish-tp);	// the shift for the index, bit precision of the interpolation, the mask and how to make t be 0.10
	uint32_t lutind, t;
	int32_t a, b, y;

	if (x >= 4<<16)
		return 1<<outfmt;
	if (x <= -(4<<16))
		return 0;

	x += 4<<16;

	lutind = x >> ish;		// index for the LUT
	t = (x & tmask) >> tbd;		// interpolator between two LUT entries

	a = fperfr_lut[lutind];
	b = fperfr_lut[lutind+1];
	y = (((b-a) * (int32_t) t) >> abdp) + (a<<ip);

	return y >> ofs;		// truncates the result, output in s1.outfmt format
}

int32_t fperfr_d0(int32_t x)	// 0.5erf(x)+0.5 function in the [-4 , 4] range, input is s3+.16, output is 1.30
{
	#include "fperfr_d0.h"			// includes the one-sided raised erf LUT as generated by tablegen.exe, the entry bit precision (prec) and LUT size power (lutsp)
	const uint32_t outfmt = 30;		// final output format in 1.outfmt
	const uint32_t ish=19-lutsp, ip=outfmt-prec;	// the shifts for the index and the output

	if (x >= 4<<16)
		return 1<<outfmt;
	if (x <= -(4<<16))
		return 0;

	x += 4<<16;

	return fperfr_lut[x >> ish] << ip;
}

#endif
