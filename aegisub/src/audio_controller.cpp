// Copyright (c) 2009, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_controller.cpp
/// @brief Manage open audio and abstract state away from display
/// @ingroup audio_ui
///


#include "config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#endif

#include "audio_controller.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/audio_player.h"
#include "audio_provider_dummy.h"



AudioController::AudioController()
: provider(0)
, player(0)
, playback_mode(PM_NotPlaying)
{
}


AudioController::~AudioController()
{
	CloseAudio();
}


void AudioController::OpenAudio(const wxString &url)
{
	CloseAudio();

	if (!url)
		throw Aegisub::InternalError(_T("AudioController::OpenAudio() was passed an empty string. This must not happen."));

	wxString path_part;
	AudioProvider *new_provider = 0;

	if (url.StartsWith(_T("dummy-audio:"), &path_part))
	{
		/*
		 * scheme            ::= "dummy-audio" ":" signal-specifier "?" signal-parameters
		 * signal-specifier  ::= "silence" | "noise" | "sine" "/" frequency
		 * frequency         ::= integer
		 * signal-parameters ::= signal-parameter [ "&" signal-parameters ]
		 * signal-parameter  ::= signal-parameter-name "=" integer
		 * signal-parameter-name ::= "sr" | "bd" | "ch" | "ln"
		 *
		 * Signal types:
		 * "silence", a silent signal is generated.
		 * "noise", a white noise signal is generated.
		 * "sine", a sine wave is generated at the specified frequency.
		 *
		 * Signal parameters:
		 * "sr", sample rate to generate signal at.
		 * "bd", bit depth to generate signal at (usually 16).
		 * "ch", number of channels to generate, usually 1 or 2. The same signal is generated
		 *       in every channel even if one would be LFE.
		 * "ln", length of signal in samples. ln/sr gives signal length in seconds.
		 */
	}
	else if (url.StartsWith(_T("video-audio:"), &path_part))
	{
		/*
		 * scheme      ::= "video-audio" ":" stream-type
		 * stream-type ::= "stream" | "cache"
		 *
		 * Stream types:
		 *
		 * "stream", the audio is streamed as required directly from the video provider,
		 * and cannot be used to drive an audio display. Seeking is unreliable.
		 *
		 * "cache", the entire audio is cached to memory or disk. Audio displays can be
		 * driven and seeking is reliable. Opening takes longer because the entire audio
		 * stream has to be decoded and stored.
		 */
	}
	else if (url.StartsWith(_T("file:"), &path_part))
	{
		/*
		 * scheme    ::= "file" ":" "//" file-system-path
		 *
		 * On Unix-like systems, the file system path is regular. On Windows-systems, the
		 * path uses forward slashes instead of back-slashes and the drive letter is
		 * preceded by a slash.
		 *
		 * URL-encoding??
		 */
	}
	else
	{
		/*
		 * Assume it's not a URI but instead a filename in the platform's native format.
		 */
		wxFileName fn(url);
		if (!fn.FileExists())
			throw Aegisub::AudioOpenError(
				_T("Failed opening audio file (parsing as plain filename)"),
				&(Aegisub::FileNotFoundError(url)));
	}
}


void AudioController::CloseAudio()
{
	Stop();

	delete player;
	delete provider;
	player = 0;
	provider = 0;
	/// @todo tell all listeners about this
}


bool AudioController::IsAudioOpen() const
{
	return player && provider;
}


wxString AudioController::GetAudioURL() const
{
	/// @todo figure out how to get the url
	return _T("");
}


void AudioController::AddListener(AudioControllerEventListener *listener)
{
	listeners.insert(listener);
}


void AudioController::RemoveListener(AudioControllerEventListener *listener)
{
	listeners.erase(listener);
}


void AudioController::PlayRange(const AudioController::SampleRange &range)
{
	if (!IsAudioOpen()) return;
}


void AudioController::PlayToEnd(int64_t start_sample)
{
	if (!IsAudioOpen()) return;
}


void AudioController::Stop()
{
	if (!IsAudioOpen()) return;
}


bool AudioController::IsPlaying()
{
	return IsAudioOpen() && playback_mode != PM_NotPlaying;
}


void AudioController::ChangePlaybackEnd(int64_t end_sample)
{
	if (!IsAudioOpen()) return;
	if (playback_mode != PM_Range) return;
}


int64_t AudioController::GetPlaybackPosition()
{
	if (!IsPlaying()) return 0;
	return 0;
}


int64_t AudioController::SamplesFromMilliseconds(int64_t ms) const
{
	/// @todo There might be some subtle rounding errors here.

	if (!provider) return 0;

	int64_t sr = provider->GetSampleRate();

	int64_t millisamples = ms * sr;

	return (millisamples + 999) / 1000;
}


int64_t AudioController::MillisecondsFromSamples(int64_t samples) const
{
	/// @todo There might be some subtle rounding errors here.

	if (!provider) return 0;

	int64_t sr = provider->GetSampleRate();

	int64_t millisamples = samples * 1000;

	return millisamples / sr;
}

