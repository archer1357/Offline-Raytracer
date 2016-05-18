#ifndef BASE64_H
#define BASE64_H


#ifdef __cplusplus
extern "C" {
#endif

  void base64_encode(unsigned char const* bytes_to_encode,
                     unsigned int in_len, char *out);
  void base64_decode(const char *encoded_string,int strlength,char *out);

  int base64_binary_size(int strLength);
  int base64_str_size(int binarySize);

#ifdef __cplusplus
}
#endif

#endif
