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
 * @file IAMF_encoder.h
 * @brief Claimed IAMF encoder APIs
 * @version 0.1
 * @date Created 3/3/2023
**/

#ifndef IAMF_ENCODER_H
#define IAMF_ENCODER_H
#include <stdint.h>

#include "IAMF_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct IAMF_Encoder IAMF_Encoder;

typedef struct AmbisonicsMonoConfig {
  uint8_t output_channel_count;
  uint8_t substream_count;
  uint8_t channel_mapping[255];
} AmbisonicsMonoConfig;

typedef struct AmbisonicsProjectionConfig {
  uint8_t output_channel_count;
  uint8_t substream_count;
  uint8_t coupled_substream_count;
  uint16_t demixing_matrix[256];
} AmbisonicsProjectionConfig;

typedef struct AudioElementConfig {
  // channel-based
  IAChannelLayoutType layout_in;
  IAChannelLayoutType *layout_cb;

  // scene-baesd
  AmbisonicsMode ambisonics_mode;
  int input_channels;
  union {
    AmbisonicsMonoConfig ambisonics_mono_config;
    AmbisonicsProjectionConfig ambisonics_projection_config;
  };
} AudioElementConfig;

typedef struct MixPresentationAnnotations {
  char *mix_presentation_friendly_label;
} MixPresentationAnnotations;

typedef struct MixPresentationElementAnnotations {
  char *mix_presentation_friendly_label;
} MixPresentationElementAnnotations;

typedef struct Bs2127DirectSpeakersConfig {
  int distance_flag;
  int position_bounds_flag;
  int screen_edge_lock_azimuth_flag;
  int screen_edge_lock_elevation_flag;
  int num_channels;
  uint32_t distance[22];

  uint32_t azimuth_max[22];
  uint32_t azimuth_min[22];
  uint32_t elevation_max[22];
  uint32_t elevation_min[22];
  uint32_t distance_max[22];
  uint32_t distance_min[22];

  uint32_t screen_edge_lock_azimuth[22];
  uint32_t screen_edge_lock_elevation[22];
} Bs2127DirectSpeakersConfig;

typedef struct Bs2127HoaConfig {
  int distance_flag;
  int position_bounds_flag;
  int screen_edge_lock_azimuth_flag;
  int screen_edge_lock_elevation_flag;
  int num_channels;
  uint32_t distance[22];

  uint32_t azimuth_max[22];
  uint32_t azimuth_min[22];
  uint32_t elevation_max[22];
  uint32_t elevation_min[22];
  uint32_t distance_max[22];
  uint32_t distance_min[22];

  uint32_t screen_edge_lock_azimuth[22];
  uint32_t screen_edge_lock_elevation[22];
} Bs2127HoaConfig;

typedef struct {
  union {
    Bs2127DirectSpeakersConfig direct_speakers_config;
    Bs2127HoaConfig hoa_config;
  };
} RenderingConfig;

typedef struct {
  float start_point_value;
} StepParameterData;

typedef struct {
  float start_point_value;
  float end_point_value;
} LinearParameterData;

typedef struct {
  float start_point_value;
  float end_point_value;
  float control_point_value;
} BezierParameterData;

typedef struct {
  int animation_type;
  union {
    StepParameterData step_parameter_data;
    LinearParameterData linear_parameter_data;
    BezierParameterData bezier_parameter_data;
  };
} AnimatedParameterData;

typedef struct {
  /*
  mix_gain is a value in dBs. For mixing of the audio element, this gain shall
  be applied to all of channels of the audio element. It shall be stored in a
  16-bit, signed, two�s complement fixed-point value with 8 fractional bits
  (i.e. Q7.8 in [Q-Format]).
  */
  int time_base;
  float default_mix_gain;  // db

  uint32_t duration;
  uint32_t num_segments;
  uint32_t constant_segment_interval;

  uint32_t *segment_interval;                      // num_segments
  AnimatedParameterData *animated_parameter_data;  // num_segments
} ElementMixConfig;

typedef struct OutputMixConfig {
  int time_base;
  float default_mix_gain;  // db

  uint32_t duration;
  uint32_t num_segments;
  uint32_t constant_segment_interval;

  uint32_t *segment_interval;                      // num_segments
  AnimatedParameterData *animated_parameter_data;  // num_segments
} OutputMixConfig;

typedef struct {
  uint8_t info_type;
  int16_t integrated_loudness;
  int16_t digital_peak;
  int16_t true_peak;
} LoudnessInfo;

