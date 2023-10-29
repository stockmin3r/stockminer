static uv_loop_t  default_loop_struct;
static uv_loop_t *default_loop_ptr;

uv_loop_t *uv_default_loop(void) {
	if (default_loop_ptr != NULL)
		return default_loop_ptr;

	if (uv_loop_init(&default_loop_struct))
		return NULL;

	default_loop_ptr = &default_loop_struct;
	return default_loop_ptr;
}


uv_loop_t *uv_loop_new(void) {
	uv_loop_t *loop;

	loop = uv__malloc(sizeof(*loop));
	if (loop == NULL)
		return NULL;

	if (uv_loop_init(loop)) {
		uv__free(loop);
		return NULL;
	}
	return loop;
}
