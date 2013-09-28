// dxAudioCapture.cpp : Defines the entry point for the console application.
//
#error "error " NTDDI_VERSION
#include "pcc.h"

#define MAX_CAPTURE_GUIDS_COUNT (5)

#define SAMPLES_BUFFER_SIZE (65536)

short g_wav_data_buffer[SAMPLES_BUFFER_SIZE];

waveFormat_2_textDescription_t const g_wave_format_table[] = {
	MAKE_WAVE_FORMAT(WAVE_INVALIDFORMAT, "invalid format "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_1M08, " 11.025 kHz, Mono,   8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_1S08, " 11.025 kHz, Stereo, 8-bit  "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_1M16, " 11.025 kHz, Mono,   16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_1S16, " 11.025 kHz, Stereo, 16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_2M08, " 22.05  kHz, Mono,   8-bit  "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_2S08, " 22.05  kHz, Stereo, 8-bit  "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_2M16, " 22.05  kHz, Mono,   16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_2S16, " 22.05  kHz, Stereo, 16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_4M08, " 44.1   kHz, Mono,   8-bit  "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_4S08, " 44.1   kHz, Stereo, 8-bit  "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_4M16, " 44.1   kHz, Mono,   16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_4S16, " 44.1   kHz, Stereo, 16-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_44M08, " 44.1   kHz, Mono,   8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_44S08, " 44.1   kHz, Stereo, 8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_44M16, " 44.1   kHz, Mono,   16-bit"),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_44S16, " 44.1   kHz, Stereo, 16-bit"),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_48M08, " 48     kHz, Mono,   8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_48S08, " 48     kHz, Stereo, 8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_48M16, " 48     kHz, Mono,   16-bit"),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_48S16, " 48     kHz, Stereo, 16-bit"),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_96M08, " 96     kHz, Mono,   8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_96S08, " 96     kHz, Stereo, 8-bit "),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_96M16, " 96     kHz, Mono,   16-bit"),
	MAKE_WAVE_FORMAT(WAVE_FORMAT_96S16, " 96     kHz, Stereo, 16-bit"),
};

#if 0
#define WAVE_FORMAT_1M16       0x00000004       /* 11.025 kHz, Mono,   16-bit */
#define WAVE_FORMAT_1S16       0x00000008       /* 11.025 kHz, Stereo, 16-bit */
#define WAVE_FORMAT_2M08       0x00000010       /* 22.05  kHz, Mono,   8-bit  */
#define WAVE_FORMAT_2S08       0x00000020       /* 22.05  kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_2M16       0x00000040       /* 22.05  kHz, Mono,   16-bit */
#define WAVE_FORMAT_2S16       0x00000080       /* 22.05  kHz, Stereo, 16-bit */
#define WAVE_FORMAT_4M08       0x00000100       /* 44.1   kHz, Mono,   8-bit  */
#define WAVE_FORMAT_4S08       0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_4M16       0x00000400       /* 44.1   kHz, Mono,   16-bit */
#define WAVE_FORMAT_4S16       0x00000800       /* 44.1   kHz, Stereo, 16-bit */
#define WAVE_FORMAT_44M08      0x00000100       /* 44.1   kHz, Mono,   8-bit  */
#define WAVE_FORMAT_44S08      0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_44M16      0x00000400       /* 44.1   kHz, Mono,   16-bit */
#define WAVE_FORMAT_44S16      0x00000800       /* 44.1   kHz, Stereo, 16-bit */
#define WAVE_FORMAT_48M08      0x00001000       /* 48     kHz, Mono,   8-bit  */
#define WAVE_FORMAT_48S08      0x00002000       /* 48     kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_48M16      0x00004000       /* 48     kHz, Mono,   16-bit */
#define WAVE_FORMAT_48S16      0x00008000       /* 48     kHz, Stereo, 16-bit */
#define WAVE_FORMAT_96M08      0x00010000       /* 96     kHz, Mono,   8-bit  */
#define WAVE_FORMAT_96S08      0x00020000       /* 96     kHz, Stereo, 8-bit  */
#define WAVE_FORMAT_96M16      0x00040000       /* 96     kHz, Mono,   16-bit */
#define WAVE_FORMAT_96S16      0x00080000       /* 96     kHz, Stereo, 16-bit */
#endif