#define MAX_LOUDSPEAKERS_NUM 256  // TODO
typedef struct {
  /*
  Specifies a playback layout type that used for authoring this mix
  presentation. layout_type : Layout type 0      : NOT_DEFINED 1      :
  LOUDSPEAKERS_SP_LABEL 2      : LOUDSPEAKERS_SS_CONVENTION 3      : BINAURAL
  */
  uint32_t layout_type;

  /*
  Specify the number of loudspeakers.
  */
  uint32_t num_loudspeakers;

  /*
  Define the SP label as specified in [ITU2051-3].
  */
  uint32_t sp_label[MAX_LOUDSPEAKERS_NUM];

  /*
  Specify the sound system.
  sound_system : Sound system
  0            : A
  1            : B
  2            : C
  3            : D
  4            : E
  5            : F
  6            : G
  7            : H
  8            : I
  9            : J
  10~15     : Reserved
  */
  IAMF_SoundSystem sound_system;
} IAMFLayout;

#define MAX_MEASURED_LAYOUT_NUM 10
typedef struct MixPresentation {
  MixPresentationAnnotations mix_presentation_annotations;
  int num_sub_mixes;  // simple and base profile, set with 1

  int num_audio_elements;
  int audio_element_id[2];
  MixPresentationElementAnnotations mix_presentation_element_annotations[2];
  RenderingConfig rendering_config[2];
  ElementMixConfig element_mix_config[2];

  OutputMixConfig output_mix_config;

  int num_layouts;
  IAMFLayout loudness_layout[MAX_MEASURED_LAYOUT_NUM];
  LoudnessInfo loudness[MAX_MEASURED_LAYOUT_NUM];
} MixPresentation;

#define MAX_OBU_ID_NUM 100  // TODO
typedef struct {
  uint32_t global_offset;
  uint32_t num_obu_ids;
  uint32_t obu_id[MAX_OBU_ID_NUM];
  uint32_t obu_data_type[MAX_OBU_ID_NUM];
  uint32_t reinitialize_decoder[MAX_OBU_ID_NUM];
  uint32_t relative_offset[MAX_OBU_ID_NUM];
} SyncInfo;

#ifndef DEMIXING_MATRIX_SIZE_MAX
#define DEMIXING_MATRIX_SIZE_MAX (18 * 18 * 2)
#endif

typedef struct IAFrame {
  uint32_t num_samples_to_trim_at_start;
  uint32_t num_samples_to_trim_at_end;
  int element_id;
  int frame_size;
  void *pcm;
  struct IAFrame *next;
} IAFrame;

typedef struct IAPacket {
  int packet_size;
  unsigned char *data;
  int samples;
} IAPacket;

/**
 * @brief     Create IAMF encoder handler
 * @param     [in] Fs : sample rate.
 * @param     [in] bits_per_sample : bits indicating one sample,16, 24 and 32.
 * @param     [in] sample_format : sample format.
 * 0x01 indicates little-endian PCM sample format and 0x00 indicates big-endian
 * PCM sample format
 * @param     [in] codec_id : the codec id. 1:opus, 2:aac
 * @param     [in] frame_size : the frame size
 * @param     [out] error : the return error when create ia handle
 * @return    return IAMF encoder handler
 */
IAMF_Encoder *IAMF_encoder_create(int32_t Fs, int bits_per_sample,
                                  int sample_format,
                                  int codec_id,  // 1:opus, 2:aac
                                  int frame_size, int *error);

/**
 * @brief     Add one audio element into stream.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] element_type : the audio element type
 * @param     [in] element_config : the audio element configue.
 * @return    return audio element id.
 * @remark    Adjacent channel layouts of a scalable format (where CLn-1 is the
 * precedent channel layout and CLn is the next one) are only allowed as below,
 * where CLn = S(n).W(n).H(n) S(n-1) ? S(n) and W(n 1) ? W(n) and H(n 1) ? H(n)
 * except "S(n 1) = S(n) and W(n 1) = W(n) and H(n 1) = H(n)" NOTE: S(Surround
 * Channel), W(Subwoofer Channel), H(Height Channel) if [in]channel_layout_cb is
 * set with IA_CHANNEL_LAYOUT_COUNT, then it is non-scalable encoding.
 */
int IAMF_audio_element_add(IAMF_Encoder *ie, AudioElementType element_type,
                           AudioElementConfig element_config);

/**
 * @brief     Delete one audio element from stream.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] element_id : the audio element id
 * @return    @0: success,@others: fail
 */
void IAMF_audio_element_delete(IAMF_Encoder *ie, int element_id);

