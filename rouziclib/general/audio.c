#ifndef RL_EXCL_THREADING

void audiosys_callback(void *userdata, uint8_t *stream, int stream_bytes)
{
	int ib;
	audiosys_t *sys = (audiosys_t *) userdata;
	double now = get_time_hr();

	memset(stream, 0, stream_bytes);

	rl_mutex_lock(&sys->mutex);

	for (ib=0; ib < sys->bus_count; ib++)
	{
		// Check expiry
		if (sys->bus[ib].on)
			if (now - sys->bus[ib].last_reg_time > sys->bus[ib].expiry_dur)
			{
				sys->bus[ib].on = 0;
				sys->bus[ib].callback(NULL, sys, ib, sys->bus[ib].data);	// deinitialisation signal to the callback (non-blocking)
			}

		if (sys->bus[ib].on)
		{
			// Lock mutex
			if (sys->bus[ib].use_mutex)
				rl_mutex_lock(&sys->bus[ib].mutex);

			// Resync stime (sample time) if needed
			if (sys->bus[ib].stime==0. || sys->bus[ib].stime + sys->sec_per_buf*2. < sys->bus[ib].last_reg_time)
			{
				if (sys->bus[ib].stime > 0)
					fprintf_rl(stdout, "audiosys_callback() resync: was off by %.5f sec\n", sys->bus[ib].stime - (sys->bus[ib].last_reg_time - sys->sec_per_buf));
				sys->bus[ib].stime = sys->bus[ib].last_reg_time - sys->sec_per_buf;
			}

			// Call the function
			sys->bus[ib].callback((float *) stream, sys, ib, sys->bus[ib].data);

			sys->bus[ib].stime += sys->sec_per_buf;

			// Unlock mutex
			if (sys->bus[ib].use_mutex)
				rl_mutex_unlock(&sys->bus[ib].mutex);
		}
	}

	rl_mutex_unlock(&sys->mutex);
}

#if defined(RL_SDL) && RL_SDL == 3
static void SDLCALL sdl_audiosys_stream_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	audiosys_t *sys = (audiosys_t *) userdata;
	(void) total_amount;

	// Ignore requests until the fixed callback buffer is ready
	if (additional_amount <= 0 || sys->callback_buffer==NULL || sys->callback_buffer_bytes <= 0)
		return;

	// Supply complete mixer buffers until the stream has enough input data
	while (additional_amount > 0)
	{
		audiosys_callback(sys, sys->callback_buffer, sys->callback_buffer_bytes);
		if (!SDL_PutAudioStreamData(stream, sys->callback_buffer, sys->callback_buffer_bytes))
		{
			fprintf_rl(stderr, "SDL_PutAudioStreamData failed: %s\n", SDL_GetError());
			return;
		}
		additional_amount -= sys->callback_buffer_bytes;
	}
}
#endif

int audiosys_bus_register(audiosys_bus_callback_t bus_callback, void *bus_data, int use_mutex, double expiry_dur)
{
	int ib;

	// Check if the bus is already registered
	for (ib=0; ib < audiosys.bus_count; ib++)
	{
		if (audiosys.bus[ib].data == bus_data)
		{
			audiosys.bus[ib].on = 1;
			audiosys.bus[ib].last_reg_time = get_time_hr();
			return ib;
		}
	}

	rl_mutex_lock(&audiosys.mutex);

	// Add new bus
	ib = audiosys.bus_count;
	alloc_enough(&audiosys.bus, audiosys.bus_count+=1, &audiosys.bus_as, sizeof(audiosys_bus_t), 2.);

	audiosys.bus[ib].on = 1;
	audiosys.bus[ib].callback = bus_callback;
	audiosys.bus[ib].data = bus_data;
	audiosys.bus[ib].use_mutex = use_mutex;
	if (use_mutex)
		rl_mutex_init(&audiosys.bus[ib].mutex);
	audiosys.bus[ib].expiry_dur = expiry_dur==0. ? 2. : expiry_dur;
	audiosys.bus[ib].last_reg_time = get_time_hr();

	rl_mutex_unlock(&audiosys.mutex);

	return ib;
}

