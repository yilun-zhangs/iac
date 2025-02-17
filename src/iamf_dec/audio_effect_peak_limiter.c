/*
BSD 3-Clause Clear License The Clear BSD License

Copyright (c) 2023, Alliance for Open Media.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file audio_effect_peak_limiter.c
 * @brief Peak limiter.
 * @version 0.1
 * @date Created 03/03/2023
 **/

#include "audio_effect_peak_limiter.h"

#include <math.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IAMF_debug.h"

static int init_default(AudioEffectPeakLimiter*);
static float compute_target_gain(AudioEffectPeakLimiter*, float);
inline static float curve_accel(float x);

AudioEffectPeakLimiter* audio_effect_peak_limiter_create(void) {
  return (AudioEffectPeakLimiter*)calloc(1, sizeof(AudioEffectPeakLimiter));
}

void audio_effect_peak_limiter_uninit(AudioEffectPeakLimiter* ths) {
#if USE_TRUEPEAK
  for (int c = 0; c < MAX_OUTPUT_CHANNELS; ++c) {
    audio_true_peak_meter_deinit(&ths->truePeakMeters[c]);
  }
#endif
}

void audio_effect_peak_limiter_destroy(AudioEffectPeakLimiter* ths) {
  audio_effect_peak_limiter_uninit(ths);
  if (ths) free(ths);
}

// threashold_db: Peak threshold in dB
// sample_rate : Sample rate of the samples
// num_channels: number of channels in frame
// atk_sec : attack duration in seconds
// rel_sec : release duration in seconds
// delay_size: number of samples in delay buffer
void audio_effect_peak_limiter_init(AudioEffectPeakLimiter* ths,
                                    float threashold_db, int sample_rate,
                                    int num_channels, float atk_sec,
                                    float rel_sec, int delay_size) {
  init_default(ths);

  ths->linearThreashold = pow(10, threashold_db / 20);
  ths->attackSec = atk_sec;
  ths->releaseSec = rel_sec;
  ths->incTC = (float)1 / (float)sample_rate;
  ths->numChannels = num_channels;

  ths->delaySize = delay_size;
  ths->padsize = delay_size;

  for (int channel = 0; channel < num_channels; channel++) {
    for (int i = 0; i < MAX_DELAYSIZE + 1; i++)
      ths->delayData[channel][i] = 0.0f;
  }
}

int audio_effect_peak_limiter_process_block(AudioEffectPeakLimiter* ths,
                                            float* inblock, float* outblock,
                                            int frame_size) {
  // Look ahead
  float peak;
  float channel_peak = 0.0f;
  float gain;
  int k;
  float peakMax = 0.0f;
  float* audioBlock = outblock;
  float out;
  int idx, pos;
#if USE_TRUEPEAK
  float data;
#endif

  if (!inblock) return (0);

#define DB_IDX(i) ((i) % ths->delaySize)

  for (k = 0; k < frame_size; k++) {
    peak = 0.0f;
    idx = k + ths->entryIndex;
#ifndef OLD_CODE
    if (ths->peak_pos < 0) {
#endif
      for (int i = 0; i < ths->delaySize; i++) {
        channel_peak = ths->peakData[DB_IDX(i + idx)];
        if (channel_peak > peak) {
          peak = channel_peak;
#ifndef OLD_CODE
          ths->peak_pos = DB_IDX(i + idx);
#endif
        }
      }

#ifndef OLD_CODE
    } else {
      peak = ths->peakData[ths->peak_pos];
    }

    ia_logt("index %d : peak value %f vs %f", k, peak,
            ths->peak_pos < 0 ? 0 : ths->peakData[ths->peak_pos]);
#else
    ia_logt("index %d : peak value %f", k, peak);
#endif
    gain = compute_target_gain(ths, peak);
    ia_logt("index %d : gain value %f", k, gain);
    peakMax = 0;

    for (int channel = 0; channel < ths->numChannels; channel++) {
      pos = channel * frame_size;
      if (ths->delaySize > 0) {
        out = ths->delayData[channel][DB_IDX(idx)] * gain;
#if USE_TRUEPEAK
        data =
#endif
            ths->delayData[channel][DB_IDX(idx)] = inblock[pos + k];
        audioBlock[pos + k] = out;
      } else {  // no delay mode
#if USE_TRUEPEAK
        data = inblock[pos + k];
#endif
        audioBlock[pos + k] = inblock[pos + k] * gain;
      }
#if USE_TRUEPEAK
      // compute true peak if you want
      ia_logt("data value %f", data);
      channel_peak = audio_true_peak_meter_next_true_peak(
          &ths->truePeakMeters[channel], data);
      channel_peak = fabs(channel_peak);
#else
      channel_peak = fabs(ths->delayData[channel][DB_IDX(idx)]);
#endif
      if (channel_peak > peakMax) peakMax = channel_peak;
    }

#ifndef OLD_CODE
    if (ths->peak_pos == DB_IDX(idx))
      ths->peak_pos = -1;
    else if (ths->peak_pos < 0 || ths->peakData[ths->peak_pos] < peakMax)
      ths->peak_pos = DB_IDX(idx);
#endif

    ths->peakData[DB_IDX(idx)] = peakMax;
    ia_logt("index %d : peak max value %.10f", k, peakMax);
  }

  if (ths->delaySize > 0) {
    ths->entryIndex = DB_IDX(ths->entryIndex + frame_size);
  }
  if (!ths->init) {
    if (ths->padsize >= frame_size) {
      ths->padsize -= frame_size;
      frame_size = 0;
    } else {
      int i = 0;
      for (int c = 0; c < ths->numChannels; c++) {
        pos = c * frame_size;
        for (int k = ths->padsize; k < frame_size; k++) {
          audioBlock[i++] = audioBlock[pos + k];
        }
      }
      frame_size -= ths->padsize;
      ths->padsize = 0;
      ths->init = 1;
    }
  }
  // transmit the block and release memory
  return (frame_size);
}

