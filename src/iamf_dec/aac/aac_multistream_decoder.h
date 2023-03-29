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
 * @file aac_multistream_decoder.h
 * @brief AAC decoder APIs.
 * @version 0.1
 * @date Created 03/03/2023
 **/


#ifndef _AAC_MULTISTREAM2_DECODER_H_
#define _AAC_MULTISTREAM2_DECODER_H_

#include <stdint.h>

typedef struct AACMSDecoder AACMSDecoder;

AACMSDecoder *aac_multistream_decoder_open(uint8_t *config, uint32_t size,
                                           int streams, int coupled_streams,
                                           uint32_t flags, int *error);

int aac_multistream_decode_list(AACMSDecoder *st, uint8_t *buffer[],
                                uint32_t len[], void *pcm, uint32_t frame_size);

void aac_multistream_decoder_close(AACMSDecoder *st);

#endif /* _AAC_MULTISTREAM2_DECODER_H_ */