void audiosys_bus_unregister(void *bus_data)
{
	int ib;

	rl_mutex_lock(&audiosys.mutex);

	// Find the bus and erase it
	for (ib=0; ib < audiosys.bus_count; ib++)
	{
		if (audiosys.bus[ib].data == bus_data)
		{
			// Deinit mutex
			if (audiosys.bus[ib].use_mutex)
				rl_mutex_destroy(&audiosys.bus[ib].mutex);

			// Deinit signal to the callback (blocking)
			audiosys.bus[ib].callback(NULL, &audiosys, -1, audiosys.bus[ib].data);

			// Remove bus from bus array
			memset(&audiosys.bus[ib], 0, sizeof(audiosys_bus_t));
			if (ib == audiosys.bus_count-1)
				audiosys.bus_count--;
			break;
		}
	}

	rl_mutex_unlock(&audiosys.mutex);
}

void sdl_audiosys_init(int def_buflen)
{
#ifdef RL_SDL
	SDL_AudioSpec audio_format={0}, obtained={0};
	int i, driver_count, driver_index=0, device_index=0;
	const char *driver_name=NULL, *device_name=NULL;

	// Release a previous device before rebuilding the shared audio system
	if (audiosys.device_id || audiosys.sdl_stream || audiosys.mutex_initialised)
		sdl_audiosys_quit();

	// Require at least one available SDL audio driver
	driver_count = SDL_GetNumAudioDrivers();
	if (driver_count <= 0)
	{
		fprintf_rl(stderr, "SDL reported no audio drivers: %s\n", SDL_GetError());
		return;
	}

	// Pick the default driver
	for (i=0; i < driver_count; i++)
		if (strcmp("wasapi", SDL_GetAudioDriver(i)))	// if the driver isn't called "wasapi"
		{
			driver_index = i;
			break;
		}
	driver_name = SDL_GetAudioDriver(driver_index);

	// Load preferred driver name from pref file
	if (pref_def.path)
		driver_name = pref_get_string(&pref_def, "Audio output:Preferred driver", driver_name);

	// Select the preferred driver only when a valid name is available
	if (driver_name)
		for (i=0; i < driver_count; i++)
			if (strstr_i(SDL_GetAudioDriver(i), driver_name))
			{
				driver_index = i;
				break;
			}

	// Init the driver
#if RL_SDL == 3
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	SDL_SetHint(SDL_HINT_AUDIO_DRIVER, SDL_GetAudioDriver(driver_index));
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf_rl(stderr, "SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n", SDL_GetError());
		return;
	}
#else
	SDL_AudioQuit();					// quit the current audio driver
	if (SDL_AudioInit(SDL_GetAudioDriver(driver_index)))
	{
		fprintf_rl(stderr, "SDL_AudioInit failed: %s\n", SDL_GetError());
		return;
	}
#endif

