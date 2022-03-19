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

void stream_serialize_init(mpack_writer_t *writer, char *msg, size_t n, size_t samples)
{
    mpack_writer_init(writer, msg, n);
    mpack_build_map(writer);
    mpack_write_cstr(writer, "s");
    mpack_start_array(writer, samples);
}

size_t stream_serialize_close(mpack_writer_t *writer)
{
    mpack_complete_map(writer);
    mpack_writer_destroy(writer);

    return mpack_writer_buffer_used(writer);
}

void stream_serialize(mpack_writer_t *writer, uint16_t x, uint16_t y, uint16_t z)
{
    mpack_write_float(writer, x);
    mpack_write_float(writer, y);
    mpack_write_float(writer, z);
}