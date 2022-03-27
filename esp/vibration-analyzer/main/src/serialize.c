#include "pipeline.h"
#include "peripheral.h"
#include "inertial_unit.h"


size_t stats_serialize(size_t timestamp, char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);

    mpack_build_map(&writer);
    mpack_write_cstr(&writer, "t");
    mpack_write_u32(&writer, timestamp);

    if (c->min) {
        mpack_write_cstr(&writer, "min");
        mpack_write_float(&writer, stats->min);
    }
    if (c->max) {
        mpack_write_cstr(&writer, "max");
        mpack_write_float(&writer, stats->max);
    }
    if (c->rms) {
        mpack_write_cstr(&writer, "rms");
        mpack_write_float(&writer, stats->rms);
    }
    if (c->mean) {
        mpack_write_cstr(&writer, "avg");
        mpack_write_float(&writer, stats->mean);
    }
    if (c->std) {
        mpack_write_cstr(&writer, "std");
        mpack_write_float(&writer, stats->std);
    }
    if (c->skewness) {
        mpack_write_cstr(&writer, "skew");
        mpack_write_float(&writer, stats->skew);
    }
    if (c->kurtosis) {
        mpack_write_cstr(&writer, "kurt");
        mpack_write_float(&writer, stats->kurtosis);
    }
    if (c->median) {
        mpack_write_cstr(&writer, "med");
        mpack_write_float(&writer, stats->median);
    }
    if (c->mad) {
        mpack_write_cstr(&writer, "mad");
        mpack_write_float(&writer, stats->mad);
    }

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}

size_t spectra_serialize(size_t timestamp, char *msg, size_t size, float *spectrum, size_t n, uint16_t fs)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);

    mpack_build_map(&writer);
    mpack_write_cstr(&writer, "t");
    mpack_write_u32(&writer, timestamp);
    mpack_write_cstr(&writer, "fs");
    mpack_write_u16(&writer, fs);

    mpack_write_cstr(&writer, "bins");
    mpack_start_array(&writer, n);
    for (size_t i = 0; i < n; i++)
        mpack_write_float(&writer, spectrum[i]);
    mpack_finish_array(&writer);

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}

static void events_serialize_by_type(SpectrumEventAction action, mpack_writer_t *writer, SpectrumEvent *events, size_t n)
{
    mpack_build_array(writer);
    for (uint16_t i = 0; i < n; i++) {
        if (events[i].action == action) {
            mpack_build_map(writer);
            mpack_write_cstr(writer, "t");
            mpack_write_u32(writer, events[i].start);
            mpack_write_cstr(writer, "d");
            mpack_write_u32(writer, events[i].duration);
            mpack_write_cstr(writer, "i");
            mpack_write_u16(writer, i);
            mpack_write_cstr(writer, "h");
            mpack_write_float(writer, events[i].amplitude);
            mpack_complete_map(writer);
        }
    }
    mpack_complete_array(writer);
}

size_t stream_serialize(char *msg, size_t size, float *stream, size_t n)
{
    mpack_writer_t writer;

    mpack_writer_init(&writer, msg, size);
    mpack_build_array(&writer);

    for (size_t i = 0; i < n; i++)
        mpack_write_float(&writer, stream[i]);

    mpack_complete_array(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}

size_t events_serialize(size_t timestamp, float bin_width, char *msg, size_t size, SpectrumEvent *events, size_t n)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);
    mpack_build_map(&writer);
    mpack_write_cstr(&writer, "t");
    mpack_write_u32(&writer, timestamp);
    mpack_write_cstr(&writer, "df");
    mpack_write_float(&writer, bin_width);

    mpack_write_cstr(&writer, "start");
    events_serialize_by_type(SPECTRUM_EVENT_START, &writer, events, n);
    mpack_write_cstr(&writer, "finish");
    events_serialize_by_type(SPECTRUM_EVENT_FINISH, &writer, events, n);

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}


static void conf_bool(mpack_reader_t *reader, bool *change, bool *field, bool x)
{
    if (mpack_reader_error(reader) == mpack_ok) {
        *field = x;
        *change = true;
    }
}

static void conf_u8(mpack_reader_t *reader, bool *change, uint8_t *field, uint8_t x)
{
    if (mpack_reader_error(reader) == mpack_ok) {
        *field = x;
        *change = true;
    }
}

