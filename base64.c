
/*
base64.cpp and base64.h

Copyright (C) 2004-2008 Ren� Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

Ren� Nyffenegger rene.nyffenegger@adp-gmbh.ch


***This source has been alterered.***
*/

#include "base64.h"

void base64_encode(unsigned char const* bytes_to_encode,unsigned int in_len, char *out) {
  //todo: test

  static const char * base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

  int outInd=0;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while(in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);

    if(i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4); i++) {
        out[outInd++]=base64_chars[char_array_4[i]];
      }

      i = 0;
    }
  }

  if(i) {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for(j = 0; (j < i + 1); j++) {
      out[outInd++]=base64_chars[char_array_4[j]];
    }

    while((i++ < 3)) {
      out[outInd++]='=';
    }
  }
}

void base64_decode(const char *encoded_string,int strlength,char *out) {
  static const char base64_chars2[256]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    62,
    0,0,0,
    63,52,53,54,55,56,57,58,59,60,61,
    0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    0,0,0,0,0,0,
    26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  int in_len = strlength;
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4],char_array_3[3];
  int outInd=0;

  while(in_len-- && (encoded_string[in_] != '=') ) {
    char_array_4[i++] = encoded_string[in_++];

    if(i ==4) {
      for(i = 0; i <4; i++) {
        char_array_4[i] = base64_chars2[char_array_4[i]];
      }

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for(i = 0; (i < 3); i++) {
        out[outInd++]=char_array_3[i];
      }

      i = 0;
    }
  }

  if(i) {
    for(j = i; j <4; j++) {
      char_array_4[j] = 0;
    }

    for(j = 0; j <4; j++) {
      char_array_4[j] = base64_chars2[char_array_4[j]];
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for(j = 0; (j < i - 1); j++) {
      out[outInd++]=char_array_3[j];
    }
  }
}

int base64_binary_size(int strlength) {
  return (strlength * 3) / 4;
}

int base64_str_size(int binsize) {
  return (binsize * 4) / 3;
}
