/**
 * @file        unittest_datareposrc.cc
 * @date        21 Apr 2023
 * @brief       Unit test for datareposrc
 * @see         https://github.com/nnstreamer/nnstreamer
 * @author      Hyunil Park <hyunil46.park@samsung.com>
 * @bug         No known bugs
 */

#include <gtest/gtest.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#include <unittest_util.h>

static const gchar filename[] = "mnist.data";
static const gchar json[] = "mnist.json";

/**
 * @brief Get file path
 */
static gchar *
get_file_path (const gchar *filename)
{
  const gchar *root_path = NULL;
  gchar *file_path = NULL;

  root_path = g_getenv ("NNSTREAMER_SOURCE_ROOT_PATH");

  /** supposed to run test in build directory */
  if (root_path == NULL)
    root_path = "..";

  file_path = g_build_filename (
      root_path, "tests", "test_models", "data", "datarepo", filename, NULL);

  return file_path;
}

/**
 * @brief Bus callback function
 */
static gboolean
bus_callback (GstBus *bus, GstMessage *message, gpointer data)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
    case GST_MESSAGE_ERROR:
      g_main_loop_quit ((GMainLoop *) data);
      break;
    default:
      break;
  }

  return TRUE;
}

/**
 * @brief Callback for tensor sink signal.
 */
static void
new_data_cb (GstElement *element, GstBuffer *buffer, gint *user_data)
{
  (*user_data)++;
  g_warning ("count:%d", *user_data);
  return;
}

/**
 * @brief create sparse tensors file
 */
static void
create_sparse_tensors_test_file ()
{
  GstBus *bus;
  GMainLoop *loop;
  gchar *file_path = get_file_path (filename);
  gchar *json_path = get_file_path (json);

  const gchar *str_pipeline = g_strdup_printf (
      "datareposrc location=%s json=%s start-sample-index=0 stop-sample-index=9 ! "
      "tensor_sparse_enc ! other/tensors,format=sparse,framerate=0/1 ! "
      "datareposink location=sparse.data json=sparse.json",
      file_path, json_path);

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
}

/**
 * @brief create flexible tensors file
 */
static void
create_flexible_tensors_test_file ()
{
  GstBus *bus;
  GMainLoop *loop;
  const gchar *str_pipeline
      = "videotestsrc num-buffers=3 ! videoconvert ! videoscale ! "
        "video/x-raw,format=RGB,width=176,height=144,framerate=10/1 ! tensor_converter ! join0.sink_0 "
        "videotestsrc num-buffers=3 ! videoconvert ! videoscale ! "
        "video/x-raw,format=RGB,width=320,height=240,framerate=10/1 ! tensor_converter ! join0.sink_1 "
        "videotestsrc num-buffers=3 ! videoconvert ! videoscale ! "
        "video/x-raw,format=RGB,width=640,height=480,framerate=10/1 ! tensor_converter ! join0.sink_2 "
        "join name=join0 ! other/tensors,format=flexible ! "
        "datareposink location=flexible.data json=flexible.json";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
}

/**
 * @brief create video test file
 */
static void
create_video_test_file ()
{
  GstBus *bus;
  GMainLoop *loop;
  const gchar *str_pipeline = "videotestsrc num-buffers=10 ! "
                              "datareposink location=video1.raw json=video1.json";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
}

/**
 * @brief create audio test file
 */
static void
create_audio_test_file ()
{
  GstBus *bus;
  GMainLoop *loop;
  const gchar *str_pipeline
      = "audiotestsrc samplesperbuffer=44100 num-buffers=1 ! "
        "audio/x-raw, format=S16LE, layout=interleaved, rate=44100, channels=1 ! "
        "datareposink location=audio1.raw json=audio1.json";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  loop = g_main_loop_new (NULL, FALSE);
  ASSERT_NE (pipeline, nullptr);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
}

/**
 * @brief create image test file
 */
static void
create_image_test_file ()
{
  GstBus *bus;
  GMainLoop *loop;
  const gchar *str_pipeline = "videotestsrc num-buffers=5 ! pngenc ! "
                              "datareposink location=img_%02d.png json=img.json";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
}

