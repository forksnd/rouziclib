extern void gaussian_blur(float *a, float *b, xyi_t dim, const int channels, double radius);
extern void image_downscale_fast_box(raster_t r0, raster_t *r1, const xyi_t ratio, const int mode);
extern void image_pixel_process_arg0(raster_t r, const int mode, void (*func)(void));
extern void image_pixel_process_arg1f(raster_t r, const int mode, void (*func)(void), float arg1);
extern void pixel_invert_linear(float *p0, float *p1);
extern void pixel_alpha_to_grey(float *p0, float *p1);
extern void pixel_mul_by_own_alpha(float *p0, float *p1);
extern void pixel_mul_by_ratio(float *p0, float *p1, float ratio);

#define image_invert_linear(r, mode)		image_pixel_process_arg0(r, mode, pixel_invert_linear)
#define image_alpha_to_grey(r, mode)		image_pixel_process_arg0(r, mode, pixel_alpha_to_grey)
#define image_mul_by_own_alpha(r, mode)		image_pixel_process_arg0(r, mode, pixel_mul_by_own_alpha)
#define image_mul_by_ratio(r, mode, ratio)	image_pixel_process_arg1f(r, mode, pixel_mul_by_ratio, ratio)
