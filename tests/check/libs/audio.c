/* GStreamer
 *
 * unit tests for audio support library
 *
 * Copyright (C) 2006 Tim-Philipp Müller <tim centricular net>
 * Copyright (C) 2011 Sebastian Dröge <sebastian.droege@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/check/gstcheck.h>

#include <gst/audio/audio.h>
#include <string.h>

static GstBuffer *
make_buffer (guint8 ** _data)
{
  GstBuffer *buf = gst_buffer_new ();
  guint8 *data = (guint8 *) g_malloc (1000);

  gst_buffer_append_memory (buf,
      gst_memory_new_wrapped (0, data, 1000, 0, 1000, data, g_free));
  if (_data)
    *_data = data;
  return buf;
}

static void
setup_segment (GstSegment * s, GstFormat fmt, guint64 start, guint64 stop,
    guint64 stream_time)
{
  gst_segment_init (s, fmt);
  s->start = start;
  s->stop = stop;
  s->time = stream_time;
}

GST_START_TEST (test_buffer_clip_unsupported_format)
{
  GstSegment s;
  GstBuffer *buf;
  guint8 *data;

  /* If the format is not TIME or DEFAULT it should assert()
   * FIXME: check if return value is the same as the input buffer.
   *        probably can't be done because the assert() does a SIGABRT.
   */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_PERCENT, 0, 10, 0);

  GST_BUFFER_TIMESTAMP (buf) = 0 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 0;
  GST_BUFFER_OFFSET (buf) = GST_BUFFER_OFFSET_NONE;
  GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET_NONE;

  ASSERT_CRITICAL (gst_audio_buffer_clip (buf, &s, 100, 1));

  gst_buffer_unref (buf);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_start_and_stop)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip start and end */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_TIME, 4 * GST_SECOND, 8 * GST_SECOND,
      4 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 400);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 800);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 400);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_start)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip only start */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_TIME, 4 * GST_SECOND, 12 * GST_SECOND,
      4 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 8 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 400);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 1200);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 800);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_stop)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip only stop */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_TIME, 2 * GST_SECOND, 10 * GST_SECOND,
      2 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 2 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 8 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 200);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 1000);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data);
  fail_unless (map.size == 800);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_outside)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;

  /* Buffer outside segment */
  buf = make_buffer (NULL);
  setup_segment (&s, GST_FORMAT_TIME, 12 * GST_SECOND, 20 * GST_SECOND,
      12 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret == NULL);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_start_and_stop_no_meta)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip start and end but don't touch duration, offset and offset_end */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_TIME, 4 * GST_SECOND, 8 * GST_SECOND,
      4 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_OFFSET (buf) = GST_BUFFER_OFFSET_NONE;
  GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET_NONE;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), GST_CLOCK_TIME_NONE);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), GST_BUFFER_OFFSET_NONE);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret),
      GST_BUFFER_OFFSET_NONE);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 400);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_no_timestamp)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;

  /* If the buffer has no timestamp it should assert()
   * FIXME: check if return value is the same as the input buffer.
   *        probably can't be done because the assert() does a SIGABRT.
   */
  buf = make_buffer (NULL);
  setup_segment (&s, GST_FORMAT_TIME, 0 * GST_SECOND, 10 * GST_SECOND,
      0 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_OFFSET (buf) = GST_BUFFER_OFFSET_NONE;
  GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET_NONE;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret == buf);

  gst_buffer_unref (buf);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_time_handles_rounding)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;
  GstClockTime time_per_sample = GST_SECOND / 100;

  /* Clip only stop */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_TIME, 2 * GST_SECOND, 10 * GST_SECOND,
      2 * GST_SECOND);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  /* the max duration that still converts back to the same size of samples */
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND + (time_per_sample - 1);
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 2 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 8 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 200);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 1000);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data);
  fail_unless (map.size == 800);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_start_and_stop)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip start and end */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_DEFAULT, 400, 800, 400);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 400);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 800);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 400);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_start)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip only start */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_DEFAULT, 400, 1200, 400);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 8 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 400);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 1200);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 800);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_stop)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip only stop */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_DEFAULT, 200, 1000, 200);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 2 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), 8 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 200);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret), 1000);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data);
  fail_unless (map.size == 800);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_outside)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;

  /* Buffer outside segment */
  buf = make_buffer (NULL);
  setup_segment (&s, GST_FORMAT_DEFAULT, 1200, 2000, 1200);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = 10 * GST_SECOND;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = 1200;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret == NULL);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_start_and_stop_no_meta)
{
  GstSegment s;
  GstBuffer *buf;
  GstBuffer *ret;
  GstMapInfo map;
  guint8 *data;

  /* Clip start and end but don't touch duration and offset_end */
  buf = make_buffer (&data);
  setup_segment (&s, GST_FORMAT_DEFAULT, 400, 800, 400);

  GST_BUFFER_TIMESTAMP (buf) = 2 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_OFFSET (buf) = 200;
  GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET_NONE;

  ret = gst_audio_buffer_clip (buf, &s, 100, 1);
  fail_unless (ret != NULL);

  fail_unless_equals_int64 (GST_BUFFER_TIMESTAMP (ret), 4 * GST_SECOND);
  fail_unless_equals_int64 (GST_BUFFER_DURATION (ret), GST_CLOCK_TIME_NONE);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET (ret), 400);
  fail_unless_equals_int64 (GST_BUFFER_OFFSET_END (ret),
      GST_BUFFER_OFFSET_NONE);
  gst_buffer_map (ret, &map, GST_MAP_READ);
  fail_unless (map.data == data + 200);
  fail_unless (map.size == 400);
  gst_buffer_unmap (ret, &map);

  gst_buffer_unref (ret);
}

