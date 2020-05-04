#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ao/ao.h>

#include "uadeconstants.h"
#include "audio.h"
#include "uade123.h"

ao_sample_format format;

static ao_device *libao_device = NULL;

static ao_option *options = NULL;

void audio_close(void)
{
  if (libao_device) {

    /* Work-around libao/alsa, sleep 10ms to drain audio stream. */
    if (uade_output_file_name[0] == 0)
      usleep(10000);

    ao_close(libao_device);
  }
}

void process_config_options(const struct uade_config *uc)
{
  char *s;
  char *key;
  char *value;

  if (uc->buffer_time > 0) {
      char val[32];
      /* buffer_time is given in milliseconds, so convert to microseconds */
      snprintf(val, sizeof val, "%d", 1000 * uc->buffer_time);
      ao_append_option(&options, "buffer_time", val);
  }

  format.bits = UADE_BYTES_PER_SAMPLE * 8;
  format.channels = UADE_CHANNELS; 
  format.rate = uc->frequency;
  format.byte_format = AO_FMT_NATIVE;
  
  // Added by Airmann
  // set channel matrix
  if (uc->use_quad_mode) {

    format.channels = 4;

    // Hint: multichannel assignment under Linux and in general is a mess.
    // There are so many factors/components: libao, drivers, multimedia players and so on.
    // Each component handles multichannel audio order slightly different:
    // some players just omit channels 3+4, others play them but treat them
    // as center channels etc. and thus distort the original stereo image.
    // Then, if you don't have a real multichannel soundcard, stereo downmix 
    // is handled differently and so on.
    // I tried several combinations as described in libao documentation, but each matrix
    // had it's disadvantages. It turned out that omitting any specific matrix
    // seems to be the best solution. A really suitable surround matrix for correct
    // Amiga stereo imaging (L=1+3, R=2+4) doesn't exist anyway, though  
    // classic quadrophonic matrix: L,R,BL,BR or L,R,CL,CR are theoretically useable.
    // Funny: it turned out, that Windows Media Player did the best job regarding
    // stereo imaging. It was the only player that did it correctly.
    format.matrix = NULL;
    //format.matrix = "L,R,BL,BR"; // quadrophonic default
  }
  else {
    format.matrix = "L,R";
  }

  s = (char *) uc->ao_options.o;
  while (s != NULL && *s != 0) {
    key = s;

    s = strchr(s, '\n');
    if (s == NULL)
      break;
    *s = 0;
    s++;

    value = strchr(key, ':');
    if (value == NULL) {
      fprintf(stderr, "uade: Invalid ao option: %s\n", key);
      continue;
    }
    *value = 0;
    value++;

    ao_append_option(&options, key, value);
  }
}

int audio_init(const struct uade_config *uc)
{
  int driver;

  if (uade_no_audio_output)
    return 1;

  process_config_options(uc);

  ao_initialize();

  if (uade_output_file_name[0]) {
    driver = ao_driver_id(uade_output_file_format[0] ? uade_output_file_format : "wav");
    if (driver < 0) {
      fprintf(stderr, "Invalid libao driver\n");
      return 0;
    }
    libao_device = ao_open_file(driver, uade_output_file_name, 1, &format, NULL);
  } else {
    driver = ao_default_driver_id();
    libao_device = ao_open_live(driver, &format, options);
  }

  if (libao_device == NULL) {
    fprintf(stderr, "Can not open ao device: %d\n", errno);
    return 0;
  }

  return 1;
}


int audio_play(unsigned char *samples, int bytes)
{
  if (libao_device == NULL)
    return bytes;

  /* ao_play returns 0 on failure */
  return ao_play(libao_device, (char *) samples, bytes);
}
