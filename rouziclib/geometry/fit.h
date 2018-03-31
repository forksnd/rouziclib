extern double fit_n_squares_in_area(xy_t area_dim, int count, xyi_t *grid_count_ptr);
extern double fit_n_squares_in_area_fill_x(xy_t area_dim, int count, xyi_t *grid_count_ptr);
extern xy_t fit_n_rects_in_area_fill_x(xy_t area_dim, xy_t rect_dim, int count, xyi_t *grid_count_ptr);
extern xyi_t get_dim_of_tile(xyi_t fulldim, xyi_t tilesize, xyi_t index);
extern rect_t fit_rect_in_area(xy_t r_dim0, rect_t area, xy_t off);
extern int hilbert_curve_coord_to_index(const int n, xyi_t p);
extern xyi_t hilbert_curve_index_to_coord(const int n, int index);
extern void hilbert_curve_rotate(const int n, xyi_t *p, xyi_t r);
