typedef struct
{
	rect_t bound;
	size_t array_index;
	xyi_t degree;
	xy_t eval_offset;
} polynomial_cell_t;

typedef struct
{
	rect_t bound;
	xyi_t dim;
	xy_t step, inv_step;
	polynomial_cell_t **cell;

	double *coef;
	size_t coef_count, coef_as;
} polynomial_grid_t;

typedef struct
{
	uint32_t codepoint, alias;
	vobj_t *obj;
	vobj_tri_t tri_mesh;
	polynomial_grid_t polynomial_grid;
	float bl, br, bb, bt;	// bounds to the left and right, bottom and top
	float width;
	char *glyphdata;
	int16_t point_count, line_count, pid_offset, max_pid;
} letter_t;

typedef struct
{
	letter_t *l;
	int letter_count, alloc_count;
	int32_t **codepoint_letter_lut;

	double letter_spacing, line_vspacing;
	int8_t use_mesh;		// toggled to use the triangle mesh instead of the polynomial grid when both are present

	int32_t *cjkdec_pos, *cjkdec_data;
	int cjkdec_data_count, cjkdec_alloc_count;
} vector_font_t;

typedef struct
{
	xy_t pos;
	double scale;
	int bidi;
} text_pos_t;

static text_pos_t make_text_pos(xy_t pos, double scale, int bidi)
{
	text_pos_t tp;

	tp.pos = pos;
	tp.scale = scale;
	tp.bidi = bidi;

	return tp;
}

typedef struct
{
	int getpos_count;
	text_pos_t *getpos;
} text_param_t;
