#define DQS_BRACKET_LEVELS 4
#define DQS_BLOCK_COUNT (DQS_BRACKET_LEVELS + 1)

extern int dqsb_bracket_open(float **block, int brlvl, int chan_stride);
extern int dqsb_bracket_close(float **block, int brlvl, enum dq_blend blendmode, int chan_stride);
extern void dqsb_draw_black_rect_inv(float *le, float *block, xy_t start_pos, int bs, int chan_stride);
extern void dqsb_draw_polygon(float *le, int point_count, float *block, xy_t start_pos, int bs, int chan_stride);
extern void dqsb_draw_effect(enum dq_type type, float *le, float *block, int chan_stride);
extern void dqsb_draw_circle(enum dq_type type, float *le, float *block, xy_t start_pos, int bs, int chan_stride);
extern void dqsb_draw_gradient_test(float *block, xy_t start_pos, int bs, int chan_stride, xyi_t frame_dim);
