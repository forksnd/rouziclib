// in graphics/graphics_struct.h:
// mipmap_t, mipmap_level_t

extern void alloc_mipmap_level(mipmap_level_t *ml, xyi_t fulldim, xyi_t tilesize, const int mode);
extern void *get_tile_pixel_ptr(mipmap_level_t ml, xyi_t pos, const int mode);
extern frgb_t get_tile_pixel(mipmap_level_t ml, xyi_t pos);
extern void set_tile_pixel(mipmap_level_t ml, xyi_t pos, frgb_t pv);
extern void copy_from_raster_to_tiles(raster_t r, mipmap_level_t ml, const int mode);
extern void tile_downscale_fast_box(mipmap_level_t r0, mipmap_level_t r1, const xyi_t ratio);
extern void tile_downscale_box_2x2(mipmap_level_t ml0, mipmap_level_t ml1, const int mode);
extern mipmap_t raster_to_tiled_mipmaps_fast(raster_t r, xyi_t tilesize, xyi_t mindim, const int mode);
extern mipmap_t raster_to_tiled_mipmaps_fast_defaults(raster_t r, const int mode);
extern mipmap_t raster_to_mipmap_then_free(raster_t *r, const int mode);
extern void free_mipmap_level(mipmap_level_t *ml);
extern void free_mipmap(mipmap_t *m);
extern void free_mipmap_array(mipmap_t *m, int count);
extern void remove_mipmap_levels_above_dim(mipmap_t *m, xyi_t dim);
extern void blit_mipmap(mipmap_t m, xy_t pscale, xy_t pos, int interp);
extern void blit_mipmap_in_rect(mipmap_t m, rect_t r, int keep_aspect_ratio, int interp);
extern int get_largest_mipmap_lvl_index(mipmap_t m);
extern xyi_t get_largest_mipmap_lvl_dim(mipmap_t m);

#define MIPMAP_TILE_SIZE	512
#define MIPMAP_MIN_SIZE		4
