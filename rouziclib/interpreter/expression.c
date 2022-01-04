enum symbol_type
{
	sym_none,
	sym_value,
	sym_operator,
	sym_variable,
	sym_function,
	sym_bracket_open,
	sym_bracket_close,
	sym_arg_open,
	sym_arg_close,
	sym_comma,
	sym_result_real,
	sym_result_int,
};

typedef struct
{
	enum symbol_type type;
	int depth, match;
	int operator_priority;
	int result_id;
	int can_imply_mul_with_prev, can_imply_mul_with_next;
	const char *p;
	int p_len;
} symbol_data_t;

int find_res_taken(int *res_taken, int count)
{
	// Pick result ID
	for (int i=0; i < count; i++)
		if (res_taken[i] == 0)
		{
			res_taken[i] = 1;
			return i;
		}

	return -1;	// won't happen
}

void sym_move(symbol_data_t *sym, size_t *sym_count, int dst, int src)
{
	memmove(&sym[dst], &sym[src], (*sym_count - src) * sizeof(symbol_data_t));
	*sym_count -= src - dst;

	// Update match IDs
	for (int is=0; is < *sym_count; is++)
		if (sym[is].match >= dst)
			sym[is].match -= src - dst;
}

buffer_t expression_to_rlip_listing(const char *expression, const char *cmd_suffix, int use_real, buffer_t *comp_log)
{
	buffer_t prog_s={0}, *prog=&prog_s;
	int i, is, n=0, cant_be_operator=1, depth=0, max_depth=0, pi_loaded=0;
	size_t len = strlen(expression);
	symbol_data_t *sym=NULL;
	size_t sym_count=0, sym_as=0;
	char name[33], red_str[8], grey_str[8];
	const char *p = expression, *end = &expression[len];
	int *res_taken_r=NULL, *res_taken_i=NULL;
	char var_decl = use_real ? 'r' : 'd';
	int last_res_type=0, last_res_id=0;

	sprint_unicode(red_str, sc_red);
	sprint_unicode(grey_str, sc_grey);


	// FIXME a one-symbol expression does nothing

	// Map the string as symbols
	while (1)
	{
loop_start:
		p = skip_whitespace(&p[n]);
		if (p == end)
			break;

		// Prepare new symbol
		is = sym_count;
		alloc_enough(&sym, sym_count+=1, &sym_as, sizeof(symbol_data_t), 2.);
		sym[is].p = p;
		sym[is].match = -1;
		sym[is].result_id = -1;
		sym[is].depth = depth;
		max_depth = MAXN(max_depth, depth);

		// Try to read operators
		if (cant_be_operator==0 && isalnum(p[0]) == 0)
		{
			n = 0;
			sscanf(p, "%32[-+*/%%%^=<>!]%n", name, &n);
			if (n)
			{
				bufprintf(comp_log, "Operator %s, %d chars\n", name, n);
				sym[is].type = sym_operator;
				sym[is].p = p;
				sym[is].p_len = n;

				switch (p[0])
				{
					case '^':
						sym[is].operator_priority = 4;
						break;
					case '*':
					case '/':
					case '%':
						sym[is].operator_priority = 3;
						break;
					case '+':
					case '-':
						sym[is].operator_priority = 2;
						break;
					case '=':
					case '<':
					case '>':
					case '!':
						sym[is].operator_priority = 1;
						break;
				}

				cant_be_operator = 1;
				goto loop_start;
			}
			else if (p[0] == ',')
			{
				n = 1;
				bufprintf(comp_log, "Comma\n");
				sym[is].type = sym_comma;
				cant_be_operator = 1;
				goto loop_start;
			}
		}

		// Try to read number
		n = 0;
		sscanf(p, "%*g%n", &n);
		if (n)
		{
			//string_to_ddouble(p, NULL);
			bufprintf(comp_log, "Value, %d chars\n", n);
			sym[is].type = sym_value;
			sym[is].p = p;
			sym[is].p_len = n;
			sym[is].can_imply_mul_with_prev = 1;
			sym[is].can_imply_mul_with_next = 1;
			cant_be_operator = 0;
			goto loop_start;
		}

		// Try to read variable/function name
		if (isalpha(p[0]))
		{
			n = 0;
			sscanf(p, "%32[a-zA-Z0-9_]%n", name, &n);
			if (n)
			{
				// The "pi" exception
				if (pi_loaded == 0 && strcmp(name, "pi") == 0)
				{
					// Create a variable "pi" and call the pi function to set it
					bufprintf(prog, "%c pi = pi%s\n", var_decl, cmd_suffix);
					pi_loaded = 1;
				}

				bufprintf(comp_log, "%s named '%s', %d chars\n", p[n]=='(' ? "Function" : "Variable", name, n);
				sym[is].type = p[n]=='(' ? sym_function : sym_variable;
				sym[is].p = p;
				sym[is].p_len = n;
				sym[is].can_imply_mul_with_prev = 1;
				sym[is].can_imply_mul_with_next = (sym[is].type == sym_variable);
				cant_be_operator = 0;
				goto loop_start;
			}
		}

		// Try to read brackets
		if (p[0] == '(')
		{
			n = 1;
			sym[is].type = sym_bracket_open;
			if (is > 0)
				if (sym[is-1].type == sym_function)
					sym[is].type = sym_arg_open;
			bufprintf(comp_log, "%s\n", sym[is].type==sym_arg_open ? "Function bracket open" : "Bracket open");
			sym[is].can_imply_mul_with_prev = (sym[is].type == sym_bracket_open);
			cant_be_operator = 1;
			depth++;
			goto loop_start;
		}

		if (p[0] == ')')
		{
			n = 1;
			sym[is].type = sym_bracket_close;
			sym[is].can_imply_mul_with_next = 1;

			// Match brackets by finding the first preceding unmatched '('
			for (i=is-1; i >= 0; i--)
			{
				if ((sym[i].type == sym_bracket_open || sym[i].type == sym_arg_open) && sym[i].match == -1)
				{
					sym[i].match = is;
					sym[is].match = i;

					// If the brackets are part of a function call
					if (sym[i].type == sym_arg_open)
					{
						sym[is].type = sym_arg_close;
						sym[i-1].match = is;			// indicate where the function call ends
					}

					break;
				}
			}

			bufprintf(comp_log, sym[is].type == sym_bracket_close ? "Bracket close\n" : "Function bracket close\n");

			// If the ')' remains unmatched
			if (sym[is].match == -1)
			{
				bufprintf(comp_log, "%sUnmatched ')' at position %d%s\n", red_str, is, grey_str);
				goto invalid_expr;
			}

			cant_be_operator = 0;
			depth--;
			goto loop_start;
		}

		// Unidentified character
		if (p[0] > 0)
			bufprintf(comp_log, "%sCharacter '%c' (0x%x) unidentified%s\n", red_str, p[0], p[0], grey_str);
		else
			bufprintf(comp_log, "%sCharacter 0x%x unidentified%s\n", red_str, p[0], grey_str);
		n = 1;
		sym_count--;
		goto invalid_expr;
	}

	// Check for unmatched brackets
	for (is=0; is < sym_count; is++)
		if ((sym[is].type == sym_bracket_open || sym[is].type == sym_arg_open) && sym[is].match == -1)
		{
			bufprintf(comp_log, "%sUnmatched '(' at position %d%s\n", red_str, is, grey_str);
			goto invalid_expr;
		}

	// Turned implied multiplications into operators
	for (is=1; is < sym_count; is++)
	{
		if (sym[is-1].can_imply_mul_with_next && sym[is].can_imply_mul_with_prev)
		{
			// Move everything one place to make room for the operator
			alloc_enough(&sym, sym_count+1, &sym_as, sizeof(symbol_data_t), 2.);
			sym_move(sym, &sym_count, is+1, is);

			// Add operator
			bufprintf(comp_log, "Implied '*' operator added\n");
			sym[is].type = sym_operator;
			sym[is].p = "*";
			sym[is].p_len = 1;
			sym[is].operator_priority = 3;
			sym[is].can_imply_mul_with_prev = 0;
			sym[is].can_imply_mul_with_next = 0;
		}
	}

	int id, max_prio, max_prio_pos, result_id;
	size_t res_taken_count = sym_count;
	res_taken_r = calloc(res_taken_count, sizeof(int));		// result ID taken (0 or 1)
	res_taken_i = calloc(res_taken_count, sizeof(int));

	// Go through every depth
	for (id = max_depth; id >= 0; id--)
	{
		for (is = sym_count-1; is >= 0; is--)
		{
			// Process function calls
			if (sym[is].depth == id && sym[is].type == sym_function)
			{
				// Free result IDs that are about to be consumed
				for (i = is+1; i <= sym[is].match; i++)
				{
					if (sym[i].type == sym_result_real)
						res_taken_r[sym[i].result_id] = 0;

					if (sym[i].type == sym_result_int)
						res_taken_i[sym[i].result_id] = 0;
				}

				// Pick result ID
				result_id = find_res_taken(res_taken_r, res_taken_count);

				// Print start of command
				bufprintf(prog, "%c v%d = %.*s%s", var_decl, result_id, sym[is].p_len, sym[is].p, cmd_suffix);
				last_res_id = result_id;
				last_res_type = sym_result_real;

				// Go through all arguments and print them
				for (i = is+1; i <= sym[is].match; i++)
				{
					// Print values and variables as they were written
					if (sym[i].type == sym_value || sym[i].type == sym_variable)
						bufprintf(prog, " %.*s", sym[i].p_len, sym[i].p);

					// Print result variables
					if (sym[i].type == sym_result_real || sym[i].type == sym_result_int)
						bufprintf(prog, " %c%d", sym[i].type == sym_result_real ? 'v' : 'i', sym[i].result_id);
				}
				bufprintf(prog, "\n");

				// Remove everything that was just used from the sym array
				sym_move(sym, &sym_count, is+1, sym[is].match+1);

				// Turn command call into result
				sym[is].type = sym_result_real;
				sym[is].result_id = result_id;
			}

			// Process brackets that contain no operator (therefore contain only one symbol)
			if (sym[is].depth == id && sym[is].type == sym_bracket_open)
			{
				// Check that it contains only one symbol
				if (sym[is].match != is + 2)
				{
					bufprintf(comp_log, "%sBracket '%.*s' doesn't reduce to one symbol, an operator must be missing.%s\n", red_str, sym[sym[is].match].p_len + sym[sym[is].match].p - sym[is].p, sym[is].p, grey_str);
					goto invalid_expr;
				}

				// Move symbol to the position and depth of the bracket
				sym[is].type = sym[is+1].type;
				sym[is].result_id = sym[is+1].result_id;
				sym[is].p = sym[is+1].p;
				sym[is].p_len = sym[is+1].p_len;

				// Remove the two symbols
				sym_move(sym, &sym_count, is+1, is+3);
			}
		}

prio_loop_start:
		max_prio = 0;

		// Go through every symbol from the end to find the highest priority operator
		for (is = sym_count-1; is >= 0; is--)
		{
			if (sym[is].depth == id && sym[is].type == sym_operator && sym[is].operator_priority > max_prio)
			{
				max_prio = sym[is].operator_priority;
				max_prio_pos = is;
			}
		}

		// Process an operator
		if (max_prio > 0)
		{
			is = max_prio_pos;

			// Identify operator
			int is_comparison = 0;
			const char *identified_op=NULL, *identified_cmd=NULL;
			const char *op[] =  {  "==",   "<",    ">",    "<=",   ">=",   "!=",   "+",   "-",   "*",   "/",  "%",  "^" };
			const char *cmd[] = { "cmpr", "cmpr", "cmpr", "cmpr", "cmpr", "cmpr", "add", "sub", "mul", "div", "mod", "pow" };
			for (i=0; i < sizeof(op)/sizeof(*op); i++)
				if (compare_varlen_word_to_fixlen_word(sym[is].p, sym[is].p_len, op[i]))
				{
					identified_op = op[i];
					identified_cmd = cmd[i];
					is_comparison = (strcmp(identified_cmd, "cmpr") == 0);
					break;
				}

			// Make sure operator isn't the final symbol
			if (is == sym_count-1)
			{
				bufprintf(comp_log, "%sOperator '%.*s' is the final symbol.%s\n", red_str, sym[is].p_len, sym[is].p, grey_str);
				goto invalid_expr;
			}

			// Free result IDs that are about to be consumed
			for (i = is-1; i <= is+1; i+=2)
			{
				if (sym[i].type == sym_result_real)
					res_taken_r[sym[i].result_id] = 0;

				if (sym[i].type == sym_result_int)
					res_taken_i[sym[i].result_id] = 0;
			}

			// Pick result ID
			result_id = find_res_taken(is_comparison ? res_taken_i : res_taken_r, res_taken_count);

			// Print start of command
			if (is_comparison)
				bufprintf(prog, "i i%d = %s", result_id, identified_cmd);
			else
				bufprintf(prog, "%c v%d = %s%s", var_decl, result_id, identified_cmd, cmd_suffix);
			last_res_id = result_id;
			last_res_type = is_comparison ? sym_result_int : sym_result_real;

			// Go through all two arguments and print them
			for (i = is-1; i <= is+1; i+=2)
			{
				// Print values and variables as they were written
				if (sym[i].type == sym_value || sym[i].type == sym_variable)
					bufprintf(prog, " %.*s", sym[i].p_len, sym[i].p);

				// Print result variables
				if (sym[i].type == sym_result_real || sym[i].type == sym_result_int)
					bufprintf(prog, " %c%d", sym[i].type == sym_result_real ? 'v' : 'i', sym[i].result_id);

				// Print comparison operator
				if (is_comparison && i == is-1)
					bufprintf(prog, " %s", identified_op);
			}
			bufprintf(prog, "\n");

			// Replace the 3 symbols with 1 result symbol
			sym_move(sym, &sym_count, is, is+2);
			sym[is-1].type = is_comparison ? sym_result_int : sym_result_real;
			sym[is-1].result_id = result_id;
			sym[is-1].can_imply_mul_with_prev = 1;
			sym[is-1].can_imply_mul_with_next = 1;

			goto prio_loop_start;
		}
	}

	// Print return value
	if (last_res_type)
		bufprintf(prog, "%s %c%d\n", use_real ? "return_real" : "return", last_res_type == sym_result_real ? 'v' : 'i', last_res_id); 

invalid_expr:
	free(res_taken_r);
	free(res_taken_i);
	free(sym);

	return prog_s;
}