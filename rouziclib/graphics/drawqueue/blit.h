extern void dqsb_blit_sprite_flattop(float *lef, uint8_t *drawq_data, float *block, xy_t start_pos, const int bs, int chan_stride);  // SSE4.1
extern void dqsb_blit_sprite_flattop_rot(float *lef, uint8_t *drawq_data, float *block, xy_t start_pos, const int bs, int chan_stride);
extern void dqsb_blit_sprite_aa_nearest(float *lef, uint8_t *drawq_data, float *block, xy_t start_pos, const int bs, int chan_stride);
extern void dqsb_blit_sprite_aa_nearest_rot(float *lef, uint8_t *drawq_data, float *block, xy_t start_pos, const int bs, int chan_stride);
