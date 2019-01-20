#include <cstdio>
#include <string>
#include <cstring>

#include "sndfile.h"
#include "portaudio.h"

#define BUFFER_LEN 1024
#define SAMPLE_RATE (44100)

struct SoundData
{
	SNDFILE *file;
	SF_INFO info;
};

static int patestCallback(const void *inputBuffer,
						  void *outputBuffer,
						  unsigned long framesPerBuffer,
						  const PaStreamCallbackTimeInfo *timeInfo,
						  PaStreamCallbackFlags statusFlags,
						  void *userData)
{
	float *out;
	SoundData *p_data = (SoundData *)userData;
	sf_count_t num_read;

	out = (float *)outputBuffer;

	/* clear output buffer */
	memset(out, 0, sizeof(float) * framesPerBuffer * p_data->info.channels);

	/* read directly into output buffer */
	num_read = sf_read_float(p_data->file, out, framesPerBuffer * p_data->info.channels);

	/*  If we couldn't read a full frameCount of samples we've reached EOF */
	if (num_read < framesPerBuffer)
	{
		return paComplete;
	}

	return paContinue;
}

int main()
{
	/* Open the soundfile */
	SoundData mySound;
	std::string fileName = "servo.wav";

	mySound.file = sf_open(fileName.c_str(), SFM_READ, &mySound.info);
	if (sf_error(mySound.file) != SF_ERR_NO_ERROR)
	{
		fprintf(stderr, "%s\n", sf_strerror(mySound.file));
		fprintf(stderr, "File: %s\n", fileName.c_str());
		return -1;
	}

	PaError err = Pa_Initialize();
	if (err != paNoError)
	{
		printf("PortAudio init error: %s\n", Pa_GetErrorText(err));
		return -1;
	}

	PaStream *stream;
	/* Open an audio I/O stream. */
	err = Pa_OpenDefaultStream(&stream,
							   0,					  /* no input channels */
							   mySound.info.channels, /* stereo output */
							   paFloat32,			  /* 32 bit floating point output */
							   mySound.info.samplerate,
							   256,			   /* frames per buffer, i.e. the number
                                                   of sample frames that PortAudio will
                                                   request from the callback. Many apps
                                                   may want to use
                                                   paFramesPerBufferUnspecified, which
                                                   tells PortAudio to pick the best,
                                                   possibly changing, buffer size.*/
							   patestCallback, /* this is your callback function */
							   &mySound);	  /*This is a pointer that will be passed to
                                                   your callback*/
	if (err != paNoError)
	{
		printf("PortAudio stream error: %s\n", Pa_GetErrorText(err));
		return -1;
	}
	//do stuff

	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		printf("PortAudio startstream error: %s\n", Pa_GetErrorText(err));
	}

	// Sleep while busy

	while (Pa_IsStreamActive(stream))
	{
		Pa_Sleep(100);
	}

	err = Pa_StopStream(stream);
	if (err != paNoError)
	{
		printf("PortAudio stopstream error: %s\n", Pa_GetErrorText(err));
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError)
	{
		printf("PortAudio stream close error: %s\n", Pa_GetErrorText(err));
		return -1;
	}

	err = Pa_Terminate();
	if (err != paNoError)
	{
		printf("PortAudio terminate error: %s\n", Pa_GetErrorText(err));
		return -1;
	}

	printf("End Nominal\n");

	return 0;
}