/**
 * @brief     Prepare the loudness measurement for presetting target layouts.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] mix_presentation : the mix_presentation struct
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_target_loudness_measure_start(
    IAMF_Encoder *ie, MixPresentation *mix_presentation);

/**
 * @brief     Loudness measuring for for presetting target layouts.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] mix_presentation : the mix_presentation struct]
 * @param     [in] frame : input audio frame struct.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_target_loudness_measure(IAMF_Encoder *ie,
                                         MixPresentation *mix_presentation,
                                         IAFrame *frame);

/**
 * @brief     Stop and output loudness handler for presetting target layouts.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] mix_presentation : the mix_presentation struct
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_target_loudness_measure_stop(
    IAMF_Encoder *ie, MixPresentation *mix_presentation);

/**
 * @brief     Set mix presentation.
 * @param     [in] ie : IAMF encoder handler.
 * @param     [in] mix_presentation : the mix_presentation struct
 * @return    @0: success,@others: fail
 */
void IAMF_encoder_set_mix_presentation(IAMF_Encoder *ie,
                                       MixPresentation mix_presentation);

/**
 * @brief     Clear all mix presentations.
 * @param     [in] ie : IAMF encoder handler.
 * @return    @0: success,@others: fail
 */
void IAMF_encoder_clear_mix_presentation(IAMF_Encoder *ie);

/*
Following 3 apis are used to implement Down-Mix Parameter Determination.
*/
/**
 * @brief     Prepare the starting of own-Mix Parameter Determination
 * @param     [in] ie : channel group encoder handle.
 * @param     [in] element_id : audio element id.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_dmpd_start(IAMF_Encoder *ie, int element_id);

/**
 * @brief     Down-Mix Parameter Determination pre-process.
 * @param     [in] ie : channel group encoder handle.
 * @param     [in] element_id : audio element id.
 * @param     [in] pcm : input pcm audio data.
 * @param     [in] frame_size : input pcm frame size.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_dmpd_process(IAMF_Encoder *ie, int element_id,
                              const int16_t *pcm, int frame_size);

/**
 * @brief     Stop the Down-Mix Parameter Determination pre-process.
 * @param     [in] ie : channel group encoder handle.
 * @param     [in] element_id : audio element id.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_dmpd_stop(IAMF_Encoder *ie, int element_id);

/*
Following 2 apis are used to calculate loundness and output_gain.
*/
/**
 * @brief     loudness &gain process for single scalable channle-based audio
 * element
 * @param     [in] ie : immersive audio encoder handle.
 * @param     [in] element_id : audio element id.
 * @param     [in] pcm : input pcm sample data.
 * @param     [in] frame_size : frame size of input pcm.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_scalable_loudnessgain_measure(IAMF_Encoder *ie, int element_id,
                                               const void *pcm, int frame_size);

/**
 * @brief     End the loudness &gain process and output the values
 * @param     [in] ie : immersive audio encoder handle.
 * @param     [in] element_id : audio element id.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_scalable_loudnessgain_stop(IAMF_Encoder *ie, int element_id);

/**
 * @brief     Calculate the reconstruct gain.
 * @param     [in] ie : immersive audio encoder handle.
 * @param     [in] element_id : audio element id.
 * @param     [in] pcm : input pcm sample data.
 * @param     [in] frame_size : frame size of input pcm.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_reconstruct_gain(IAMF_Encoder *ie, int element_id,
                                  const int16_t *pcm, int frame_size);

/**
 * @brief     Encode a IAMF frame.
 * @param     [in] ie : IAMF encoder handle.
 * @param     [in] frame : input audio frame struct.
 * @param     [in] iapkt : output packet struct.
 * @param     [in] max_data_bytes : Size of the allocated memory for the output
 * payload
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_encode(IAMF_Encoder *ie, const IAFrame *frame, IAPacket *iapkt,
                        int32_t max_data_bytes);

/**
 * @brief     Perform a CTL function on a  immersive audio encoder.
 * @param     [in] ie : immersive audio encoder handle.
 * @param     [in] element_id : audio element id.
 * @param     [in] request : control request.
 * @return    @0: success,@others: fail
 */
int IAMF_encoder_ctl(IAMF_Encoder *ie, int element_id, int request, ...);

/*
Following api is used in mp4/fmp4 encapsulation
*/
/**
 * @brief     Return IAMF encoder descriptor which is used in mp4 muxer
 * @param     [in] ie : IAMF encoder handle.
 * @param     [out] data : IAMF Descriptor OBUs, specified using the OBU syntax.
 * @param     [in] max_size : The max size of IAMF Descriptor OBUs.
 * @return    return Size of IAMF Descriptor OBUs
 */
int IAMF_encoder_get_descriptor(IAMF_Encoder *ie, uint8_t *data, int max_size);

/**
 * @brief     Free the iamf encoder handler.
 * @param     [in] ie : iamf encoder handler.
 */
void IAMF_encoder_destroy(IAMF_Encoder *ie);

#ifdef __cplusplus
}
#endif

#endif /* IAMF_ENCODER_H */
