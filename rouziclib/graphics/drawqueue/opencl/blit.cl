/*bool check_image_bounds(int2 pi, int2 im_dim)
{
	if (pi.x >= 0 && pi.x < im_dim.x)
		if (pi.y >= 0 && pi.y < im_dim.y)
			return true;
	return false;
}

float4 image_interp_nearest(global float4 *im, int2 im_dim, float2 pif)
{
	float4 pv;
	int2 pi;

	pi = convert_int2(pif + 0.5f);

	if (check_image_bounds(pi, im_dim))
		pv = im[pi.y * im_dim.x + pi.x];
	else
		pv = 0.f;

	return pv;
}

float4 get_image_pixel_weighted_linear(global float4 *im, int2 im_dim, float2 pif, float2 pjf)
{
	float4 pv;
	float2 w;
	int2 pj = convert_int2(pjf);

	w = 1.f - fabs(pif-pjf);		// weight for current pixel

	if (check_image_bounds(pj, im_dim))
		pv = im[pj.y * im_dim.x + pj.x] * (w.x*w.y);
	else
		pv = 0.f;

	return pv;
}

float4 image_interp_linear(global float4 *im, int2 im_dim, float2 pif)
{
	float4 pv = 0.f;
	float2 pif00;

	pif00 = floor(pif);

	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00);
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(0.f, 1.f));
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(1.f, 0.f));
	pv += get_image_pixel_weighted_linear(im, im_dim, pif, pif00 + (float2)(1.f, 1.f));

	return pv;
}*/

float4 read_frgb_pixel(global float4 *im, int index)
{
	return im[index];
}

float4 read_sqrgb_pixel(global uint *im, int index)
{
	float4 pv;
	uint4 pvi;
	uint v;
	const float mul_rb = 1.f / (1023.f*1023.f);
	const float mul_g = 1.f / (4092.f*4092.f);
	const float4 mul_pvi = (float4) (mul_rb, mul_g, mul_rb, 1.f);

	v = im[index];
	pvi.z = v >> 22;
	pvi.y = (v >> 10) & 4095;
	pvi.x = v & 1023;
	pvi.w = 1;

	pv = convert_float4(pvi*pvi) * mul_pvi;

	return pv;
}

float4 read_srgb_pixel(global uint *im, int index)
{
	float4 pv;
	uint v;

	v = im[index];
	pv.z = s8lrgb((v >> 16) & 255);
	pv.y = s8lrgb((v >> 8) & 255);
	pv.x = s8lrgb(v & 255);
	pv.w = 1.;

	return pv;
}

float4 raw_yuv_to_lrgb(float3 raw, float depth_mul)
{
	float y, u, v, r, g, b;
	float4 pv;

	raw *= depth_mul;
	y = raw.x - 16.f;
	y *= 255.f / 219.f;
	u = raw.y - 128.f;
	v = raw.z - 128.f;

	r = y + 1.596f * v;
        g = y - 0.813f * v - 0.391f * u;
        b = y + 2.018f * u;

	pv.x = s8lrgb(r);
	pv.y = s8lrgb(g);
	pv.z = s8lrgb(b);
	pv.w = 1.;

	return pv;
}

float4 read_yuv420p8_pixel(global uchar *im, int2 im_dim, int2 i)
{
	float4 pv;
	float y, u, v, r, g, b;
	int2 im_dimh = im_dim / 2;
	int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
	global uchar *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = i.y/2 * im_dimh.x + i.x/2;	// TODO fix for MPEG-2 layout

	pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f );

	return pv;
}

float4 read_yuv420pN_pixel(global ushort *im, int2 im_dim, int2 i, float depth_mul)	// yuv420p formats above 8 bpc
{
	float4 pv;
	float y, u, v, r, g, b;
	int2 im_dimh = im_dim / 2;
	int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
	global ushort *u_plane, *v_plane;

	u_plane = &im[size_full];
	v_plane = &im[size_full + size_half];
	y_index = i.y * im_dim.x + i.x;
	uv_index = i.y/2 * im_dimh.x + i.x/2;

	pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), depth_mul );

	return pv;
}

// Compressed texture
uint bits_to_mask(uint bits)	// 7 becomes 0x7F
{
	return (1UL << bits) - 1;
}

float bits_to_mul(uint bits)	// 7 becomes 127.
{
	//return (float) ((1ULL << bits) - 1);
	return convert_float((int) ((1ULL << bits) - 1));
}