#if RL_SDL == 2
	int device_count = SDL_GetNumAudioDevices(0);

	// Load preferred output device name from pref file
	if (device_count > 0)
		device_name = SDL_GetAudioDeviceName(0, 0);
	if (pref_def.path)
		device_name = pref_get_string(&pref_def, "Audio output:Preferred device", device_name);

	if (device_name)
		for (i=0; i < device_count; i++)
			if (strstr_i(SDL_GetAudioDeviceName(i, 0), device_name))
			{
				device_index = i;
				break;
			}

	// Init the output device
	audio_format.freq = pref_get_double(&pref_def, "Audio output:Sample rate", 44100, " Hz");
	audio_format.format = AUDIO_F32;
	audio_format.channels = 2;
	audio_format.samples = pref_get_double(&pref_def, "Audio output:Buffer length", def_buflen ? def_buflen : audio_format.freq/50, " samples");
	audio_format.callback = audiosys_callback;
	audio_format.userdata = &audiosys;
	rl_mutex_init(&audiosys.mutex);
	audiosys.mutex_initialised = 1;

	audiosys.device_id = SDL_OpenAudioDevice(device_count > 0 ? SDL_GetAudioDeviceName(device_index, 0) : NULL, 0, &audio_format, &obtained, 0);
	if (audiosys.device_id==0)
	{
		fprintf_rl(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		sdl_audiosys_quit();
		return;
	}

	if (audio_format.format!=obtained.format)
		fprintf_rl(stderr, "sdl_audio_init(): sdlaudioformat.format > wanted : %6x\tobtained : %6x\n", audio_format.format, obtained.format);
	if (audio_format.channels!=obtained.channels)
		fprintf_rl(stderr, "sdl_audio_init(): sdlaudioformat.channels > wanted : %6d\tobtained : %6d\n", audio_format.channels, obtained.channels);

	audiosys.buffer_len = obtained.samples;
	audiosys.samplerate = obtained.freq;
	if (pref_def.path)
		pref_set_double(&pref_def, "Audio output:Buffer length", audiosys.buffer_len, " samples");

	audiosys.sec_per_buf = (double) audiosys.buffer_len / audiosys.samplerate;
	audiosys.sec_per_sample = 1. / audiosys.samplerate;
#else
	int device_count=0;
	SDL_AudioDeviceID selected_device = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
	SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&device_count);

	// Select the preferred physical playback device when it is still available
	if (devices && device_count > 0)
		device_name = SDL_GetAudioDeviceName(devices[0]);
	if (pref_def.path)
		device_name = pref_get_string(&pref_def, "Audio output:Preferred device", device_name);
	if (device_name)
		for (i=0; i < device_count; i++)
			if (strstr_i(SDL_GetAudioDeviceName(devices[i]), device_name))
			{
				selected_device = devices[i];
				break;
			}
	SDL_free(devices);

	// Configure the application side of the SDL 3 conversion stream
	audio_format.freq = pref_get_double(&pref_def, "Audio output:Sample rate", 44100, " Hz");
	audio_format.format = SDL_AUDIO_F32;
	audio_format.channels = 2;
	audiosys.buffer_len = pref_get_double(&pref_def, "Audio output:Buffer length", def_buflen ? def_buflen : audio_format.freq/50, " samples");
	audiosys.samplerate = audio_format.freq;
	audiosys.sec_per_buf = (double) audiosys.buffer_len / audiosys.samplerate;
	audiosys.sec_per_sample = 1. / audiosys.samplerate;
	audiosys.callback_buffer_bytes = audiosys.buffer_len * audio_format.channels * sizeof(float);
	audiosys.callback_buffer = calloc(1, audiosys.callback_buffer_bytes);
	if (audiosys.callback_buffer==NULL)
	{
		fprintf_rl(stderr, "Failed to allocate the SDL audio callback buffer\n");
		return;
	}
	rl_mutex_init(&audiosys.mutex);
	audiosys.mutex_initialised = 1;
	if (pref_def.path)
		pref_set_double(&pref_def, "Audio output:Buffer length", audiosys.buffer_len, " samples");

	// Open a paused playback stream that adapts the legacy mixer callback
	audiosys.sdl_stream = SDL_OpenAudioDeviceStream(selected_device, &audio_format, sdl_audiosys_stream_callback, &audiosys);
	if (audiosys.sdl_stream==NULL)
	{
		fprintf_rl(stderr, "SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError());
		sdl_audiosys_quit();
		return;
	}
	audiosys.device_id = SDL_GetAudioStreamDevice(audiosys.sdl_stream);
	if (SDL_GetAudioDeviceFormat(audiosys.device_id, &obtained, NULL))
		fprintf_rl(stdout, "SDL audio output: %d Hz, %d channels, format 0x%x\n", obtained.freq, obtained.channels, obtained.format);
	if (!SDL_ResumeAudioStreamDevice(audiosys.sdl_stream))
		fprintf_rl(stderr, "SDL_ResumeAudioStreamDevice failed: %s\n", SDL_GetError());
#endif

	#if RL_SDL == 2
	SDL_PauseAudioDevice(audiosys.device_id, 0);
	#endif
#endif
}

void sdl_audiosys_quit()
{
	int ib;

#ifdef RL_SDL
	#if RL_SDL == 3
	// Stop the SDL 3 callback before releasing mixer-owned state
	if (audiosys.sdl_stream)
		SDL_DestroyAudioStream(audiosys.sdl_stream);
	audiosys.sdl_stream = NULL;
	#else
	// Stop the SDL 2 callback before releasing mixer-owned state
	if (audiosys.device_id)
		SDL_CloseAudioDevice(audiosys.device_id);
	#endif
#endif

	// Release every registered bus after audio callbacks have stopped
	for (ib=0; ib < audiosys.bus_count; ib++)
		if (audiosys.bus[ib].callback)
		{
			audiosys.bus[ib].callback(NULL, &audiosys, -1, audiosys.bus[ib].data);
			if (audiosys.bus[ib].use_mutex)
				rl_mutex_destroy(&audiosys.bus[ib].mutex);
		}
	free_null(&audiosys.bus);
	free_null(&audiosys.callback_buffer);

	// Release the mixer lock only when initialization completed
	if (audiosys.mutex_initialised)
		rl_mutex_destroy(&audiosys.mutex);
	memset(&audiosys, 0, sizeof(audiosys));
}

#endif // RL_EXCL_THREADING
