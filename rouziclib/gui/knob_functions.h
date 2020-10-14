// in gui/controls_struct.h:
// knob_func_t

extern double knobf_linear(double v, double min, double max, const int mode);
extern double knobf_log(double v, double min, double max, const int mode);
extern double knobf_recip(double v, double min, double max, const int mode);
extern double knobf_dboff(double v, double min, double max, const int mode);

extern const char *knob_func_name[];
extern const knob_func_t knob_func_array[];
extern knob_func_t knob_func_name_to_ptr(const char *name);
extern const char *knob_func_ptr_to_name(knob_func_t fp);
