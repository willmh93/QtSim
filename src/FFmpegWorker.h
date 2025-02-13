#pragma once
#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class FFmpegWorker : public QObject
{
    Q_OBJECT

    int src_w;
    int src_h;
    int targ_w;
    int targ_h;
    int fps;
    int frame_index;

    bool initialized = false;
    bool finalizing = false;
    bool busy = false;

    std::string output_path;
    AVFormatContext* format_context;
    AVStream* stream;
    AVCodecContext* codec_context;
    AVFrame* frame;
    AVFrame* rgb_frame;
    SwsContext* sws_ctx;
    AVPacket* packet;

    void doFinalize();

public:

    void setOutputInfo(
        std::string save_path,
        int src_width,
        int src_height,
        int targ_width = 640,
        int targ_height = 480,
        int record_fps = 60)
    {
        output_path = save_path;
        src_w = src_width;
        src_h = src_height;
        targ_w = src_width;
        targ_h = src_height;
        fps = record_fps;
    }

    bool isInitialized()
    {
        return initialized;
    }

signals:

    void frameFlushed();
    void onFinalized();

public slots:

    bool startRecording();
    bool encodeFrame(uint8_t* data);
    void finalizeRecording()
    {
        if (busy)
            finalizing = true;
        else
            doFinalize();

        emit onFinalized();
    }

};
