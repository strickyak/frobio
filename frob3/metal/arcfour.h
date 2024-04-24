/*
 * From: nob...@vox.xs4all.nl (An0nYm0Us UsEr)
 * Newsgroups: sci.crypt
 * Subject: RC4 ?
 * Date: 13 Sep 1994 21:30:36 GMT
 * Organization: Global Anonymous Remail Services Ltd.
 * Lines: 83
 * Message-ID: <3555ls$fsv@news.xs4all.nl>
 * NNTP-Posting-Host: xs1.xs4all.nl
 * X-Comment: This message did not originate from the above address.
 * X-Comment: It was automatically remailed by an anonymous mailservice.
 * X-Comment: Info: us...@xs4all.nl, Subject: remailer-help
 * X-Comment: Please report inappropriate use to <ad...@vox.xs4all.nl>
 */

struct rc4_engine {
  unsigned char state[256];
  unsigned char x;
  unsigned char y;
};

void rc4_swap_byte(unsigned char *a, unsigned char *b) {
  unsigned char swapByte;

  swapByte = *a;
  *a = *b;
  *b = swapByte;
}

void rc4_init_engine(struct rc4_engine *engine) {
  byte *state = engine->state;
  short counter;
  for (counter = 0; counter < 256; counter++) state[counter] = counter;
  engine->x = 0;
  engine->y = 0;
}

void rc4_mix_key(unsigned char *key_data_ptr, int key_data_len,
                 struct rc4_engine *engine) {
  unsigned char swapByte;
  unsigned char index1;
  unsigned char index2;
  unsigned char *state;
  short counter;

  state = engine->state;
  index1 = 0;
  index2 = 0;
  for (counter = 0; counter < 256; counter++) {
    index2 = (key_data_ptr[index1] + state[counter] + index2) & 255;
    rc4_swap_byte(&state[counter], &state[index2]);

    index1 = (index1 + 1);
    if (index1 >= key_data_len) {
      index1 = 0;
    }
  }
}

void rc4(unsigned char *buffer_ptr, int buffer_len, struct rc4_engine *engine) {
  unsigned char x;
  unsigned char y;
  unsigned char *state;
  unsigned char xorIndex;
  short counter;

  x = engine->x;
  y = engine->y;

  state = &engine->state[0];
  for (counter = 0; counter < buffer_len; counter++) {
    x = (x + 1) & 255;
    y = (state[x] + y) & 255;
    rc4_swap_byte(&state[x], &state[y]);

    xorIndex = state[x] + (state[y]) & 255;

    buffer_ptr[counter] ^= state[xorIndex];
  }
  engine->x = x;
  engine->y = y;
}

/* Usage Suggestions:

struct rc4_engine Rc4Engine;

void Rc4RandomSetup() {
  prepare_key("RandomSetup", 12, &Rc4Engine);
}

byte Rc4RandomByte() {
  unsigned char buf[1] = {0};
  jerboa_random::rc4(buf, 1, &jerboa_random::Engine);
  return buf[0];
}

*/
