// in gui/zoom_struct.h:
// zoom_t

extern zoom_t init_zoom(mouse_t *mouse, double drawing_thickness);
extern double to_screen_coord_x(double x);
extern double to_screen_coord_y(double y);
extern xy_t to_screen_coord_xy(xy_t p);
extern double to_world_coord_x(double x);
extern double to_world_coord_y(double y);
extern xy_t to_world_coord_xy(xy_t p);
extern rect_t to_screen_coord_rect(rect_t r);
extern rect_t to_world_coord_rect(rect_t r);
extern void pinned_os_to_world_os(xy_t pinned_offset, double pinned_sm, xy_t *world_offset, double *world_sm);
extern void world_os_to_pinned_os(xy_t world_offset, double world_sm, xy_t *pinned_offset, double *pinned_sm);

extern void zoom_toggle(zoom_t *zc, int8_t *flag_zoom_key);
extern void zoom_reset(zoom_t *zc, int8_t *flag_zoom_key);
extern void zoom_key_released(zoom_t *zc, int8_t *flag_zoom_key, int source);
extern void zoom_wheel(zoom_t *zc, int8_t flag_zoom_key, int y);
extern void calc_screen_limits(zoom_t *zc);
extern void toggle_guizoom(zoom_t *zc, int on);
extern void change_zoom(xy_t pos, double zoom_scale);
extern void change_zoom_and_turn_off_zoom_mode(xy_t pos, double zoom_scale);

extern void zoom_keyboard_control(zoom_t *zc, int8_t *flag_zoom_key);
extern void zoom_overlay_control(zoom_t *zc, int8_t *flag_zoom_key);

#define sc_x(p) to_screen_coord_x(p)
#define sc_y(p) to_screen_coord_y(p)
#define sc_xy(p) to_screen_coord_xy(p)
#define sc_rect(r) to_screen_coord_rect(r)
#define wc_x(p) to_world_coord_x(p)
#define wc_y(p) to_world_coord_y(p)
#define wc_xy(p) to_world_coord_xy(p)
#define wc_rect(r) to_world_coord_rect(r)

extern void prepare_focus_transition(filter_focus_t *focus);
extern void perform_focus_transition(filter_focus_t *focus);
