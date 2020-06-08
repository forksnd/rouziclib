typedef struct
{
	int block_size, block_step, bits_per_block, odd_offset, quincunx;
	int window_base_count;
	float *window_base;
	xy_t block_centre;
	int bits_ch, bits_cs, bits_cl, bits_erf_th, bits_erf_off, bits_erf_width, bits_per_pixel, bits_padding;
} compression_param1_t;

extern raster_t frgb_to_compressed_texture(raster_t r0, compression_param1_t *cp_in);
extern raster_t compressed_texture_to_frgb(raster_t r0);