static void conf_u16(mpack_reader_t *reader, bool *change, uint16_t *field, uint16_t x)
{
    if (mpack_reader_error(reader) == mpack_ok) {
        *field = x;
        *change = true;
    }
}

static void conf_int(mpack_reader_t *reader, bool *change, int *field, int x)
{
    if (mpack_reader_error(reader) == mpack_ok) {
        *field = x;
        *change = true;
    }
}

static void conf_float(mpack_reader_t *reader, bool *change, float *field, float x)
{
    if (mpack_reader_error(reader) == mpack_ok) {
        *field = x;
        *change = true;
    }
}

/* ------------------ CONFIGURATION ----------------- */
enum login_key_names {KEY_LOGIN_SSID, KEY_LOGIN_PASSWORD, KEY_LOGIN_MQTT_URL, KEY_LOGIN_COUNT};
static const char *login_keys[KEY_LOGIN_COUNT] = {"ssid", "pass", "url"};

size_t login_serialize(char *msg, size_t size, const Provisioning *conf)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);
    mpack_build_map(&writer);

    mpack_write_cstr(&writer, login_keys[KEY_LOGIN_SSID]);
    mpack_write_cstr(&writer, conf->wifi_ssid);
    mpack_write_cstr(&writer, login_keys[KEY_LOGIN_PASSWORD]);
    mpack_write_cstr(&writer, conf->wifi_pass);
    mpack_write_cstr(&writer, login_keys[KEY_LOGIN_MQTT_URL]);
    mpack_write_cstr(&writer, conf->mqtt_url);

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}

bool login_parse(char *msg, size_t size, Provisioning *conf)
{
    bool found[KEY_LOGIN_COUNT] = {0};
    mpack_reader_t reader;
    mpack_reader_init_data(&reader, msg, size);

    for (size_t i = mpack_expect_map_max(&reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(&reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(&reader, login_keys, found, KEY_LOGIN_COUNT)) {
            case KEY_LOGIN_SSID: 
                mpack_expect_cstr(&reader, conf->wifi_ssid, MAX_CREDENTIALS_LENGTH); break;
            case KEY_LOGIN_PASSWORD:
                mpack_expect_cstr(&reader, conf->wifi_pass, MAX_CREDENTIALS_LENGTH); break;
            case KEY_LOGIN_MQTT_URL:
                mpack_expect_cstr(&reader, conf->mqtt_url, MAX_CREDENTIALS_LENGTH); break;
            default: 
                mpack_discard(&reader); break;
        }
    }

    bool error = (mpack_reader_error(&reader) != mpack_ok);
    mpack_reader_destroy(&reader);
    return error;
}


static const char *sensor_range[IMU_RANGE_COUNT] = {"2g", "4g", "8g", "16g"};
enum sensor_key_names {KEY_FS, KEY_RANGE, KEY_N, KEY_OVERLAP, KEY_AXIS, KEY_SENSOR_COUNT};
static const char *sensor_keys[KEY_SENSOR_COUNT] = {"fs", "range", "n", "overlap", "axis"};

static void config_sensor_serialize(mpack_writer_t *writer, const SamplingConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, sensor_keys[KEY_FS]);
    mpack_write_u16(writer, conf->frequency);
    mpack_write_cstr(writer, sensor_keys[KEY_RANGE]);
    mpack_write_cstr(writer, sensor_range[conf->range]);
    mpack_write_cstr(writer, sensor_keys[KEY_N]);
    mpack_write_u16(writer, conf->n);
    mpack_write_cstr(writer, sensor_keys[KEY_OVERLAP]);
    mpack_write_float(writer, conf->overlap);

    mpack_write_cstr(writer, sensor_keys[KEY_AXIS]);
    mpack_build_array(writer);
    for (uint8_t i = 0; i < AXIS_COUNT; i++)
        mpack_write_bool(writer, conf->axis[i]);
    mpack_complete_array(writer);

    mpack_complete_map(writer);
}