/**
 * @brief Test for reading image files
 */
TEST (datareposrc, readImageFiles)
{
  gchar *filename;
  gint buffer_count = 0, i;
  GCallback handler = G_CALLBACK (new_data_cb);
  GstElement *tensor_sink;
  GstBus *bus;
  GMainLoop *loop;
  const gchar *str_pipeline
      = "datareposrc location=img_%02d.png json=img.json start-sample-index=0 stop-sample-index=4 !"
        "pngdec ! tensor_converter ! tensor_sink name=tensor_sink0";

  create_image_test_file ();

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);

  EXPECT_NE (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  for (i = 0; i < 5; i++) {
    filename = g_strdup_printf ("img_%02d.png", i);
    g_remove (filename);
    g_free (filename);
  }
}

/**
 * @brief Test for reading a video raw file
 */
TEST (datareposrc, readVideoRaw)
{
  gint buffer_count = 0;
  GstElement *tensor_sink;
  GstBus *bus;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  const gchar *str_pipeline
      = "datareposrc location=video1.raw json=video1.json ! tensor_converter ! tensor_sink name=tensor_sink0";

  create_video_test_file ();

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  EXPECT_NE (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("video1.json");
  g_remove ("video1.raw");
}

/**
 * @brief Test for reading a video raw file
 */
TEST (datareposrc, readAudioRaw)
{
  gint buffer_count = 0;
  GstElement *tensor_sink;
  GstBus *bus;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  const gchar *str_pipeline
      = "datareposrc location=audio1.raw json=audio1.json ! tensor_converter ! tensor_sink name=tensor_sink0";

  create_audio_test_file ();

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);

  EXPECT_NE (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("audio1.json");
  g_remove ("audio1.raw");
}

/**
 * @brief Test for reading a file with invalid param (JSON path)
 */
TEST (datareposrc, invalidJsonPath0_n)
{
  GstElement *datareposrc = NULL;
  const gchar *str_pipeline = "datareposrc name=datareposrc ! fakesink";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "location", "video1.raw", NULL);
  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "json", NULL, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (JSON path)
 */
TEST (datareposrc, invalidJsonPath1_n)
{
  GstElement *datareposrc = NULL;
  const gchar *str_pipeline = "datareposrc name=datareposrc ! fakesink";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "location", "video1.raw", NULL);
  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "json", "no_search_file", NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (File path)
 */
TEST (datareposrc, invalidFilePath0_n)
{
  GstElement *datareposrc = NULL;
  const gchar *str_pipeline = "datareposrc name=datareposrc ! fakesink";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "json", "video1.json", NULL);
  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "location", NULL, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (File path)
 */
TEST (datareposrc, invalidFilePath1_n)
{
  GstElement *datareposrc = NULL;
  const gchar *str_pipeline = "datareposrc name=datareposrc ! fakesink";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "json", "video1.json", NULL);
  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "location", "no_search_file", NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (caps)
 */
TEST (datareposrc, invalidCapsWithoutJSON_n)
{
  GstElement *datareposrc = NULL;
  const gchar *str_pipeline = "datareposrc name=datareposrc ! fakesink";

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "location", "video1.raw", NULL);
  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "caps", NULL, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a tensors file
 * the number of total sample(mnist.data) is 10 (0~9)
 * the number tensors is 2 and indices (0,1), default is (0,1)
 * the default epochs is 1,
 * the default shuffle is TRUE.
 * can remove start-sample-index, epochs, tensors-sequence, shuffle property.
 */
TEST (datareposrc, readTensors)
{
  GstBus *bus;
  GMainLoop *loop;
  gchar *file_path = NULL;
  gchar *json_path = NULL;
  GstElement *datareposrc = NULL;
  gchar *get_str;
  guint get_value;

  loop = g_main_loop_new (NULL, FALSE);

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                                         "start-sample-index=0 stop-sample-index=9 epochs=2 tensors-sequence=0,1 ! "
                                         "fakesink",
      file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  EXPECT_NE (datareposrc, nullptr);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  g_object_get (datareposrc, "location", &get_str, NULL);
  EXPECT_STREQ (get_str, file_path);

  g_object_get (datareposrc, "json", &get_str, NULL);
  EXPECT_STREQ (get_str, json_path);

  g_object_get (datareposrc, "tensors-sequence", &get_str, NULL);
  EXPECT_STREQ (get_str, "0,1");

  g_object_get (datareposrc, "is-shuffle", &get_value, NULL);
  ASSERT_EQ (get_value, 1U);

  EXPECT_EQ (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  g_main_loop_run (loop);

  EXPECT_EQ (setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
  g_free (file_path);
  g_free (json_path);
}

/**
 * @brief Test for reading a file composed of flexible tensors
 * the default shuffle is TRUE.
 */
TEST (datareposrc, readFlexibleTensors)
{
  gint buffer_count = 0;
  GstElement *tensor_sink;
  GstBus *bus;
  const gchar *str_pipeline = NULL;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  str_pipeline = "datareposrc location=flexible.data json=flexible.json ! tensor_sink name=tensor_sink0";

  create_flexible_tensors_test_file ();
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  EXPECT_NE (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("flexible.json");
  g_remove ("flexible.data");
}

/**
 * @brief Test for reading a file composed of sparse tensors
 * the default shuffle is TRUE.
 */
TEST (datareposrc, readSparseTensors)
{
  GFile *file = NULL;
  GFileInfo *info = NULL;
  gint64 size, org_size = 31760;
  gint buffer_count = 0;
  GstElement *tensor_sink;
  GstBus *bus;
  const gchar *str_pipeline = NULL;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  str_pipeline
      = "datareposrc location=sparse.data json=sparse.json ! tensor_sparse_dec ! "
        "other/tensors, format=static, num_tensors=2, framerate=0/1, "
        "dimensions=1:1:784:1.1:1:10:1, types=\"float32,float32\" ! tee name= t "
        "t. ! queue ! filesink location=sample.data t. ! queue ! tensor_sink name=tensor_sink0";

  create_sparse_tensors_test_file ();
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);
  EXPECT_NE (buffer_count, 0);
  handler = NULL;

  file = g_file_new_for_path ("sparse.data");
  info = g_file_query_info (
      file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, NULL);
  size = g_file_info_get_size (info);
  g_object_unref (file);
  g_object_unref (info);
  EXPECT_LT (size, org_size);

  file = g_file_new_for_path ("sample.data");
  info = g_file_query_info (
      file, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, NULL);
  size = g_file_info_get_size (info);
  g_object_unref (file);
  g_object_unref (info);
  EXPECT_EQ (size, org_size);

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("sparse.json");
  g_remove ("sparse.data");
  g_remove ("sample.data");
}

/**
 * @brief Test for reading a tensors file with Caps property
 */
TEST (datareposrc, readTensorsNoJSONWithCapsParam)
{
  GstBus *bus;
  GMainLoop *loop;
  gchar *file_path = NULL;
  gchar *json_path = NULL;
  GstElement *datareposrc = NULL;
  gchar *get_str;
  guint get_value;

  loop = g_main_loop_new (NULL, FALSE);

  file_path = get_file_path (filename);

  gchar *str_pipeline = g_strdup_printf (
      "datareposrc name=datareposrc location=%s "
      "start-sample-index=0 stop-sample-index=9 epochs=2 tensors-sequence=0,1 "
      "caps =\"other/tensors, format=(string)static, framerate=(fraction)0/1, "
      "num_tensors=(int)2, dimensions=(string)1:1:784:1.1:1:10:1, types=(string)float32.float32\" ! "
      "fakesink",
      file_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  EXPECT_NE (datareposrc, nullptr);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  g_object_get (datareposrc, "location", &get_str, NULL);
  EXPECT_STREQ (get_str, file_path);

  g_object_get (datareposrc, "tensors-sequence", &get_str, NULL);
  EXPECT_STREQ (get_str, "0,1");

  g_object_get (datareposrc, "is-shuffle", &get_value, NULL);
  ASSERT_EQ (get_value, 1U);

  EXPECT_EQ (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  g_main_loop_run (loop);

  EXPECT_EQ (setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
  g_main_loop_unref (loop);
  g_free (file_path);
  g_free (json_path);
}

/**
 * @brief Test for reading a file with invalid param (start-sample-index)
 * the number of total sample(mnist.data) is 1000 (0~999)
 */
TEST (datareposrc, invalidStartSampleIndex0_n)
{
  GstElement *datareposrc = NULL;
  int idx_out_of_range = 1000;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline
      = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                         "stop-sample-index=9 epochs=2 tensors-sequence=0,1 ! fakesink",
          file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "start-sample-index", idx_out_of_range, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (start-sample-index)
 * the number of total sample(mnist.data) is 10 (0~9)
 */
TEST (datareposrc, invalidStartSampleIndex1_n)
{
  GstElement *datareposrc = NULL;
  gint idx_out_of_range = -1;
  guint get_value;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline
      = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                         "stop-sample-index=9 epochs=2 tensors-sequence=0,1 ! fakesink",
          file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "start-sample-index", idx_out_of_range, NULL);
  /** value "-1" of type 'gint' is invalid or out of range for property
     'start-sample-index' of type 'gint' default value is set */
  g_object_get (GST_OBJECT (datareposrc), "start-sample-index", &get_value, NULL);
  EXPECT_EQ (get_value, 0U);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (stop-sample-index)
 * the number of total sample(mnist.data) is 1000 (0~999)
 */
TEST (datareposrc, invalidStopSampleIndex0_n)
{
  GstElement *datareposrc = NULL;
  guint idx_out_of_range = 1000;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline
      = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                         "start-sample-index=0 epochs=2 tensors-sequence=0,1 ! fakesink",
          file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  g_object_set (GST_OBJECT (datareposrc), "stop-sample-index", idx_out_of_range, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (start-sample-index)
 * the number of total sample(mnist.data) is 10 (0~9)
 */
TEST (datareposrc, invalidStopSampleIndex1_n)
{
  GstElement *datareposrc = NULL;
  gint idx_out_of_range = -1;
  guint get_value;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline
      = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                         "start-sample-index=0 epochs=2 tensors-sequence=0,1 ! fakesink",
          file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "stop-sample-index", idx_out_of_range, NULL);
  /** value "-1" of type 'gint' is invalid or out of range for property
     'start-sample-index' of type 'gint' default value is set */
  g_object_get (GST_OBJECT (datareposrc), "stop-sample-index", &get_value, NULL);
  EXPECT_EQ (get_value, 0U);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (epochs)
 */
TEST (datareposrc, invalidEpochs0_n)
{
  GstElement *datareposrc = NULL;
  gint invalid_epochs = -1;
  guint get_value;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline = g_strdup_printf (
      "datareposrc name=datareposrc location=%s json=%s "
      "start-sample-index=0 stop-sample-index=9 tensors-sequence=0,1 ! fakesink",
      file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "epochs", invalid_epochs, NULL);
  /** value "-1" of type 'gint' is invalid or out of range for property
     'start-sample-index' of type 'gint' default value is set */
  g_object_get (GST_OBJECT (datareposrc), "epochs", &get_value, NULL);
  EXPECT_EQ (get_value, 1U);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (epochs)
 */
TEST (datareposrc, invalidEpochs1_n)
{
  GstElement *datareposrc = NULL;
  guint invalid_epochs = 0;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline = g_strdup_printf (
      "datareposrc name=datareposrc location=%s json=%s "
      "start-sample-index=0 stop-sample-index=9 tensors-sequence=0,1 ! fakesink",
      file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "epochs", invalid_epochs, NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file with invalid param (tensors-sequence)
 * the number tensors is 2 and indices (0,1)
 */
TEST (datareposrc, invalidTensorsSequence0_n)
{
  GstElement *datareposrc = NULL;
  gchar *file_path = NULL;
  gchar *json_path = NULL;

  file_path = get_file_path (filename);
  json_path = get_file_path (json);

  gchar *str_pipeline
      = g_strdup_printf ("datareposrc name=datareposrc location=%s json=%s "
                         "start-sample-index=0 stop-sample-index=9 ! fakesink",
          file_path, json_path);
  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  g_free (str_pipeline);
  g_free (file_path);
  g_free (json_path);
  ASSERT_NE (pipeline, nullptr);

  datareposrc = gst_bin_get_by_name (GST_BIN (pipeline), "datareposrc");
  ASSERT_NE (datareposrc, nullptr);

  /* set invalid param */
  g_object_set (GST_OBJECT (datareposrc), "tensors-sequence", "1,0,2", NULL);

  /* state chagne failure is expected */
  EXPECT_NE (setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT), 0);

  gst_object_unref (pipeline);
}

/**
 * @brief Test for reading a file composed of non-flexible tensors
 * the default shuffle is TRUE.
 */
TEST (datareposrc, readInvalidFlexibleTensors)
{
  gint buffer_count = 0;
  GstBus *bus;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  const gchar *str_pipeline
      = "datareposrc location=audio1.raw json=flexible.json ! tensor_sink name=tensor_sink0";
  GstElement *tensor_sink;

  create_flexible_tensors_test_file ();
  create_audio_test_file ();

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  /* EXPECT_EQ not checked due to internal data stream error */
  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);

  /* Internal data stream error */
  EXPECT_EQ (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("audio1.json");
  g_remove ("audio1.raw");
  g_remove ("flexible.json");
  g_remove ("flexible.data");
}

/**
 * @brief Test for reading a file composed of non-sparse tensors
 * the default shuffle is TRUE.
 */
TEST (datareposrc, readInvalidSparseTensors)
{
  gint buffer_count = 0;
  GstBus *bus;
  GMainLoop *loop;
  GCallback handler = G_CALLBACK (new_data_cb);
  const gchar *str_pipeline
      = "datareposrc location=audio1.raw json=sparse.json ! tensor_sink name=tensor_sink0";
  GstElement *tensor_sink;

  create_sparse_tensors_test_file ();
  create_audio_test_file ();

  GstElement *pipeline = gst_parse_launch (str_pipeline, NULL);
  ASSERT_NE (pipeline, nullptr);

  tensor_sink = gst_bin_get_by_name (GST_BIN (pipeline), "tensor_sink0");
  ASSERT_NE (tensor_sink, nullptr);

  g_signal_connect (tensor_sink, "new-data", (GCallback) handler, &buffer_count);

  loop = g_main_loop_new (NULL, FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  ASSERT_NE (bus, nullptr);
  gst_bus_add_watch (bus, bus_callback, loop);
  gst_object_unref (bus);

  /* EXPECT_EQ not checked due to internal data stream error */
  setPipelineStateSync (pipeline, GST_STATE_PLAYING, UNITTEST_STATECHANGE_TIMEOUT);
  g_main_loop_run (loop);

  setPipelineStateSync (pipeline, GST_STATE_NULL, UNITTEST_STATECHANGE_TIMEOUT);

  /* Internal data stream error */
  EXPECT_EQ (buffer_count, 0);
  handler = NULL;

  gst_object_unref (tensor_sink);
  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  g_remove ("audio1.json");
  g_remove ("audio1.raw");
  g_remove ("sparse.json");
  g_remove ("sparse.data");
}

/**
 * @brief Main GTest
 */
int
main (int argc, char **argv)
{
  int result = -1;

  try {
    testing::InitGoogleTest (&argc, argv);
  } catch (...) {
    g_warning ("catch 'testing::internal::<unnamed>::ClassUniqueToAlwaysTrue'");
  }

  gst_init (&argc, &argv);

  try {
    result = RUN_ALL_TESTS ();
  } catch (...) {
    g_warning ("catch `testing::internal::GoogleTestFailureException`");
  }

  return result;
}
