#include "QtSim.h"
#include <QtWidgets/QApplication>
/*
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}
#include <cstdlib>
#include <ctime>
#include <iostream>

void generate_random_color_frame(uint8_t* data, int width, int height, int linesize) {
    // Generate a random RGB color
    uint8_t r = std::rand() % 256;
    uint8_t g = std::rand() % 256;
    uint8_t b = std::rand() % 256;

    // Fill the frame with the random color
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            data[y * linesize + x * 3 + 0] = r; // Red
            data[y * linesize + x * 3 + 1] = g; // Green
            data[y * linesize + x * 3 + 2] = b; // Blue
        }
    }
}

int test()
{
    // Initialize random number generator
    std::srand(std::time(nullptr));

    // Video parameters
    const int width = 640;
    const int height = 480;
    const int fps = 30;
    const int total_frames = 100;

    // Output file
    const char* filename = "output1.mp4";

    // Allocate format context
    AVFormatContext* format_context = nullptr;
    if (avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename) < 0) {
        std::cerr << "Could not create output context" << std::endl;
        return -1;
    }

    // Find H.264 codec
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "H.264 codec not found" << std::endl;
        return -1;
    }

    // Create video stream
    AVStream* stream = avformat_new_stream(format_context, codec);
    if (!stream) {
        std::cerr << "Could not allocate stream" << std::endl;
        return -1;
    }

    // Allocate codec context
    AVCodecContext* codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return -1;
    }

    // Set codec parameters
    codec_context->bit_rate = 400000; // 400 kbps
    codec_context->width = width;
    codec_context->height = height;
    codec_context->time_base = { 1, fps };
    codec_context->framerate = { fps, 1 };
    codec_context->gop_size = 12; // Group of pictures
    codec_context->max_b_frames = 2;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Set preset for quality/speed tradeoff
    av_opt_set(codec_context->priv_data, "preset", "medium", 0);

    // Open codec
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return -1;
    }

    // Associate codec parameters with stream
    avcodec_parameters_from_context(stream->codecpar, codec_context);
    stream->time_base = codec_context->time_base;

    // Open output file
    if (!(format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&format_context->pb, filename, AVIO_FLAG_WRITE) < 0) {
            std::cerr << "Could not open output file" << std::endl;
            return -1;
        }
    }

    // Write the stream header
    if (avformat_write_header(format_context, nullptr) < 0) {
        std::cerr << "Error occurred when writing header to output file" << std::endl;
        return -1;
    }

    // Allocate frame for YUV420P
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Could not allocate frame" << std::endl;
        return -1;
    }
    frame->format = codec_context->pix_fmt;
    frame->width = codec_context->width;
    frame->height = codec_context->height;
    if (av_frame_get_buffer(frame, 32) < 0) {
        std::cerr << "Could not allocate frame data" << std::endl;
        return -1;
    }

    // Allocate frame for RGB
    AVFrame* rgb_frame = av_frame_alloc();
    if (!rgb_frame) {
        std::cerr << "Could not allocate RGB frame" << std::endl;
        return -1;
    }
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = width;
    rgb_frame->height = height;
    if (av_frame_get_buffer(rgb_frame, 32) < 0) {
        std::cerr << "Could not allocate RGB frame data" << std::endl;
        return -1;
    }

    // Initialize scaling context
    SwsContext* sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_RGB24,     // Source format
        width, height, AV_PIX_FMT_YUV420P,  // Destination format
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!sws_ctx) {
        std::cerr << "Could not initialize sws context" << std::endl;
        return -1;
    }

    // Allocate packet
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        std::cerr << "Could not allocate packet" << std::endl;
        return -1;
    }

    // Encode and write frames
    for (int i = 0; i < total_frames; ++i) {
        av_frame_make_writable(rgb_frame);
        generate_random_color_frame(rgb_frame->data[0], width, height, rgb_frame->linesize[0]);

        // Convert RGB frame to YUV420P
        sws_scale(
            sws_ctx,
            rgb_frame->data, rgb_frame->linesize,
            0, height,
            frame->data, frame->linesize);

        frame->pts = i;

        if (avcodec_send_frame(codec_context, frame) < 0) {
            std::cerr << "Error sending frame to encoder" << std::endl;
            break;
        }

        while (avcodec_receive_packet(codec_context, packet) == 0) {
            av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
            packet->stream_index = stream->index;

            if (av_interleaved_write_frame(format_context, packet) < 0) {
                std::cerr << "Error writing packet to file" << std::endl;
                break;
            }
            av_packet_unref(packet);
        }
    }

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

    return 0;
}
*/
int main(int argc, char *argv[])
{
    qDebug() << "woo;";
    // load res from libqnanopainter
    Q_INIT_RESOURCE(libqnanopainterdata);

#ifdef Q_OS_WIN
    // Select between OpenGL and OpenGL ES (Angle)
    //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    //test();

    QApplication a(argc, argv);
    //a.setStyleSheet("* { border: 1px solid red; }");

    QtSim w;
    w.setWindowTitle("QtSim - Developer: Will Hemsworth");
    w.resize(1024, 768);
    //w.show();
    w.showMaximized();
    return a.exec();
}