static void config_sensor_parse(mpack_reader_t *reader, SamplingConfig *conf, bool *change)
{
    bool found[KEY_SENSOR_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, sensor_keys, found, KEY_SENSOR_COUNT)) {
            case KEY_FS:
                conf_u16(reader, change, &conf->frequency,
                         mpack_expect_u16_range(reader, 1, MAX_FREQUENCY));
                break;
            case KEY_RANGE:
                conf_int(reader, change, (int *)&conf->range,
                         mpack_expect_enum(reader, sensor_range, IMU_RANGE_COUNT));
                break;
            case KEY_N:
                conf_u16(reader, change, &conf->n, 
                         mpack_expect_u16_range(reader, 1, MAX_BUFFER_SAMPLES));
                break;
            case KEY_OVERLAP: 
                conf_float(reader, change, &conf->overlap, 
                           mpack_expect_float_range(reader, 0, MAX_OVERLAP));
                break;
            case KEY_AXIS:
                mpack_expect_array_match(reader, AXIS_COUNT);
                for (uint16_t i = 0; i < AXIS_COUNT && mpack_reader_error(reader) == mpack_ok; i++)
                    conf->axis[i] = mpack_expect_bool(reader);
                mpack_done_array(reader);
                if (mpack_reader_error(reader) == mpack_ok)
                    *change = true;
                break;
            default:
                mpack_discard(reader); break;
        }
    }
}

enum smooth_key_names {KEY_ENABLE, KEY_SMOOTH_N, KEY_REPEAT, KEY_SMOOTH_COUNT};
static const char *smooth_keys[KEY_SMOOTH_COUNT] = {"on", "n", "repeat"};

static void config_smooth_serialize(mpack_writer_t *writer, const SmoothingConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, smooth_keys[KEY_ENABLE]);
    mpack_write_bool(writer, conf->enable);
    mpack_write_cstr(writer, smooth_keys[KEY_SMOOTH_N]);
    mpack_write_u16(writer, conf->n);
    mpack_write_cstr(writer, smooth_keys[KEY_REPEAT]);
    mpack_write_u8(writer, conf->repeat);

    mpack_complete_map(writer);
}

static void config_smooth_parse(mpack_reader_t *reader, SmoothingConfig *conf, bool *change)
{
    bool found[KEY_SMOOTH_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, smooth_keys, found, KEY_SMOOTH_COUNT)) {
            case KEY_ENABLE:
                conf_bool(reader, change, &conf->enable, mpack_expect_bool(reader));
                break;
            case KEY_SMOOTH_N:
                conf_u16(reader, change, &conf->n,
                         mpack_expect_u16_range(reader, 1, MAX_BUFFER_SAMPLES));
                break;
            case KEY_REPEAT:
                conf_u8(reader, change, &conf->repeat, 
                        mpack_expect_u8_range(reader, 1, MAX_SMOOTH_REPEAT));
                break;
            default: 
                mpack_discard(reader); break;
        }
    }
}

enum stats_key_names {
    KEY_MIN, KEY_MAX, KEY_RMS, KEY_MEAN, KEY_VARIANCE,
    KEY_STD, KEY_SKEWNESS, KEY_KURTOSIS, KEY_MEDIAN, KEY_MAD,
    KEY_STATS_COUNT
};
static const char *stats_keys[KEY_STATS_COUNT] = {
    "min", "max", "rms", "avg", "var", "std", "skew", "kurtosis", "med", "mad"
};

static void config_stats_serialize(mpack_writer_t *writer, const StatisticsConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, stats_keys[KEY_MIN]);
    mpack_write_bool(writer, conf->min);
    mpack_write_cstr(writer, stats_keys[KEY_MAX]);
    mpack_write_bool(writer, conf->max);
    mpack_write_cstr(writer, stats_keys[KEY_RMS]);
    mpack_write_bool(writer, conf->rms);
    mpack_write_cstr(writer, stats_keys[KEY_MEAN]);
    mpack_write_bool(writer, conf->mean);
    mpack_write_cstr(writer, stats_keys[KEY_VARIANCE]);
    mpack_write_bool(writer, conf->variance);
    mpack_write_cstr(writer, stats_keys[KEY_STD]);
    mpack_write_bool(writer, conf->std);
    mpack_write_cstr(writer, stats_keys[KEY_SKEWNESS]);
    mpack_write_bool(writer, conf->skewness);
    mpack_write_cstr(writer, stats_keys[KEY_KURTOSIS]);
    mpack_write_bool(writer, conf->kurtosis);
    mpack_write_cstr(writer, stats_keys[KEY_MEDIAN]);
    mpack_write_bool(writer, conf->median);
    mpack_write_cstr(writer, stats_keys[KEY_MAD]);
    mpack_write_bool(writer, conf->mad);

    mpack_complete_map(writer);
}

