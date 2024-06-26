size_t alloc_enough_pattern(void **buffer, size_t needed_count, size_t alloc_count, size_t size_elem, double inc_ratio, uint8_t pattern)
{
	size_t newsize;
	void *p;

	if (needed_count > alloc_count)
	{
		newsize = ceil((double) needed_count * inc_ratio);

		// Try realloc to the new larger size
		p = realloc(*buffer, newsize * size_elem);
		if (p == NULL)
		{
			fprintf_rl(stderr, "realloc(*buffer=%p, size=%zu) failed.\n", (void *) *buffer, newsize * size_elem);
			return alloc_count;
		}
		else
			*buffer = p;

		// Set the new bytes
		memset(&((uint8_t *)(*buffer))[alloc_count * size_elem], pattern, (newsize-alloc_count) * size_elem);

		alloc_count = newsize;
	}

	return alloc_count;
}

size_t alloc_enough_and_copy2(void **buffer, void *copy_src, size_t needed_count, size_t alloc_count, size_t size_elem, double inc_ratio)
{
	alloc_enough(buffer, needed_count, &alloc_count, size_elem, inc_ratio);
	memcpy(*buffer, copy_src, needed_count * size_elem);

	return alloc_count;
}

#ifndef RL_EXCL_THREADING
size_t alloc_enough_mutex2(void **buffer, size_t needed_count, size_t alloc_count, size_t size_elem, double inc_ratio, rl_mutex_t *mutex)
{
	if (needed_count > alloc_count && mutex)
	{
		rl_mutex_lock(mutex);
		alloc_count = alloc_enough_pattern(buffer, needed_count, alloc_count, size_elem, inc_ratio, 0x00);
		rl_mutex_unlock(mutex);
	}
	else
		alloc_count = alloc_enough_pattern(buffer, needed_count, alloc_count, size_elem, inc_ratio, 0x00);

	return alloc_count;
}
#endif

void free_null(void **ptr)
{
	if (ptr)
	{
		free(*ptr);
		*ptr = NULL;
	}
}

void **calloc_2d(const size_t ptr_count, const size_t size_buffers, const size_t size_elem)
{
	size_t i;
	void **array;

	array = calloc(ptr_count, sizeof(void *));

	for (i=0; i < ptr_count; i++)
		array[i] = calloc(size_buffers, size_elem);

	return array;
}

// 2D calloc using one large contiguous buffer and an array pointer. Must be freed with free_2d(array, 1);
void **calloc_2d_contig_fullarg(const size_t ptr_count, const size_t size_buffers, const size_t size_elem, const char *filename, const char *func, int line)
{
	size_t i;
	uint8_t **array;

#ifdef ADD_CITA_INFO
	ADD_CITA_INFO
#endif

	array = calloc(ptr_count, sizeof(void *));
	array[0] = calloc(ptr_count*size_buffers, size_elem);

#ifdef ADD_CITA_INFO
	cita_input_info = NULL;
#endif

	for (i=1; i < ptr_count; i++)
		array[i] = &array[0][i*size_buffers*size_elem];

	return array;
}

void **array_1d_to_2d_contig(void *array_1d, const size_t ptr_count, const size_t size_buffers)
{
	size_t i;
	uint8_t **array;

	array = calloc(ptr_count, sizeof(void *));
	array[0] = array_1d;

	for (i=1; i < ptr_count; i++)
		array[i] = &array[0][i*size_buffers];

	return array;
}

void **memcpy_2d(void **dst, void **src, const size_t ptr_count, const size_t size_buffers)
{
	size_t i;

	if (dst==NULL || src==NULL)
		return NULL;

	for (i=0; i < ptr_count; i++)
		memcpy(dst[i], src[i], size_buffers);

	return dst;
}

void **copy_2d(void **ptr, const size_t ptr_count, const size_t size_buffers)
{
	size_t i;
	void **array;

	array = calloc(ptr_count, sizeof(void *));
	if (array==NULL)
		return NULL;

	for (i=0; i < ptr_count; i++)
		array[i] = copy_alloc(ptr[i], size_buffers);

	return array;
}

void **copy_2d_contig(void **ptr, const size_t ptr_count, const size_t size_buffers, const size_t size_elem)
{
	void **array;

	array = calloc_2d_contig(ptr_count, size_buffers, size_elem);
	if (array==NULL)
		return NULL;

	memcpy_2d(array, ptr, ptr_count, size_buffers*size_elem);

	return array;
}

void **memset_2d(void **ptr, const int word, const size_t size, const size_t count)
{
	size_t i;

	for (i=0; i < count; i++)
		memset(ptr[i], word, size);

	return ptr;
}

void free_2d(void **ptr, const size_t count)
{
	size_t i;

	if (ptr==NULL)
		return ;

	for (i=0; i < count; i++)
		free(ptr[i]);

	free(ptr);
}

void free_null_2d(void ***ptr, const size_t count)
{
	if (ptr)
	{
		free_2d(*ptr, count);
		*ptr = NULL;
	}
}

void *copy_alloc(void *b0, size_t size)		// makes an allocated copy of a buffer
{
	void *b1;

	if (b0==NULL || size <= 0)
		return NULL;

	b1 = malloc(size);
	memcpy(b1, b0, size);

	return b1;
}

size_t next_aligned_offset(size_t offset, const size_t size_elem)
{
	if (offset & (size_elem-1))			// if the offset isn't aligned
		return (offset | (size_elem-1)) + 1;	// align it to the next size_elem-aligned offset
	else
		return offset;
}

void add_to_alloc_list(void *ptr, alloc_list_t *list)
{
	alloc_enough((void **) &list->list, list->len+=1, &list->as, sizeof(void *), 2.);	// enlarge list
	list->list[list->len-1] = ptr;
}

void *malloc_list(size_t size, alloc_list_t *list)	// add allocated buffers to a list
{
	void *ptr = malloc(size);

	add_to_alloc_list(ptr, list);

	return ptr;
}

void *calloc_list(size_t nitems, size_t size, alloc_list_t *list)	// add allocated buffers to a list
{
	void *ptr = calloc(nitems, size);

	add_to_alloc_list(ptr, list);

	return ptr;
}

void free_alloc_list(alloc_list_t *list)
{
	for (int i=0; i < list->len; i++)
		free_null(&list->list[i]);

	free(list->list);
	memset(list, 0, sizeof(alloc_list_t));
}