GUID g_capture_guids[MAX_CAPTURE_GUIDS_COUNT];
unsigned int g_capture_guids_count;
unsigned int g_selected_capture_device_idx;

BOOL CALLBACK callback_enumerate_capture_devices(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
	static __declspec(thread) char tmp_buffer[1024];
	if (NULL != lpGuid)
	{
		CopyMemory(&g_capture_guids[g_capture_guids_count], lpGuid, sizeof(GUID));
		++g_capture_guids_count;
		printf("Module: %S, Description: %S\t\t\n", lpcstrDescription, lpcstrModule);
	}
	if (g_capture_guids_count < MAX_CAPTURE_GUIDS_COUNT)
		return TRUE;
	return FALSE;
}

void resample_11025_to_8000(short const * input_samples, short input_samples_size, 
		short * output_samples, short output_samples_size)
{
#define OUTPUT_SAMPLE_FACTOR 320
#define INPUT_SAMPLE_FACTOR 441
	short input_idx = 0;
	short output_idx = 0;
	short err = 0;
	for (; input_idx < input_samples_size && output_idx < output_samples_size; ++input_idx)
	{
		output_samples[output_idx] = input_samples[input_idx];
		if (2 * err < INPUT_SAMPLE_FACTOR - 2*OUTPUT_SAMPLE_FACTOR  )
			err = err + OUTPUT_SAMPLE_FACTOR;
		else
			err = err + OUTPUT_SAMPLE_FACTOR - INPUT_SAMPLE_FACTOR, ++output_idx;
	}
	return;
}

void ut_resamples_11025_to_8000(void)
{
#define INPUT_TABLE_SIZE (176)
	short samples_table[INPUT_TABLE_SIZE];
	short output_table[(int)(1+176.4*8000/11025.0)];
	size_t idx = 0;
	for (idx = 0; idx < INPUT_TABLE_SIZE; ++idx)
		samples_table[idx] = idx;
	resample_11025_to_8000(&samples_table[0], sizeof(samples_table)/sizeof(samples_table[0]), &output_table[0], sizeof(output_table)/sizeof(output_table[0]));
	for (idx = 0; idx < sizeof(output_table)/sizeof(output_table[0]); ++idx)
		printf("output_table[%d] = %d\n", idx, output_table[idx]);
}