static void config_stats_parse(mpack_reader_t *reader, StatisticsConfig *conf, bool *change)
{
    bool found[KEY_STATS_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, stats_keys, found, KEY_STATS_COUNT)) {
            case KEY_MIN: 
                conf_bool(reader, change, &conf->min, mpack_expect_bool(reader)); break;
            case KEY_MAX: 
                conf_bool(reader, change, &conf->max, mpack_expect_bool(reader)); break;
            case KEY_RMS: 
                conf_bool(reader, change, &conf->rms, mpack_expect_bool(reader)); break;
            case KEY_MEAN: 
                conf_bool(reader, change, &conf->mean, mpack_expect_bool(reader)); break;
            case KEY_VARIANCE: 
                conf_bool(reader, change, &conf->variance,mpack_expect_bool(reader)); break;
            case KEY_STD: 
                conf_bool(reader, change, &conf->std, mpack_expect_bool(reader)); break;
            case KEY_SKEWNESS: 
                conf_bool(reader, change, &conf->skewness, mpack_expect_bool(reader)); break;
            case KEY_KURTOSIS: 
                conf_bool(reader, change, &conf->kurtosis, mpack_expect_bool(reader)); break;
            case KEY_MEDIAN: 
                conf_bool(reader, change, &conf->median, mpack_expect_bool(reader)); break;
            case KEY_MAD: 
                conf_bool(reader, change, &conf->mad, mpack_expect_bool(reader)); break;
            default: 
                mpack_discard(reader); break;
        }
    }
}

static const char *transform_window[] = {"boxcar", "bartlett", "hann", "hamming", "blackman"};
static const char *transform_func[] = {"dft", "dct"};
enum transform_key_names {KEY_WINDOW, KEY_FUNC, KEY_LOG, KEY_TRANSFORM_COUNT};
static const char *transform_keys[KEY_TRANSFORM_COUNT] = {"w", "f", "log"};

static void config_transform_serialize(mpack_writer_t *writer, const FFTTransformConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, transform_keys[KEY_WINDOW]);
    mpack_write_cstr(writer, transform_window[conf->window]);
    mpack_write_cstr(writer, transform_keys[KEY_FUNC]);
    mpack_write_cstr(writer, transform_func[conf->func]);
    mpack_write_cstr(writer, transform_keys[KEY_LOG]);
    mpack_write_bool(writer, conf->log);

    mpack_complete_map(writer);
}

static void config_transform_parse(mpack_reader_t *reader, FFTTransformConfig *conf, bool *change)
{
    bool found[KEY_TRANSFORM_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, transform_keys, found, KEY_TRANSFORM_COUNT)) {
            case KEY_WINDOW:
                conf_int(reader, change, (int *)&conf->window,
                         mpack_expect_enum(reader, transform_window, WINDOW_TYPE_COUNT));
                break;
            case KEY_MAX:
                conf_int(reader, change, (int *)&conf->func,
                         mpack_expect_enum(reader, transform_func, TRANSFORM_COUNT));
                break;
            case KEY_LOG:
                conf_bool(reader, change, &conf->log, mpack_expect_bool(reader));
                break;
            default: 
                mpack_discard(reader); break;
        }
    }
}

enum logger_key_names {
    KEY_LOCAL_LOG, KEY_LOG_SAMPLES, KEY_LOG_STATS,
    KEY_LOG_SPECTRA, KEY_LOG_EVENTS, KEY_LOG_SUBSAMPLING, KEY_LOG_COUNT
};
static const char *logger_keys[KEY_LOG_COUNT] = {
    "local", "samples", "stats", "spectra", "events", "subsamp"
};

static void config_logger_serialize(mpack_writer_t *writer, const SaveFormatConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, logger_keys[KEY_LOCAL_LOG]);
    mpack_write_bool(writer, conf->openlog_raw_samples);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_SAMPLES]);
    mpack_write_bool(writer, conf->mqtt_samples);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_STATS]);
    mpack_write_bool(writer, conf->mqtt_stats);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_SPECTRA]);
    mpack_write_bool(writer, conf->mqtt_spectra);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_EVENTS]);
    mpack_write_bool(writer, conf->mqtt_events);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_SUBSAMPLING]);
    mpack_write_u16(writer, conf->subsampling);

    mpack_complete_map(writer);
}

