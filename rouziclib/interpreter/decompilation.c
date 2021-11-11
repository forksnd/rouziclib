const char *rlip_get_op_name(enum opcode op)
{
	// To update copy the enum and apply this regex:
	// s/\t\([^, ]*\).*$/\t\tcase \1:    \treturn "\1";
	switch (op)
	{
		case op_1word_ops:    	return "op_1word_ops";
		case op_end:    	return "op_end";

		case op_2word_ops:    	return "op_2word_ops";
		case op_ret_v:    	return "op_ret_v";
		case op_jmp:    	return "op_jmp";
		case op_set0_v:    	return "op_set0_v";
		case op_set0_i:    	return "op_set0_i";
		case op_inc1_v:    	return "op_inc1_v";
		case op_inc1_i:    	return "op_inc1_i";

		case op_3word_ops:    	return "op_3word_ops";
		case op_load_v:    	return "op_load_v";
		case op_load_i:    	return "op_load_i";
		case op_set_v:    	return "op_set_v";
		case op_set_i:    	return "op_set_i";
		case op_cvt_i_v:    	return "op_cvt_i_v";
		case op_cvt_v_i:    	return "op_cvt_v_i";
		case op_sq_v:    	return "op_sq_v";
		case op_sqrt_v:    	return "op_sqrt_v";

		case op_jmp_v_ez:    	return "op_jmp_v_ez";
		case op_jmp_v_nz:    	return "op_jmp_v_nz";
		case op_jmp_i_ez:    	return "op_jmp_i_ez";
		case op_jmp_i_nz:    	return "op_jmp_i_nz";
		case op_func0_v:    	return "op_func0_v";

		case op_4word_ops:    	return "op_4word_ops";
		case op_add_vv:    	return "op_add_vv";
		case op_add_ii:    	return "op_add_ii";
		case op_sub_vv:    	return "op_sub_vv";
		case op_sub_ii:    	return "op_sub_ii";
		case op_mul_vv:    	return "op_mul_vv";
		case op_mul_ii:    	return "op_mul_ii";
		case op_div_vv:    	return "op_div_vv";
		case op_div_ii:    	return "op_div_ii";
		case op_mod_ii:    	return "op_mod_ii";
		case op_mod_vv:    	return "op_mod_vv";
		case op_pow_vv:    	return "op_pow_vv";

		case op_jmp_vv_lt:    	return "op_jmp_vv_lt";
		case op_jmp_ii_lt:    	return "op_jmp_ii_lt";
		case op_func1_vv:    	return "op_func1_vv";

		case op_5word_ops:    	return "op_5word_ops";
		case op_func2_vvv:    	return "op_func2_vvv";

		case op_6word_ops:    	return "op_6word_ops";
		case op_func3_vvvv:    	return "op_func3_vvvv";

		case op_7word_ops:    	return "op_7word_ops";
		case op_8word_ops:    	return "op_8word_ops";
	}

	return "Unknown op";
}

buffer_t rlip_decompile(rlip_t *d)
{
	int i;
	buffer_t buffer={0}, *b=&buffer;
	uint64_t *op = d->op;

	while (1)
	{
		bufprintf(b, "%s\t", rlip_get_op_name(op[0]));

		for (i=1; i < op[0] >> 10; i++)
			bufprintf(b, "%6d\t", op[i]);
		bufprintf(b, "\n");

		if (op[0] == op_end)
			return buffer;

		op = &op[op[0] >> 10];
	}

	return buffer;
}