GST_END_TEST;

GST_START_TEST (test_buffer_clip_samples_no_timestamp)
{
  GstSegment s;
  GstBuffer *buf;

  /* If the buffer has no offset it should assert()
   * FIXME: check if return value is the same as the input buffer.
   *        probably can't be done because the assert() does a SIGABRT.
   */
  buf = make_buffer (NULL);
  setup_segment (&s, GST_FORMAT_DEFAULT, 0, 10, 0);

  GST_BUFFER_TIMESTAMP (buf) = 0 * GST_SECOND;
  GST_BUFFER_DURATION (buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_OFFSET (buf) = GST_BUFFER_OFFSET_NONE;
  GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET_NONE;

  ASSERT_CRITICAL (gst_audio_buffer_clip (buf, &s, 100, 1));

  gst_buffer_unref (buf);
}

GST_END_TEST;

GST_START_TEST (test_multichannel_checks)
{
  GstAudioChannelPosition pos_2_mixed[2] = {
    GST_AUDIO_CHANNEL_POSITION_MONO,
    GST_AUDIO_CHANNEL_POSITION_NONE
  };
  GstAudioChannelPosition pos_2_none[2] = {
    GST_AUDIO_CHANNEL_POSITION_NONE,
    GST_AUDIO_CHANNEL_POSITION_NONE
  };
  GstAudioChannelPosition pos_2_flr[2] = {
    GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
  };
  GstAudioChannelPosition pos_2_frl[2] = {
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT
  };
  GstAudioChannelPosition pos_2_frr[2] = {
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
  };
  GstAudioChannelPosition pos_3_flrc[3] = {
    GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER
  };
  GstAudioChannelPosition pos_3_frcl[3] = {
    GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
    GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
    GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT
  };
  GstAudioInfo info, info2;
  GstCaps *caps;

  gst_audio_info_init (&info);
  gst_audio_info_init (&info2);

  gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000, 2, NULL);
  fail_unless (memcmp (&info.position, pos_2_flr, sizeof (pos_2_flr)) == 0);

  gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000, 2, pos_2_flr);
  fail_unless (memcmp (&info.position, pos_2_flr, sizeof (pos_2_flr)) == 0);
  caps = gst_audio_info_to_caps (&info);
  fail_unless (gst_audio_info_from_caps (&info2, caps));
  fail_unless (memcmp (&info, &info2, sizeof (info)) == 0);
  gst_caps_unref (caps);

  gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000, 2, pos_2_none);
  fail_unless (memcmp (&info.position, pos_2_none, sizeof (pos_2_none)) == 0);
  caps = gst_audio_info_to_caps (&info);
  fail_unless (gst_audio_info_from_caps (&info2, caps));
  fail_unless (memcmp (&info, &info2, sizeof (info)) == 0);
  gst_caps_unref (caps);

  gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000, 3, pos_3_flrc);
  fail_unless (memcmp (&info.position, pos_3_flrc, sizeof (pos_3_flrc)) == 0);
  caps = gst_audio_info_to_caps (&info);
  fail_unless (gst_audio_info_from_caps (&info2, caps));
  fail_unless (memcmp (&info, &info2, sizeof (info)) == 0);
  gst_caps_unref (caps);

  ASSERT_WARNING (gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000,
          2, pos_2_frl));
  ASSERT_WARNING (gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000,
          2, pos_2_mixed));
  ASSERT_WARNING (gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000,
          2, pos_2_frr));
  ASSERT_WARNING (gst_audio_info_set_format (&info, GST_AUDIO_FORMAT_F32, 48000,
          3, pos_3_frcl));
}