float3 decompr_hsl(int bits_ch, int bits_cs, int bits_cl, int ih, int is, int il)
{
	float3 hsl;

	hsl.x = (float) ih / (bits_to_mul(bits_ch)-1.f);
	hsl.y = (float) (is+1) / (bits_to_mul(bits_cs)+1.f);
	hsl.z = (float) il / bits_to_mul(bits_cl);

	if (ih == bits_to_mask(bits_ch))		// if the hue is the reserved value of no saturation
	{
		hsl.x = 0.f;
		hsl.y = 0.f;
	}

	return hsl;
}

float4 compr_hsl_to_rgb(float3 hsl)
{
	float4 rgb;
	float3 w = (float3) (0.124f, 0.686f, 0.19f);

	hsl.x *= 3.f;					// H in HUE03
	hsl.z = Lab_L_to_linear(hsl.z);			// linear L
	rgb = (float4) (hsl_to_rgb_cw(w, hsl), 1.f);

	rgb.x = linear_to_Lab_L(rgb.x);
	rgb.y = linear_to_Lab_L(rgb.y);
	rgb.z = linear_to_Lab_L(rgb.z);
	return rgb;
}

float4 read_compressed_texture1_pixel(global uchar *d8, int2 im_dim, int2 i)
{
	global ushort *d16 = (global ushort *) d8;
	float4 pv;
	ulong di, blocks_start = 80;
	int block_size, bits_per_block, quincunx, bits_ch, bits_cs, bits_cl, bits_per_pixel;
	int linew0, linew1, line_count0, line_count1;
	int h0, s0, l0, h1, s1, l1, pix, qoff;
	int2 block_start, ib;
	float4 col0, col1;

	// Load decoding parameters
	block_size = d16[0];
	bits_per_block = d16[1];
	quincunx = d8[5];
	bits_ch = d8[6];
	bits_cs = d8[7];
	bits_cl = d8[8];
	bits_per_pixel = d8[9];

	block_start.y = (i.y / block_size);
	qoff = (block_start.y&1) * quincunx * (block_size>>1);
	block_start.x = (i.x + qoff) / block_size;

	// Calculate block start in bits
	linew0 = idiv_ceil(im_dim.x, block_size);
	linew1 = idiv_ceil(im_dim.x + quincunx*(block_size>>1), block_size);
	line_count0 = block_start.y+1 >> 1;
	line_count1 = block_start.y >> 1;
	di = line_count0*linew0 + line_count1*linew1;		// y block line start id
	di += block_start.x;					// x block start id
	di = di*bits_per_block + blocks_start;			// block id to bit position

	// Position in pixels of the first pixel of the block
	block_start *= block_size;
	block_start.x -= qoff;
	ib = i - block_start;		// position of the pixel in the block

	// Read block data
	h0 = get_bits_in_stream_inc(d8, &di, bits_ch);
	s0 = get_bits_in_stream_inc(d8, &di, bits_cs);
	l0 = get_bits_in_stream_inc(d8, &di, bits_cl);
	h1 = get_bits_in_stream_inc(d8, &di, bits_ch);
	s1 = get_bits_in_stream_inc(d8, &di, bits_cs);
	l1 = get_bits_in_stream_inc(d8, &di, bits_cl);

	di += (ib.y*block_size + ib.x) * bits_per_pixel;
	pix = get_bits_in_stream(d8, di, bits_per_pixel);

	// Decode colours
	col0 = compr_hsl_to_rgb(decompr_hsl(bits_ch, bits_cs, bits_cl, h0, s0, l0));
	col1 = compr_hsl_to_rgb(decompr_hsl(bits_ch, bits_cs, bits_cl, h1, s1, l1));

	pv = mix(col0, col1, convert_float(pix) / bits_to_mul(bits_per_pixel));
	pv.x = Lab_L_to_linear(pv.x);
	pv.y = Lab_L_to_linear(pv.y);
	pv.z = Lab_L_to_linear(pv.z);
	return pv;
}

float4 read_fmt_pixel(const int fmt, global uchar *im, int2 im_dim, int2 i)
{
	switch (fmt)
	{
		case 0:		// frgb_t
			return read_frgb_pixel(im, i.y * im_dim.x + i.x);

		case 1:		// sqrgb_t
			return read_sqrgb_pixel(im, i.y * im_dim.x + i.x);

		case 2:		// srgb_t
			return read_srgb_pixel(im, i.y * im_dim.x + i.x);

		case 10:	// YCbCr 420 planar 8-bit (AV_PIX_FMT_YUV420P)
			return read_yuv420p8_pixel(im, im_dim, i);

		case 11:	// YCbCr 420 planar 10-bit LE (AV_PIX_FMT_YUV420P10LE)
			return read_yuv420pN_pixel(im, im_dim, i, 0.25f);

		case 12:	// YCbCr 420 planar 12-bit LE (AV_PIX_FMT_YUV420P12LE)
			return read_yuv420pN_pixel(im, im_dim, i, 0.0625f);

		case 20:	// Compressed texture format
			return read_compressed_texture1_pixel(im, im_dim, i);
	}

	return 0.f;
}