int audio_effect_peak_limiter_get_delay(AudioEffectPeakLimiter* ths) {
  if (ths) return ths->delaySize - ths->padsize;
  return 0;
}

int init_default(AudioEffectPeakLimiter* ths) {
  if (!ths) return -1;

  audio_effect_peak_limiter_uninit(ths);
  memset(ths, 0x00, sizeof(AudioEffectPeakLimiter));

  ths->currentGain = 1.0;
  ths->targetStartGain = -1.0;
  ths->targetEndGain = -1.0;
  ths->attackSec = -1.0;
  ths->releaseSec = -1.0;
  ths->currentTC = -1.0;

#ifndef OLD_CODE
  ths->peak_pos = -1;
#endif

#if USE_TRUEPEAK
  for (int c = 0; c < MAX_OUTPUT_CHANNELS; ++c) {
    audio_true_peak_meter_init(&ths->truePeakMeters[c]);
  }
#endif

  return 0;
}

float compute_target_gain(AudioEffectPeakLimiter* ths, float peak) {
  float acc_ratio = 0;
  float gain = 0;

  if (ths->currentTC != -1 && ths->currentTC < ths->attackSec) {
    ths->currentTC += ths->incTC;
    acc_ratio = curve_accel(ths->currentTC / ths->attackSec);
    gain = ths->targetStartGain -
           acc_ratio * (ths->targetStartGain - ths->targetEndGain);
    ths->currentGain = gain;
  } else if (ths->currentTC != -1 &&
             ths->currentTC < ths->releaseSec + ths->attackSec) {
    ths->currentTC += ths->incTC;
    acc_ratio =
        curve_accel((ths->currentTC - ths->attackSec) / ths->releaseSec);
    gain = ths->targetEndGain + acc_ratio * (1.0f - ths->targetEndGain);
    ths->currentGain = gain;
  } else {
    ths->currentGain = 1.0;
  }

  if (peak * ths->currentGain > ths->linearThreashold) {  // peak detect
    ths->targetStartGain = ths->currentGain;
    ths->targetEndGain = ths->linearThreashold / peak;
    ths->currentTC = 0.0f;
  }

  return ths->currentGain;
}

float curve_accel(float x) {  // x = 0.0, y = 0.0 --> x = 1.0, y = 1.0
  if (1.0 < x) return 1.0f;
  if (x < 0) return 0.0f;
  return 1.0f - powf(x - 1, 2.0);
}