GST_END_TEST;

typedef struct
{
  gint channels;
  GstAudioChannelPosition from[32], to[32];
  gint32 in[32], out[32];
  gboolean fail;
} MultichannelReorderData;

GST_START_TEST (test_multichannel_reorder)
{
  MultichannelReorderData tests[] = {
    {1,
          {GST_AUDIO_CHANNEL_POSITION_MONO},
          {GST_AUDIO_CHANNEL_POSITION_MONO},
          {0, 1, 2, 3},
          {0, 1, 2, 3},
        FALSE},
    {1,
          {GST_AUDIO_CHANNEL_POSITION_MONO},
          {GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER},
          {0, 1, 2, 3},
          {0, 1, 2, 3},
        TRUE},
    {2,
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT},
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT},
          {0, 1, 2, 3},
          {0, 1, 2, 3},
        FALSE},
    {2,
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT},
          {GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT},
          {0, 1, 2, 3},
          {1, 0, 3, 2},
        FALSE},
    {4,
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
              GST_AUDIO_CHANNEL_POSITION_REAR_CENTER},
          {GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
                GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
              GST_AUDIO_CHANNEL_POSITION_REAR_CENTER},
          {0, 1, 2, 3},
          {1, 2, 0, 3},
        FALSE},
    {4,
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
              GST_AUDIO_CHANNEL_POSITION_REAR_CENTER},
          {GST_AUDIO_CHANNEL_POSITION_REAR_CENTER,
                GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER},
          {0, 1, 2, 3},
          {3, 0, 1, 2},
        FALSE},
    {4,
          {GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
                GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
              GST_AUDIO_CHANNEL_POSITION_REAR_CENTER},
          {GST_AUDIO_CHANNEL_POSITION_REAR_CENTER,
                GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
                GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
              GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT},
          {0, 1, 2, 3},
          {3, 2, 1, 0},
        FALSE},
  };
  gint i;
  GstBuffer *buf;
  GstMapInfo map;

  for (i = 0; i < G_N_ELEMENTS (tests); i++) {
    buf =
        gst_buffer_new_wrapped_full (GST_MEMORY_FLAG_READONLY, tests[i].in,
        sizeof (tests[i].in), 0, sizeof (tests[i].in), NULL, NULL);

    if (tests[i].fail) {
      fail_if (gst_audio_buffer_reorder_channels (buf, GST_AUDIO_FORMAT_S32,
              tests[i].channels, tests[i].from, tests[i].to));
    } else {
      fail_unless (gst_audio_buffer_reorder_channels (buf, GST_AUDIO_FORMAT_S32,
              tests[i].channels, tests[i].from, tests[i].to));

      gst_buffer_map (buf, &map, GST_MAP_READ);
      fail_unless_equals_int (map.size, sizeof (tests[i].in));
      fail_unless (memcmp (tests[i].out, map.data, map.size) == 0);
      gst_buffer_unmap (buf, &map);
    }
    gst_buffer_unref (buf);
  }
}

GST_END_TEST;

GST_START_TEST (test_audio_format_s8)
{
  GstAudioFormat fmt;

  fmt = gst_audio_format_build_integer (TRUE, G_BYTE_ORDER, 8, 8);
  fail_unless (fmt == GST_AUDIO_FORMAT_S8);

}