float calc_flattop_weight(float2 pif, float2 i, float2 knee, float2 slope, float2 pscale)
{
	float2 d, w;

	d = fabs(pif - i);
	d = max(d, knee);
	w = slope * (d - pscale);

	return w.x * w.y;
}

float4 image_filter_flattop(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale, float2 slope)
{
	float4 pv = 0.f;
	float2 knee, i, start, end;

	knee = 0.5f - fabs(fmod(pscale, 1.f) - 0.5f);

	start = max(0.f, ceil(pif - pscale));
	end = min(convert_float2(im_dim - 1), floor(pif + pscale));

	for (i.y = start.y; i.y <= end.y; i.y+=1.f)
		for (i.x = start.x; i.x <= end.x; i.x+=1.f)
			pv += read_fmt_pixel(fmt, im, im_dim, convert_int2(i)) * calc_flattop_weight(pif, i, knee, slope, pscale);

	return pv;
}

/*float4 blit_sprite_bilinear(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global float4 *im;
	int2 im_dim;
	float2 pscale, pos, pif;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];

	pif = pscale * (pf + pos);
	//pv += image_interp_nearest(im, im_dim, pif);
	pv += image_interp_linear(im, im_dim, pif);

	return pv;
}*/

float4 blit_sprite_flattop(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif, slope;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	fmt = lei[8];
	slope.x = lef[9];
	slope.y = lef[10];

	pif = pscale * (pf + pos);
	pscale = max(1.f, pscale);
	pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);

	return pv;
}

float4 blit_sprite_flattop_rot(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global float4 *im;
	int2 im_dim;
	int fmt;
	float2 pscale, pos, pif, pifo, slope;
	float costh, sinth;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = pscale.x;
	pos.x = lef[5];
	pos.y = lef[6];
	fmt = lei[7];
	slope.x = lef[8];
	slope.y = slope.x;
	costh = lef[9];
	sinth = lef[10];

	pifo = pscale * (pf + pos);
	pif.x = pifo.x * costh - pifo.y * sinth;
	pif.y = pifo.x * sinth + pifo.y * costh;
	pscale = max(1.f, pscale);
	pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);

	return pv;
}

/*float2 set_new_distance_from_point(float2 p0, float2 pc, float dist_mul)
{
	return pc + (p0 - pc) * dist_mul;
}

float inverse_distortion(float a, float x)
{
	float p1, p2, p3;

	p1 = cbrt(2.f / (3.f * a));
	p2 = cbrt( sqrt(3.f*a) * sqrt( 27.f*a*x*x + 4.f ) - 9.f*a*x );
	p3 = cbrt(2.f) * pow(3.f*a, 2.f / 3.f);

	return p1/p2 - p2/p3;	// http://www.wolframalpha.com/input/?i=inverse+of+x+*+(1+%2B+11*x%5E2)
}

float4 blit_photo(global uint *lei, global uchar *data_cl, float4 pv)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const float2 pf = convert_float2(p);
	global float *lef = lei;
	global float4 *im;
	int2 im_dim;
	float2 pscale, pos, pif, pc;
	float distortion, dist_scale, gain, distortion_mul, d;

	im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
	im_dim.x = lei[2];
	im_dim.y = lei[3];
	pscale.x = lef[4];
	pscale.y = lef[5];
	pos.x = lef[6];
	pos.y = lef[7];
	pc.x = lef[8];
	pc.y = lef[9];
	distortion = lef[10];
	dist_scale = lef[11];
	gain = lef[12];

	pif = pscale * (pf + pos);

	d = fast_distance(pif, pc) * dist_scale;
	if (distortion==0.f)
		distortion_mul = 1.f;
	else
		distortion_mul = inverse_distortion(distortion, d) / d;
	pif = set_new_distance_from_point(pif, pc, distortion_mul);

	//pv += image_interp_nearest(im, im_dim, pif);
	pv += gain * image_interp_linear(im, im_dim, pif);

	return pv;
}*/
