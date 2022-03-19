#include "pipeline.h"


size_t stats_serialize(char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);

    mpack_build_map(&writer);
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

size_t spectra_serialize(char *msg, size_t size, float *spectrum, size_t n, uint16_t fs)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);

    mpack_build_map(&writer);
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
    for (size_t i = 0; i < n; i++) {
        if (events[i].action == action) {
            mpack_build_map(writer);
            mpack_write_cstr(writer, "t");
            mpack_write_u32(writer, events[i].start);
            mpack_write_cstr(writer, "d");
            mpack_write_u32(writer, events[i].duration);
            mpack_write_cstr(writer, "f");
            mpack_write_float(writer, events[i].frequency);
            mpack_write_cstr(writer, "df");
            mpack_write_float(writer, events[i].tolerance);
            mpack_write_cstr(writer, "h");
            mpack_write_float(writer, events[i].amplitude);
            mpack_complete_map(writer);
        }
    }
    mpack_complete_array(writer);
}

void stream_serialize_init(mpack_writer_t *writer, char *msg, size_t n)
{
    mpack_writer_init(writer, msg, n);
    mpack_build_array(writer);
}

size_t stream_serialize_close(mpack_writer_t *writer)
{
    mpack_complete_array(writer);
    mpack_writer_destroy(writer);

    return mpack_writer_buffer_used(writer);
}

void stream_serialize(mpack_writer_t *writer, float x, float y, float z)
{
    mpack_write_float(writer, x);
    mpack_write_float(writer, y);
    mpack_write_float(writer, z);
}

size_t events_serialize(char *msg, size_t size, SpectrumEvent *events, size_t n)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);
    mpack_build_map(&writer);

    mpack_write_cstr(&writer, "start");
    events_serialize_by_type(SPECTRUM_EVENT_START, &writer, events, n);
    mpack_write_cstr(&writer, "finish");
    events_serialize_by_type(SPECTRUM_EVENT_FINISH, &writer, events, n);

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer);

    return mpack_writer_buffer_used(&writer);
}


/* ------------------ CONFIGURATION ----------------- */
static void config_sensor_serialize(mpack_writer_t *writer, const SamplingConfig *conf)
{
    static const char *sensor_range[] = {"2g", "4g", "8g", "16g"};
    enum sensor_key_names {KEY_FS, KEY_RANGE, KEY_N, KEY_OVERLAP, KEY_SENSOR_COUNT};
    static const char *sensor_keys[KEY_SENSOR_COUNT] = {"fs", "range", "n", "overlap"};

    mpack_build_map(writer);

    mpack_write_cstr(writer, sensor_keys[KEY_FS]);
    mpack_write_u16(writer, conf->frequency);
    mpack_write_cstr(writer, sensor_keys[KEY_RANGE]);
    mpack_write_cstr(writer, sensor_range[conf->range]);
    mpack_write_cstr(writer, sensor_keys[KEY_N]);
    mpack_write_u16(writer, conf->n);
    mpack_write_cstr(writer, sensor_keys[KEY_OVERLAP]);
    mpack_write_float(writer, conf->overlap);

    mpack_complete_map(writer);
}

static void config_smooth_serialize(mpack_writer_t *writer, const SmoothingConfig *conf)
{
    enum smooth_key_names {KEY_ENABLE, KEY_N, KEY_REPEAT, KEY_SMOOTH_COUNT};
    static const char *smooth_keys[KEY_SMOOTH_COUNT] = {"on", "n", "repeat"};
    
    mpack_build_map(writer);

    mpack_write_cstr(writer, smooth_keys[KEY_ENABLE]);
    mpack_write_bool(writer, conf->enable);
    mpack_write_cstr(writer, smooth_keys[KEY_N]);
    mpack_write_u16(writer, conf->n);
    mpack_write_cstr(writer, smooth_keys[KEY_REPEAT]);
    mpack_write_u8(writer, conf->repeat);

    mpack_complete_map(writer);
}