GST_END_TEST;

GST_START_TEST (test_audio_format_u8)
{
  GstAudioFormat fmt;
  fmt = gst_audio_format_build_integer (FALSE, G_BYTE_ORDER, 8, 8);
  fail_unless (fmt == GST_AUDIO_FORMAT_U8);
}

GST_END_TEST;

GST_START_TEST (test_fill_silence)
{
  GstAudioInfo info;
  GstAudioFormat f;
  gint i;
  guint8 test_silence[32];

  for (f = GST_AUDIO_FORMAT_S8; f < GST_AUDIO_FORMAT_F64; f++) {
    gst_audio_info_set_format (&info, f, 48000, 1, NULL);

    gst_audio_format_fill_silence (info.finfo, test_silence,
        GST_AUDIO_INFO_BPF (&info) * 4);

    for (i = 0; i < 4; i++)
      fail_unless (memcmp (test_silence + i * GST_AUDIO_INFO_BPF (&info),
              info.finfo->silence, GST_AUDIO_INFO_BPF (&info)) == 0);
  }
}

GST_END_TEST;

GST_START_TEST (test_stream_align)
{
  GstAudioStreamAlign *align;
  guint i;
  GstClockTime timestamp;
  GstClockTime out_timestamp, out_duration;
  gboolean discont;

  align = gst_audio_stream_align_new (1000, 40 * GST_MSECOND, 1 * GST_SECOND);

  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
    fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    if (i == 0)
      fail_unless (discont);
    else
      fail_unless (!discont);
  }

  /* Drift forwards by 1ms per 10ms buffer for the first 40 buffers.
   * - after 40 buffers we're above alignment threshold
   * - after 40 + 100 buffers we're at discont wait
   */
  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;
    if (i > 0)
      timestamp += 1 * GST_MSECOND * MIN (i, 40);

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i < 140) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 0)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 140)
        fail_unless (discont);
      else
        fail_unless (!discont);
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Drift backwards by 1ms per 10ms buffer for the first 40 buffers.
   * - after 40 buffers we're above alignment threshold
   * - after 40 + 100 buffers we're at discont wait
   */
  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;
    if (i > 0)
      timestamp -= 1 * GST_MSECOND * MIN (i, 40);

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i < 140) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 0)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 140)
        fail_unless (discont);
      else
        fail_unless (!discont);

      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i - 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Shift all buffers but the first by 40ms
   * - after 1 buffers we're above alignment threshold
   * - after 101 buffers we're at discont wait
   */
  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;
    if (i > 0)
      timestamp += 40 * GST_MSECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i < 101) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 0)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 101)
        fail_unless (discont);
      else
        fail_unless (!discont);
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Shift every second buffer by 40ms:
   * - never discont!
   */
  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;

    if (i % 2 == 0 && i > 0)
      timestamp += 40 * GST_MSECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
    fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    if (i == 0)
      fail_unless (discont);
    else
      fail_unless (!discont);
  }

  /* Shift every buffer 100 by 2: discont at buffer 200
   */
  for (i = 0; i < 500; i++) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 0;
    if (i >= 100)
      timestamp += 2 * GST_SECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i < 200) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 0)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 2 * GST_SECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 200)
        fail_unless (discont);
      else
        fail_unless (!discont);
    }
  }

  gst_audio_stream_align_free (align);
}

GST_END_TEST;