HRESULT CreateCaptureBuffer(LPDIRECTSOUNDCAPTURE8 pDSC, 
		LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCB8)
{
	HRESULT hr;
	DSCBUFFERDESC               dscbd;
	LPDIRECTSOUNDCAPTUREBUFFER  pDSCB;
	WAVEFORMATEX                wfx =
	{WAVE_FORMAT_PCM, 1, 11025, 11025*2, 4, 16, 0 };
	// wFormatTag, nChannels, nSamplesPerSec, mAvgBytesPerSec,
	// nBlockAlign, wBitsPerSample, cbSize

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 11025;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.cbSize = 0;

	if ((NULL == pDSC) || (NULL == ppDSCB8)) 
		return E_INVALIDARG;
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = 0;
	dscbd.dwBufferBytes = SAMPLES_BUFFER_SIZE * sizeof(short);
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &wfx;
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;

	hr = pDSC->CreateCaptureBuffer(&dscbd, &pDSCB, NULL);

	if (SUCCEEDED(hr))
	{
		hr = pDSCB->QueryInterface(IID_IDirectSoundCaptureBuffer8, (LPVOID*)ppDSCB8);
		pDSCB->Release();  
	}
	else
	{
		fprintf(stderr, "%4.4u %s : 0x%8.8x\n", __LINE__, __FILE__, hr);
	}
	return hr;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr = CoInitialize(0);
	if (SUCCEEDED(hr))
	{
		hr = DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)&callback_enumerate_capture_devices, NULL);
	}
	if (g_capture_guids_count > 0)
	{
		IDirectSoundCapture8 * p_capture_itf = NULL;
		g_selected_capture_device_idx = 0;
		hr = DirectSoundCaptureCreate8(&g_capture_guids[g_selected_capture_device_idx], &p_capture_itf, NULL);
		if (SUCCEEDED(hr))
		{
			DSCCAPS dsCaps;
			ZeroMemory(&dsCaps, sizeof(dsCaps));
			dsCaps.dwSize = sizeof(dsCaps);
			hr = p_capture_itf->GetCaps(&dsCaps);
			if (SUCCEEDED(hr))
			{
				printf("dwFlags: ");
				if (dsCaps.dwFlags & DSCCAPS_CERTIFIED )
					printf("DSCAPS_CERTIFIED | ");
				if (dsCaps.dwFlags & DSCCAPS_EMULDRIVER)
					printf("DSCCAPS_EMULDRIVER | ");
				if (dsCaps.dwFlags & DSCCAPS_MULTIPLECAPTURE)
					printf("DSCCAPS_MULTIPLECAPTURE | ");
				printf("\n");
				size_t idx;
				printf("dwFormat: 0x%8.8x\n", dsCaps.dwFormats);
				for (idx = 0; idx < sizeof(g_wave_format_table)/sizeof(g_wave_format_table[0]); ++idx)
				{
					if (dsCaps.dwFormats & g_wave_format_table[idx].dwFormat_)
						printf("\t%s %s\n", g_wave_format_table[idx].tag_, g_wave_format_table[idx].text_desc_);
				}
				IDirectSoundCaptureBuffer8 * p_capture_buffer = NULL;
				hr = CreateCaptureBuffer(p_capture_itf, &p_capture_buffer);
				if (SUCCEEDED(hr))
				{
					/* Now we have a valid Capture buffer interface. We can now capture WAV data */
					DSCBCAPS dscbCaps;
					DWORD dwStatus = 0;
					WAVEFORMATEX wavFormatEx;
					ZeroMemory(&wavFormatEx, sizeof(wavFormatEx));
					dscbCaps.dwSize = sizeof(dscbCaps);
					p_capture_buffer->GetCaps(&dscbCaps);
					p_capture_buffer->GetFormat(&wavFormatEx, sizeof(wavFormatEx), NULL);
					p_capture_buffer->GetStatus(&dwStatus);
					p_capture_buffer->Start(0);
					do 
					{
						static unsigned int count = 0;
#if 1
						DWORD dwCapPos, dwReadPos;
						if (0 == (count & 0x0ff))
						{
							p_capture_buffer->GetCurrentPosition(&dwCapPos, &dwReadPos);
							fprintf(stdout, "%8.8x %8.8x \n", dwCapPos, dwReadPos);
						}
#endif
						p_capture_buffer->GetStatus(&dwStatus);
						if (0 == (DSCBSTATUS_CAPTURING & dwStatus))
							break;
						Sleep(1);
						++count;
					}while (1);
					LPVOID pAudioPtr1, pAudioPtr2;
					DWORD dwAudioBytes1, dwAudioBytes2;
					hr = p_capture_buffer->Lock(0, 0, &pAudioPtr1, &dwAudioBytes1, &pAudioPtr2, &dwAudioBytes2, DSCBLOCK_ENTIREBUFFER);
					CopyMemory(&g_wav_data_buffer[0], pAudioPtr1, dwAudioBytes1);
					{
						HANDLE hOutFile;
						hOutFile = CreateFile("caputre.dat", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE != hOutFile)
						{
							DWORD dwBytesWritten;
						 	WriteFile(hOutFile, &g_wav_data_buffer,  sizeof(g_wav_data_buffer), &dwBytesWritten, 	NULL);
							CloseHandle(hOutFile);
						}
					}
					p_capture_buffer->Release();
				}

			}
			p_capture_itf->Release();
		}
	}
	return SUCCEEDED(hr);
}
