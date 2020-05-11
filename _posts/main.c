#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>

main (int argc, char *argv[])
{
	if(argc <2) {
		printf("please enter filename.\n");
		exit(1);
	}
	printf("file is : %s\n",argv[1]);
	SF_INFO sf_info;
	SNDFILE* file_fd = sf_open(argv[1], SFM_READ, &sf_info);
	printf("read file succeed\n");

	int rate = 44100; //sf_info.samplerate; /* Sample rate */
	int exact_rate;   /* Sample rate returned by */
	/* snd_pcm_hw_params_set_rate_near */ 
	int dir;          /* exact_rate == rate --> dir = 0 */
	/* exact_rate < rate  --> dir = -1 */
	/* exact_rate > rate  --> dir = 1 */
	int periods = 2;       /* Number of periods */
	snd_pcm_uframes_t periodsize = 8192; /* Periodsize (bytes) */


	int i;
	int err;
	short buf[128];
	/* Handle for the PCM device */
	snd_pcm_t *playback_handle;

	/* Playback stream */
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

	/* This structure contains information about the hardware and can be used to specify the configuration to be used for PCM stream.*/
	snd_pcm_hw_params_t *hw_params;

	/* Name of the PCM device, like plughw:0,0 */
	/* The first number is the number of the soundcard, */
	/* The second number is the nu,ber of the device. */
	char* pcm_name;

	/* Init pcm_name. Of course, later you will make this configuralbe :-) */
	pcm_name = strdup("plughw:0,0");

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hw_params);
	
	/* Open PCM. The last parameter of this function is the mode. If this is set to 0, the standard mode is used. Possible other values are SND_PCM_NONBLOCK and SND_PCM_ASYNC. If SND_PCM_NONBLOCK is used read / write access to the PCM device will return immdediately. If SND_PCM_ASYNC is specified, SIGIO will be emitted whenever a period has been completel processed by soundcard. */

	if ((err = snd_pcm_open (&playback_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
				pcm_name,
				snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}

	/** some more explanation required */
	
	/* Set access type. This can be either SND_PCM_ACCESS_RAW_INTERLEAVED or SND_PCM_ACCESS_RW_NONINTERLEAVED. There are also access types for MMAPed access, but this is beyond the scope of this introduction */
	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
				snd_strerror (err));
		exit (1);
	}

	/* Set sample format */
	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
				snd_strerror (err));
		exit (1);
	}
	/* Set sample rate. If the exact rate is not supported by the hardware, use nearest possible rate.*/
	exact_rate =rate;
	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &exact_rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
				snd_strerror (err));
		exit (1);
	}
	if (rate != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware. \n ==> Using %d Hz instead.\n", rate, exact_rate);
	}

	/* Set number of channels */
	if (snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2) < 0) {
		fprintf(stderr, "Error setting channels.\n");
		exit(1);
	}
	
	/* Set number of periods. Periods used to be called fragments. */
	if (snd_pcm_hw_params_set_periods(playback_handle, hw_params, periods, 0) < 0) {
		fprintf(stderr, "Error seting periods.\n");
		exit(1);
	}

	/* Set buffer size (in frames). The resulting latency is given by latency = periodsize * periods / (rate * bytes_per_frame) */
	if (snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, (periodsize * periods) >> 2) < 0) {
		fprintf(stderr, "Error setting buffersize.\n");
		exit(1);
	}


	/* Apply HW parameter settings to PCM device and prepare device */
	if (snd_pcm_hw_params(playback_handle, hw_params) < 0) {
		fprintf(stderr, "Error setting HW params.\n");
		exit(1);
	}
	unsigned char *data;
	int pcmreturn, l1, l2;
	short s1, s2;
	int frames;
	int num_frames = 100;

	data = (unsigned char *)malloc(periodsize);
	frames = periodsize >> 2;
	for(l1 = 0; l1 < 100; l1++) {
		for(l2 = 0; l2 < num_frames; l2++) {
			s1 = (l2 % 128) * 100 - 5000;
			s2 = (l2 % 256) * 100 - 5000;
			data[4*l2] = (unsigned char)s1;
			data[4*l2+1] = s1 >> 8;
			data[4*l2+2] = (unsigned char)s2;
			data[4*l2+3] = s2 >> 8;
		}
		while ((pcmreturn = snd_pcm_writei(playback_handle, data, frames)) < 0) {
			snd_pcm_prepare(playback_handle);
			fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
		}
	}
	/*

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				snd_strerror (err));
		exit (1);
	}

	for (i = 0; i < 10; ++i) {
		if ((err = snd_pcm_writei (playback_handle, buf, 128)) != 128) {
			fprintf (stderr, "write to audio interface failed (%s)\n",
					snd_strerror (err));
			exit (1);
		}
	} */

	//*snd_pcm_close (playback_handle);
	printf("done \n");
	exit (0);
}