GST_START_TEST (test_stream_align_reverse)
{
  GstAudioStreamAlign *align;
  gint i;
  GstClockTime timestamp;
  GstClockTime out_timestamp, out_duration;
  gboolean discont;

  align = gst_audio_stream_align_new (-1000, 40 * GST_MSECOND, 1 * GST_SECOND);

  for (i = 499; i >= 0; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
    fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    if (i == 499)
      fail_unless (discont);
    else
      fail_unless (!discont);
  }

  /* Drift forwards by 1ms per 10ms buffer for the first 40 buffers.
   * - after 40 buffers we're above alignment threshold
   * - after 40 + 100 buffers we're at discont wait
   */
  for (i = 499; i >= 0; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;
    if (i < 499)
      timestamp += 1 * GST_MSECOND * MIN (499 - i, 40);

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i >= 500 - 140) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 499)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 499 - 140)
        fail_unless (discont);
      else
        fail_unless (!discont);
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Drift backwards by 1ms per 10ms buffer for the first 40 buffers.
   * - after 40 buffers we're above alignment threshold
   * - after 40 + 100 buffers we're at discont wait
   */
  for (i = 499; i >= 4; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;
    if (i < 499)
      timestamp -= 1 * GST_MSECOND * MIN (499 - i, 40);

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i >= 500 - 140) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 499)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 499 - 140)
        fail_unless (discont);
      else
        fail_unless (!discont);

      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i - 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Shift all buffers but the first by 40ms
   * - after 1 buffers we're above alignment threshold
   * - after 101 buffers we're at discont wait
   */
  for (i = 499; i >= 0; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;
    if (i < 499)
      timestamp += 40 * GST_MSECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i >= 500 - 101) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 499)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      if (i == 499 - 101)
        fail_unless (discont);
      else
        fail_unless (!discont);
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 40 * GST_MSECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    }
  }

  /* Shift every second buffer by 40ms:
   * - never discont!
   */
  for (i = 499; i >= 0; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;

    if (i % 2 == 0 && i < 499)
      timestamp += 40 * GST_MSECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
    fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
    if (i == 499)
      fail_unless (discont);
    else
      fail_unless (!discont);
  }

  /* Shift buffer 100 by 2: discont at buffer 200
   */
  for (i = 499; i >= 0; i--) {
    timestamp = 10 * GST_MSECOND * i;
    discont = i == 499;
    if (i < 500 - 100)
      timestamp += 2 * GST_SECOND;

    discont =
        gst_audio_stream_align_process (align, discont, timestamp, 10,
        &out_timestamp, &out_duration, NULL);

    if (i >= 500 - 200) {
      fail_unless_equals_uint64 (out_timestamp, 10 * GST_MSECOND * i);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 499)
        fail_unless (discont);
      else
        fail_unless (!discont);
    } else {
      fail_unless_equals_uint64 (out_timestamp,
          10 * GST_MSECOND * i + 2 * GST_SECOND);
      fail_unless_equals_uint64 (out_duration, 10 * GST_MSECOND);
      if (i == 499 - 200)
        fail_unless (discont);
      else
        fail_unless (!discont);
    }
  }

  gst_audio_stream_align_free (align);
}

GST_END_TEST;

static Suite *
audio_suite (void)
{
  Suite *s = suite_create ("audio support library");

  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_buffer_clip_unsupported_format);
  tcase_add_test (tc_chain, test_buffer_clip_time_start_and_stop);
  tcase_add_test (tc_chain, test_buffer_clip_time_start);
  tcase_add_test (tc_chain, test_buffer_clip_time_stop);
  tcase_add_test (tc_chain, test_buffer_clip_time_outside);
  tcase_add_test (tc_chain, test_buffer_clip_time_start_and_stop_no_meta);
  tcase_add_test (tc_chain, test_buffer_clip_time_no_timestamp);
  tcase_add_test (tc_chain, test_buffer_clip_time_handles_rounding);
  tcase_add_test (tc_chain, test_buffer_clip_samples_start_and_stop);
  tcase_add_test (tc_chain, test_buffer_clip_samples_start);
  tcase_add_test (tc_chain, test_buffer_clip_samples_stop);
  tcase_add_test (tc_chain, test_buffer_clip_samples_outside);
  tcase_add_test (tc_chain, test_buffer_clip_samples_start_and_stop_no_meta);
  tcase_add_test (tc_chain, test_buffer_clip_samples_no_timestamp);
  tcase_add_test (tc_chain, test_multichannel_checks);
  tcase_add_test (tc_chain, test_multichannel_reorder);
  tcase_add_test (tc_chain, test_audio_format_s8);
  tcase_add_test (tc_chain, test_audio_format_u8);
  tcase_add_test (tc_chain, test_fill_silence);
  tcase_add_test (tc_chain, test_stream_align);
  tcase_add_test (tc_chain, test_stream_align_reverse);

  return s;
}

GST_CHECK_MAIN (audio);