static void config_stats_serialize(mpack_writer_t *writer, const StatisticsConfig *conf)
{
    enum stats_key_names {
        KEY_MIN, KEY_MAX, KEY_RMS, KEY_MEAN, KEY_VARIANCE,
        KEY_STD, KEY_SKEWNESS, KEY_KURTOSIS, KEY_MEDIAN, KEY_MAD,
        KEY_STATS_COUNT
    };
    static const char *stats_keys[KEY_STATS_COUNT] = {
        "min", "max", "rms", "avg", "var", "std", "skew", "kurtosis", "med", "mad"
    };
    
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

static void config_transform_serialize(mpack_writer_t *writer, const FFTTransformConfig *conf)
{
    static const char *transform_window[] = {"boxcar", "bartlett", "hann", "hamming", "blackman"};
    static const char *transform_func[] = {"dft", "dct"};
    enum transform_key_names {KEY_WINDOW, KEY_FUNC, KEY_LOG, KEY_TRANSFORM_COUNT};
    static const char *transform_keys[KEY_TRANSFORM_COUNT] = {"w", "f", "log"};

    mpack_build_map(writer);

    mpack_write_cstr(writer, transform_keys[KEY_WINDOW]);
    mpack_write_cstr(writer, transform_window[conf->window]);
    mpack_write_cstr(writer, transform_keys[KEY_FUNC]);
    mpack_write_cstr(writer, transform_func[conf->func]);
    mpack_write_cstr(writer, transform_keys[KEY_LOG]);
    mpack_write_bool(writer, conf->log);

    mpack_complete_map(writer);
}

static void config_welch_serialize(mpack_writer_t *writer, const WelchAverageConfig *conf)
{
    enum welch_key_names {KEY_ENABLE, KEY_HISTORY, KEY_WELCH_COUNT};
    static const char *welch_keys[KEY_WELCH_COUNT] = {"on", "k"};

    mpack_build_map(writer);

    mpack_write_cstr(writer, welch_keys[KEY_ENABLE]);
    mpack_write_bool(writer, conf->enable);
    mpack_write_cstr(writer, welch_keys[KEY_HISTORY]);
    mpack_write_u8(writer, conf->history);

    mpack_complete_map(writer);
}

static void config_logger_serialize(mpack_writer_t *writer, const SaveFormatConfig *conf)
{
    enum logger_key_names {
        KEY_MQTT_LOG, KEY_LOCAL_LOG, KEY_LOG_SAMPLES, KEY_LOG_STATS,
        KEY_LOG_SPECTRA, KEY_LOG_EVENTS, KEY_SUBSAMPLING, KEY_LOG_COUNT
    };
    static const char *logger_keys[KEY_LOG_COUNT] = {
        "mqtt", "local", "samples", "stats", "spectra", "events", "subsamp"
    };

    mpack_build_map(writer);

    mpack_write_cstr(writer, logger_keys[KEY_MQTT_LOG]);
    mpack_write_bool(writer, conf->mqtt);
    mpack_write_cstr(writer, logger_keys[KEY_LOCAL_LOG]);
    mpack_write_bool(writer, conf->local);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_SAMPLES]);
    mpack_write_bool(writer, conf->samples);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_STATS]);
    mpack_write_bool(writer, conf->stats);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_SPECTRA]);
    mpack_write_bool(writer, conf->spectra);
    mpack_write_cstr(writer, logger_keys[KEY_LOG_EVENTS]);
    mpack_write_bool(writer, conf->events);
    mpack_write_cstr(writer, logger_keys[KEY_SUBSAMPLING]);
    mpack_write_u16(writer, conf->subsampling);

    mpack_complete_map(writer);
}

static void config_ev_threshold_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    enum ev_threshold_names {KEY_THRESHOLD_LEVEL, KEY_THRESHOLD_COUNT};
    static const char *ev_threshold_keys[KEY_THRESHOLD_COUNT] = {"t"};
    
    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_threshold_keys[KEY_THRESHOLD_LEVEL]);
    mpack_write_float(writer, conf->threshold.t);

    mpack_complete_map(writer);
}