static void config_logger_parse(mpack_reader_t *reader, SaveFormatConfig *conf, bool *change)
{
    bool found[KEY_LOG_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, logger_keys, found, KEY_LOG_COUNT)) {
            case KEY_LOCAL_LOG: 
                conf_bool(reader, change, &conf->openlog_raw_samples, mpack_expect_bool(reader)); break;
            case KEY_LOG_SAMPLES: 
                conf_bool(reader, change, &conf->mqtt_samples, mpack_expect_bool(reader)); break;
            case KEY_LOG_STATS: 
                conf_bool(reader, change, &conf->mqtt_stats, mpack_expect_bool(reader)); break;
            case KEY_LOG_SPECTRA:
                conf_bool(reader, change, &conf->mqtt_spectra, mpack_expect_bool(reader)); break;
            case KEY_LOG_EVENTS: 
                conf_bool(reader, change, &conf->mqtt_events, mpack_expect_bool(reader)); break;
            case KEY_LOG_SUBSAMPLING: 
                conf_u16(reader, change, &conf->subsampling, mpack_expect_u16(reader)); break;
            default: 
                mpack_discard(reader); break;
        }
    }
}

enum ev_threshold_names {KEY_THRESHOLD_LEVEL, KEY_THRESHOLD_COUNT};
static const char *ev_threshold_keys[KEY_THRESHOLD_COUNT] = {"t"};

static void config_ev_threshold_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_threshold_keys[KEY_THRESHOLD_LEVEL]);
    mpack_write_float(writer, conf->threshold.t);

    mpack_complete_map(writer);
}

static void config_ev_threshold_parse(mpack_reader_t *reader, EventDetectionConfig *conf, bool *change)
{
    bool found[KEY_THRESHOLD_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, ev_threshold_keys, found, KEY_THRESHOLD_COUNT)) {
            case KEY_THRESHOLD_LEVEL: 
                conf_float(reader, change, &conf->threshold.t, mpack_expect_float(reader)); break;
            default:
                mpack_discard(reader); break;
        }
    }
}

enum ev_neighbours_names {
    KEY_NEIGHBOURS_K, KEY_NEIGHBOURS_E, KEY_NEIGHBOURS_H,
    KEY_NEIGHBOURS_H_REL, KEY_NEIGHBOURS_COUNT
};
static const char *ev_neighbours_keys[KEY_NEIGHBOURS_COUNT] = {"k", "e", "h", "h_rel"};

static void config_ev_neighbours_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_neighbours_keys[KEY_NEIGHBOURS_K]);
    mpack_write_u16(writer, conf->neighbours.k);
    mpack_write_cstr(writer, ev_neighbours_keys[KEY_NEIGHBOURS_E]);
    mpack_write_float(writer, conf->neighbours.e);
    mpack_write_cstr(writer, ev_neighbours_keys[KEY_NEIGHBOURS_H]);
    mpack_write_float(writer, conf->neighbours.h);
    mpack_write_cstr(writer, ev_neighbours_keys[KEY_NEIGHBOURS_H_REL]);
    mpack_write_float(writer, conf->neighbours.h_rel);

    mpack_complete_map(writer);
}

static void config_ev_neighbours_parse(mpack_reader_t *reader, EventDetectionConfig *conf, bool *change)
{
    bool found[KEY_NEIGHBOURS_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, ev_neighbours_keys, found, KEY_NEIGHBOURS_COUNT)) {
            case KEY_NEIGHBOURS_K:
                conf_u16(reader, change, &conf->neighbours.k,
                         mpack_expect_u16_range(reader, 2, MAX_BUFFER_SAMPLES));
                break;
            case KEY_NEIGHBOURS_E:
                conf_float(reader, change, &conf->neighbours.e, mpack_expect_float(reader));
                break;
            case KEY_NEIGHBOURS_H:
                conf_float(reader, change, &conf->neighbours.h, mpack_expect_float(reader));
                break;
            case KEY_NEIGHBOURS_H_REL:
                conf_float(reader, change, &conf->neighbours.h_rel, mpack_expect_float(reader));
                break;
            default:
                mpack_discard(reader); break;
        }
    }
}

enum ev_crossing_names {KEY_CROSSING_K, KEY_CROSSING_SLOPE, KEY_CROSSING_COUNT};
static const char *ev_crossing_keys[KEY_CROSSING_COUNT] = {"k", "slope"};

static void config_ev_zero_crossing_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_crossing_keys[KEY_CROSSING_K]);
    mpack_write_u16(writer, conf->zero_crossing.k);
    mpack_write_cstr(writer, ev_crossing_keys[KEY_CROSSING_SLOPE]);
    mpack_write_float(writer, conf->zero_crossing.slope);

    mpack_complete_map(writer);
}

