#ifdef RL_IMAGE_FILE

#define SIMPLEWEBP_DISABLE_STDIO
#define SIMPLEWEBP_IMPLEMENTATION
#include "orig/simplewebp.h"

#endif

int check_data_is_webp(const uint8_t *raw_data, size_t size)
{
	// Check the RIFF container and WebP signature
	return size >= 12 && memcmp(raw_data, "RIFF", 4) == 0 && memcmp(&raw_data[8], "WEBP", 4) == 0;
}

raster_t load_image_mem_libsimplewebp(uint8_t *raw_data, size_t size, const int mode)
{
	// Prepare an empty result
	raster_t im={0};

#ifdef RL_IMAGE_FILE
	// Declare the decoder state and output buffer
	simplewebp *decoder=NULL;
	simplewebp_error error;
	size_t width, height, pixel_count;
	uint8_t *rgba=NULL;

	// Load the WebP container from memory
	error = simplewebp_load_from_memory(raw_data, size, &decoder);
	if (error != SIMPLEWEBP_NO_ERROR)
	{
		// Report malformed or unsupported WebP data
		fprintf_rl(stderr, "SimpleWebP could not load image: %s\n", simplewebp_get_error_text(error));
		return im;
	}

	// Read and validate dimensions before narrowing them to raster coordinates
	simplewebp_get_dimensions(decoder, &width, &height);
	if (width == 0 || height == 0 || width > INT_MAX || height > INT_MAX || width > SIZE_MAX / height || width * height > SIZE_MAX / sizeof(srgb_t))
	{
		// Report dimensions that cannot fit the raster representation
		fprintf_rl(stderr, "SimpleWebP image dimensions are invalid or too large\n");
		simplewebp_unload(decoder);
		return im;
	}

	// Allocate the interleaved RGBA8 decode buffer
	pixel_count = width * height;
	rgba = malloc(pixel_count * sizeof(srgb_t));
	if (rgba == NULL)
	{
		// Report output allocation failure
		fprintf_rl(stderr, "Could not allocate SimpleWebP image buffer\n");
		simplewebp_unload(decoder);
		return im;
	}

	// Decode the image into the RGBA8 buffer
	error = simplewebp_decode(decoder, rgba, NULL);
	if (error != SIMPLEWEBP_NO_ERROR)
	{
		// Release partially decoded data after failure
		fprintf_rl(stderr, "SimpleWebP could not decode image: %s\n", simplewebp_get_error_text(error));
		free(rgba);
		simplewebp_unload(decoder);
		return im;
	}

	// Adopt the RGBA8 buffer when requested and derive the other raster modes
	im.dim = xyi((int) width, (int) height);
	if (mode & IMAGE_USE_SRGB)
		im.srgb = (srgb_t *) rgba;
	convert_image_srgb8(&im, rgba, mode);
	if (im.srgb != (srgb_t *) rgba)
		free(rgba);

	// Release the decoder after conversion
	simplewebp_unload(decoder);
#endif

	// Return the decoded raster or the empty fallback
	return im;
}
