/*
 * This file is part of QtSim
 *
 * Copyright (C) 2025 William Hemsworth
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "ffmpeg_worker.h"
#include <QDebug>

bool FFmpegWorker::startRecording()
{
    //if (!LoadFFmpegLibraries())
    //    return false;

    // Output file
    const char* filename = output_path.c_str();

    // Allocate format context
    format_context = nullptr;
    if (avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename) < 0)
        return false;

    // Find H.264 codec
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
        return false;

    // Create video stream
    stream = avformat_new_stream(format_context, codec);
    if (!stream)
        return false;

    // Allocate codec context
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context)
        return false;

    // Set codec parameters
    codec_context->bit_rate = 128000000;
    codec_context->width = trimmed_targ_w;
    codec_context->height = trimmed_targ_h;
    codec_context->time_base = { 1, fps };
    codec_context->framerate = { fps, 1 };
    codec_context->gop_size = 12; // Group of pictures
    codec_context->max_b_frames = 1;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Set preset for quality/speed tradeoff
    //av_opt_set(codec_context->priv_data, "crf", "5", 0);
    av_opt_set(codec_context->priv_data, "preset", "veryslow", 0);
    //av_opt_set(codec_context->priv_data, "tune", "animation", 0);
    av_opt_set(codec_context->priv_data, "profile", "high", 0);
    //av_opt_set(codec_context->priv_data, "preset", "slow", 0);

    // Open codec
    if (avcodec_open2(codec_context, codec, nullptr) < 0)
        return false;

    // Associate codec parameters with stream
    avcodec_parameters_from_context(stream->codecpar, codec_context);
    stream->time_base = codec_context->time_base;

    // Open output file
    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_context->pb, filename, AVIO_FLAG_WRITE) < 0)
            return false;
    }

    // Write the stream header
    if (avformat_write_header(format_context, nullptr) < 0)
        return false;

    // Allocate frame for YUV420P
    frame = av_frame_alloc();
    if (!frame)
        return false;

    frame->format = codec_context->pix_fmt;
    frame->width = trimmed_targ_w;
    frame->height = trimmed_targ_h;
    if (av_frame_get_buffer(frame, 32) < 0)
        return false;

    // Allocate frame for RGB
    rgb_frame = av_frame_alloc();
    if (!rgb_frame)
        return false;

    rgb_frame->format = AV_PIX_FMT_RGBA;
    rgb_frame->width = src_w;
    rgb_frame->height = src_h;
    if (av_frame_get_buffer(rgb_frame, 32) < 0)
        return false;

    // Initialize scaling context
    sws_ctx = sws_getContext(
        src_w, src_h, AV_PIX_FMT_RGBA,     // Source format
        codec_context->width, codec_context->height, AV_PIX_FMT_YUV420P,  // Destination format
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx)
        return false;

    // Allocate packet
    packet = av_packet_alloc();
    if (!packet)
        return false;

    frame_index = 0;
    initialized = true;
    qDebug() << "<<Recording>>";

    return true;
}

bool FFmpegWorker::encodeFrame(uint8_t* data)
{
    if (!initialized)
        return false;

    busy = true;

    av_frame_make_writable(rgb_frame);

    //int src_linesize = src_w * 4;
    //memcpy(rgb_frame->data[0], data, src_h * src_w * 4);

    int src_line_size = src_w * 4;
    int targ_line_size = rgb_frame->linesize[0];

    uint8_t* src_row;
    uint8_t* targ_row;


    if (flip)
    {
        int targ_y = 0;
        for (int src_y = trimmed_targ_h - 1; src_y >= 0; src_y--)
        {
            // Copy row
            src_row = data + (src_y * src_line_size);
            targ_row = rgb_frame->data[0] + (targ_y * targ_line_size);

            memcpy(targ_row, src_row, targ_line_size);

            targ_y++;
        }
    }
    else
    {
        for (int src_y = 0; src_y < trimmed_targ_h; src_y++)
        {
            // Copy row
            src_row = data + (src_y * src_line_size);
            targ_row = rgb_frame->data[0] + (src_y * targ_line_size);

            memcpy(targ_row, src_row, targ_line_size);
        }
    }

    // Convert RGB frame to YUV420P
    sws_scale(
        sws_ctx,
        rgb_frame->data, rgb_frame->linesize,  // Source frame data
        0, src_h, // Source frame height
        frame->data, frame->linesize); // Output frame data

    qDebug() << "<<Encoding Frame>> " << frame_index;
    frame->pts = frame_index++;

    if (avcodec_send_frame(codec_context, frame) < 0)
        return false;

    while (avcodec_receive_packet(codec_context, packet) == 0)
    {
        av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
        packet->stream_index = stream->index;

        if (av_interleaved_write_frame(format_context, packet) < 0)
            return false;

        av_packet_unref(packet);
    }

    busy = false;

    if (finalizing)
        doFinalize();

    emit frameFlushed();
    return true;
}

void FFmpegWorker::doFinalize()
{
    qDebug() << "<<Finalizing>>";

    // Flush the encoder
    avcodec_send_frame(codec_context, nullptr);
    while (avcodec_receive_packet(codec_context, packet) == 0) {
        av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
        packet->stream_index = stream->index;
        av_interleaved_write_frame(format_context, packet);
        av_packet_unref(packet);
    }

    // Write trailer
    av_write_trailer(format_context);

    // Free resources
    sws_freeContext(sws_ctx);
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    av_packet_free(&packet);
    avcodec_free_context(&codec_context);
    avio_close(format_context->pb);
    avformat_free_context(format_context);
}