static void config_ev_zero_crossing_parse(mpack_reader_t *reader, EventDetectionConfig *conf, bool *change)
{
    bool found[KEY_CROSSING_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, ev_crossing_keys, found, KEY_CROSSING_COUNT)) {
            case KEY_CROSSING_K:
                conf_u16(reader, change, &conf->zero_crossing.k,
                         mpack_expect_u16_range(reader, 1, MAX_BUFFER_SAMPLES));
                break;
            case KEY_CROSSING_SLOPE:
                conf_float(reader, change, &conf->zero_crossing.slope, 
                           mpack_expect_float(reader));
                break;
            default:
                mpack_discard(reader); break;
        }
    }
}

enum ev_walker_names {
    KEY_WALKER_TOLERANCE, KEY_WALKER_HOLE, KEY_WALKER_PROMINENCE,
    KEY_WALKER_ISOLATION, KEY_WALKER_COUNT
};
static const char *ev_walker_keys[KEY_WALKER_COUNT] = {"t", "h", "p", "i"};

static void config_ev_hill_walker_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_walker_keys[KEY_WALKER_TOLERANCE]);
    mpack_write_float(writer, conf->hill_walker.tolerance);
    mpack_write_cstr(writer, ev_walker_keys[KEY_WALKER_HOLE]);
    mpack_write_i32(writer, conf->hill_walker.hole);
    mpack_write_cstr(writer, ev_walker_keys[KEY_WALKER_PROMINENCE]);
    mpack_write_float(writer, conf->hill_walker.prominence);
    mpack_write_cstr(writer, ev_walker_keys[KEY_WALKER_ISOLATION]);
    mpack_write_float(writer, conf->hill_walker.isolation);

    mpack_complete_map(writer);
}

static void config_ev_hill_walker_parse(mpack_reader_t *reader, EventDetectionConfig *conf, bool *change)
{
    bool found[KEY_WALKER_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, ev_walker_keys, found, KEY_WALKER_COUNT)) {
            case KEY_WALKER_TOLERANCE:
                conf_float(reader, change, &conf->hill_walker.tolerance,
                           mpack_expect_float(reader));
                break;
            case KEY_WALKER_HOLE:
                conf_int(reader, change, &conf->hill_walker.hole,
                         mpack_expect_i32(reader)); 
                break;
            case KEY_WALKER_PROMINENCE:
                conf_float(reader, change, &conf->hill_walker.prominence,
                           mpack_expect_float(reader));
                break;
            case KEY_WALKER_ISOLATION:
                conf_float(reader, change, &conf->hill_walker.isolation, 
                           mpack_expect_float(reader)); 
                break;
            default:
                mpack_discard(reader); break;
        }
    }
}

enum event_key_names {
    KEY_MIN_DURATION, KEY_T_PROXIMITY, KEY_STRATEGY,
    KEY_THRESHOLD, KEY_NEIGHBOURS, KEY_ZERO_CROSSING, KEY_HILL_WALKER,
    KEY_EVENT_COUNT
};
static const char *event_keys[KEY_EVENT_COUNT] = {
    "tmin", "tprox", "strategy", "threshold", "neighbours", "zero_crossing", "hill_walker"
};
static const char *strategies[] = {"threshold", "neighbours", "zero_crossing", "hill_walker"};

static void config_events_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    mpack_build_map(writer);

    mpack_write_cstr(writer, event_keys[KEY_MIN_DURATION]);
    mpack_write_u16(writer, conf->min_duration);
    mpack_write_cstr(writer, event_keys[KEY_T_PROXIMITY]);
    mpack_write_u16(writer, conf->time_proximity);
    mpack_write_cstr(writer, event_keys[KEY_STRATEGY]);
    mpack_write_cstr(writer, strategies[conf->strategy]);
    mpack_write_cstr(writer, event_keys[KEY_THRESHOLD]);
    config_ev_threshold_serialize(writer, conf);
    mpack_write_cstr(writer, event_keys[KEY_NEIGHBOURS]);
    config_ev_neighbours_serialize(writer, conf);
    mpack_write_cstr(writer, event_keys[KEY_ZERO_CROSSING]);
    config_ev_zero_crossing_serialize(writer, conf);
    mpack_write_cstr(writer, event_keys[KEY_HILL_WALKER]);
    config_ev_hill_walker_serialize(writer, conf);

    mpack_complete_map(writer);
}