static void config_ev_neighbours_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    enum ev_neighbours_names {
        KEY_NEIGHBOURS_K, KEY_NEIGHBOURS_E, KEY_NEIGHBOURS_H,
        KEY_NEIGHBOURS_H_REL, KEY_NEIGHBOURS_COUNT
    };
    static const char *ev_neighbours_keys[KEY_NEIGHBOURS_COUNT] = {"k", "e", "h", "h_rel"};
   
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

static void config_ev_zero_crossing_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    enum ev_crossing_names {KEY_CROSSING_K, KEY_CROSSING_SLOPE, KEY_CROSSING_COUNT};
    static const char *ev_crossing_keys[KEY_CROSSING_COUNT] = {"k", "slope"};

    mpack_build_map(writer);

    mpack_write_cstr(writer, ev_crossing_keys[KEY_CROSSING_K]);
    mpack_write_u16(writer, conf->zero_crossing.k);
    mpack_write_cstr(writer, ev_crossing_keys[KEY_CROSSING_SLOPE]);
    mpack_write_float(writer, conf->zero_crossing.slope);

    mpack_complete_map(writer);
}

static void config_ev_hill_walker_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    enum ev_walker_names {
        KEY_WALKER_TOLERANCE, KEY_WALKER_HOLE, KEY_WALKER_PROMINENCE, KEY_WALKER_ISOLATION,
        KEY_WALKER_COUNT
    };
    static const char *ev_walker_keys[KEY_WALKER_COUNT] = {"t", "h", "p", "i"};
    
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

static void config_events_serialize(mpack_writer_t *writer, const EventDetectionConfig *conf)
{
    enum event_key_names {
        KEY_MIN_DURATION, KEY_T_PROXIMITY, KEY_STRATEGY, 
        KEY_THRESHOLD, KEY_NEIGHBOURS, KEY_ZERO_CROSSING, KEY_HILL_WALKER,
        KEY_EVENT_COUNT
    };
    static const char *event_keys[KEY_EVENT_COUNT] = {
        "tmin", "tprox", "strategy", "threshold", "neighbours", "zero_crossing", "hill_walker"
    };
    static const char *strategies[] = {"threshold", "neighbours", "zero_crossing", "hill_walker"};

    mpack_build_map(writer);

    mpack_write_cstr(writer, event_keys[KEY_MIN_DURATION]);
    mpack_write_bool(writer, conf->min_duration);
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
    mpack_write_cstr(&writer, config_keys[KEY_CONFIG_WELCH]);
    config_welch_serialize(&writer, &conf->welch);
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

void config_parse(char *msg, int size, const Configuration *conf)
{
    bool found[KEY_CONFIG_COUNT] = {0};

    mpack_reader_t reader;
    mpack_reader_init_data(&reader, msg, size);
    int n;

    for (size_t i = mpack_expect_map_max(&reader, KEY_CONFIG_COUNT); 
            i > 0 && mpack_reader_error(&reader) == mpack_ok; 
            i--) {
        switch (mpack_expect_key_cstr(&reader, config_keys, found, KEY_CONFIG_COUNT)) {
            case KEY_CONFIG_SENSOR: n = mpack_expect_map_max(&reader, 10); break;  // parser call
            case KEY_CONFIG_TSMOOTH:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_STATS:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_TRANSFORM:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_WELCH:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_FSMOOTH:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_PEAK:  n = mpack_expect_map_max(&reader, 10);  break;
            case KEY_CONFIG_LOGGER:  n = mpack_expect_map_max(&reader, 10);  break;
            default: mpack_discard(&reader); break;
        }
    }

    mpack_reader_destroy(&reader);
    
    
    /* compact is not optional
    if (!found[KEY_COMPACT])
        mpack_reader_flag_error(&reader, mpack_error_data);*/
}