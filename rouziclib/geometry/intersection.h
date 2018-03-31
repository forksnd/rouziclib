extern xy_t line_line_intersection(xy_t p1, xy_t p2, xy_t p3, xy_t p4);
extern double pos_on_line(xy_t p1, xy_t p2, xy_t p);
extern int check_line_collision(xy_t p1, xy_t p2, xy_t p3, xy_t p4, double *u, int32_t exclusive);
extern double point_line_distance(xy_t p1, xy_t p2, xy_t p3);
extern void border_clip(double w, double h, xy_t *l1, xy_t *l2, double radius);
extern void line_rect_clip(xy_t *l1, xy_t *l2, rect_t br);
extern int check_point_within_box(xy_t p, rect_t box);
extern int check_point_within_box_int(xyi_t p, recti_t box);
extern int plane_line_clip_far_z(xyz_t *p1, xyz_t *p2, double zplane);
extern int check_box_box_intersection(rect_t box0, rect_t box1);
extern int check_box_box_intersection_int(recti_t box0, recti_t box1);
extern rect_t rect_intersection(rect_t r1, rect_t r2);
extern int check_box_circle_intersection(rect_t box, xy_t circ, double rad);
extern int check_box_wholly_inside_circle(rect_t box, xy_t circ, double rad);
extern int check_pixel_within_image(xyi_t pos, xyi_t im_dim);