static void config_events_parse(mpack_reader_t *reader, EventDetectionConfig *conf, bool *change)
{
    bool found[KEY_EVENT_COUNT] = {0};

    for (size_t i = mpack_expect_map_max(reader, MAX_MPACK_FIELDS_COUNT);
            i > 0 && mpack_reader_error(reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(reader, event_keys, found, KEY_EVENT_COUNT)) {
            case KEY_MIN_DURATION: 
                conf_u16(reader, change, &conf->min_duration, mpack_expect_u16(reader)); break;
            case KEY_T_PROXIMITY:
                conf_u16(reader, change, &conf->time_proximity, mpack_expect_u16(reader)); break;
            case KEY_STRATEGY:
                conf_int(reader, change, (int *)&conf->strategy, 
                         mpack_expect_enum(reader, strategies, STRATEGY_COUNT));
                break;
            case KEY_THRESHOLD: config_ev_threshold_parse(reader, conf, change); break;
            case KEY_NEIGHBOURS: config_ev_neighbours_parse(reader, conf, change); break;
            case KEY_ZERO_CROSSING: config_ev_zero_crossing_parse(reader, conf, change); break;
            case KEY_HILL_WALKER: config_ev_hill_walker_parse(reader, conf, change); break;
            default: mpack_discard(reader); break;
        }
    }
}


enum config_key_names {
    KEY_CONFIG_SENSOR, KEY_CONFIG_TSMOOTH, KEY_CONFIG_STATS,
    KEY_CONFIG_TRANSFORM, KEY_CONFIG_WELCH, KEY_CONFIG_FSMOOTH,
    KEY_CONFIG_PEAK, KEY_CONFIG_LOGGER, KEY_CONFIG_COUNT
};
static const char *config_keys[KEY_CONFIG_COUNT] = {
    "sensor", "tsmooth", "stats", "transform", "welch",
    "fsmooth", "peak", "logger"
};

size_t config_serialize(char *msg, size_t size, const Configuration *conf)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);
    mpack_build_map(&writer);

    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_SENSOR]);
    config_sensor_serialize(&writer, &conf->sensor);
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_TSMOOTH]);
    config_smooth_serialize(&writer, &conf->tsmooth);
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_STATS]);
    config_stats_serialize(&writer, &conf->stats);
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_TRANSFORM]);
    config_transform_serialize(&writer, &conf->transform);
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_FSMOOTH]);
    config_smooth_serialize(&writer, &conf->fsmooth);
    mpack_write_cstr(&writer ,config_keys[KEY_CONFIG_PEAK]);
    config_events_serialize(&writer, &conf->peak);
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_LOGGER]);
    config_logger_serialize(&writer, &conf->logger);

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}

bool config_parse(char *msg, int size, Configuration *conf, bool *error)
{
    bool found[KEY_CONFIG_COUNT] = {0};
    bool change = false;

    mpack_reader_t reader;
    mpack_reader_init_data(&reader, msg, size);

    for (size_t i = mpack_expect_map_max(&reader, KEY_CONFIG_COUNT);
            i > 0 && mpack_reader_error(&reader) == mpack_ok;
            i--) {
        switch (mpack_expect_key_cstr(&reader, config_keys, found, KEY_CONFIG_COUNT)) {
            case KEY_CONFIG_SENSOR: 
                config_sensor_parse(&reader, &conf->sensor, &change); break;
            case KEY_CONFIG_TSMOOTH:
                config_smooth_parse(&reader, &conf->tsmooth, &change);  break;
            case KEY_CONFIG_STATS:
                config_stats_parse(&reader, &conf->stats, &change);  break;
            case KEY_CONFIG_TRANSFORM:
                config_transform_parse(&reader, &conf->transform, &change);  break;
            case KEY_CONFIG_FSMOOTH:
                config_smooth_parse(&reader, &conf->fsmooth, &change);  break;
            case KEY_CONFIG_PEAK:
                config_events_parse(&reader, &conf->peak, &change);  break;
            case KEY_CONFIG_LOGGER:
                config_logger_parse(&reader, &conf->logger, &change);  break;
            default: 
                mpack_discard(&reader); break;
        }
    }

    *error = (mpack_reader_error(&reader) != mpack_ok);
    mpack_reader_destroy(&reader);
    return change;
}
