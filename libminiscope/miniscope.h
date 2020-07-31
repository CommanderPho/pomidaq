/*
 * Copyright (C) 2019-2020 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 3
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the license, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MINISCOPE_H
#define MINISCOPE_H

#include <QMetaObject>
#include <QLoggingCategory>
#include <opencv2/core.hpp>

#include "mediatypes.h"

#ifdef Q_OS_WIN
#define MS_LIB_EXPORT __declspec(dllexport)
#else
#define MS_LIB_EXPORT __attribute__((visibility("default")))
#endif

namespace MScope
{
#ifndef Q_OS_WIN
#pragma GCC visibility push(default)
#endif
Q_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(logMScope)
#ifndef Q_OS_WIN
#pragma GCC visibility pop
#endif

using milliseconds_t = std::chrono::milliseconds;

using StatusMessageCallback = std::function<void(const QString&, void *)>;
using ControlChangeCallback = std::function<void(const QString&, double, double, void *)>;
using RawFrameCallback = std::function<void(const cv::Mat &, milliseconds_t &, const milliseconds_t &, const milliseconds_t &, void *)>;
using DisplayFrameCallback = std::function<void(const cv::Mat &, const milliseconds_t &, void *)>;

enum class DisplayMode {
    RawFrames,
    BackgroundDiff
};
Q_ENUM_NS(DisplayMode)

/**
 * @brief Set which type of control is needed
 */
enum class ControlKind {
    Unknown,  /// Unknown scope control
    Selector, /// switch between a set of predefined values
    Slider    /// slide between an min and a max value
};
Q_ENUM_NS(ControlKind)

class ControlDefinition
{
public:
    explicit ControlDefinition()
        : valueMin(-1),
          valueMax(-1)
    {}

    ControlKind kind;
    QString id;
    QString name;

    int valueMin;
    int valueMax;
    int stepSize;
    int startValue;

    QStringList labels;
    std::vector<double> values;
};

class MS_LIB_EXPORT Miniscope
{
public:
    explicit Miniscope();
    ~Miniscope();

    QStringList availableMiniscopeTypes() const;
    bool loadDeviceConfig(const QString &deviceType);
    QString deviceType() const;

    void setScopeCamId(int id);
    int scopeCamId() const;

    bool connect();
    void disconnect();
    bool isConnected() const;

    QList<ControlDefinition> controls() const;
    double controlValue(const QString &id);
    void setControlValue(const QString &id, double value);

    bool run();
    void stop();
    bool startRecording(const QString &fname = "");
    void stopRecording();

    bool running() const;
    bool recording() const;
    bool captureStartTimeInitialized() const;

    void setVisibleChannels(bool red, bool green, bool blue);
    bool showRedChannel() const;
    bool showGreenChannel() const;
    bool showBlueChannel() const;

    /**
     * @brief Called when a new status message is generated.
     *
     * Status messages reflect the general device state. They are not used for logging
     * or detailed event reporting.
     */
    void setOnStatusMessage(StatusMessageCallback callback, void *udata = nullptr);

    /**
     * @brief Called when a Miniscope setting is changes.
     */
    void setOnControlValueChange(ControlChangeCallback callback, void *udata = nullptr);

    /**
     * @brief Called *in the DAQ thread* when a frame was acquired.
     *
     * This callback is executed for each raw frame acquired from the Miniscope, and is equivalent
     * to what would be recorded to a video file.
     * The first timestamp is the timestamp of the frame in milliseconds. The callee may modify it,
     * to change the timestamp of a frame while it is being processed.
     *
     * The second timestamp parameter is the timestamp of the computer's clock when the frame
     * was - highly likely - acquired ("master timestamp").
     * The third timestamp parameter is the adjusted timestamp generated by the driver or device.
     * Both timestamps may already have been preprocessed a bit (adjusted for the selected start-time
     * and timestamp type), so you may not get completely "raw" timestamps.
     *
     * Please note that this function will also be called in case we are dropping frames. In this case, the
     * frame data matrix will be empty.
     */
    void setOnFrame(RawFrameCallback callback, void *udata = nullptr);

    /**
     * @brief Called *in the DAQ thread* when a frame was acquired on the edited frame.
     *
     * This callback is executed for each (possibly modified) "display frame" that an application like
     * PoMiDAQ would show to the user.
     */
    void setOnDisplayFrame(DisplayFrameCallback callback, void *udata = nullptr);

    cv::Mat currentDisplayFrame();
    uint currentFps() const;
    size_t droppedFramesCount() const;

    double fps() const;

    void setCaptureStartTime(const std::chrono::time_point<std::chrono::steady_clock> &startTime);
    bool useUnixTimestamps() const;
    void setUseUnixTimestamps(bool useUnixTime);
    milliseconds_t unixCaptureStartTime() const;

    bool externalRecordTrigger() const;
    void setExternalRecordTrigger(bool enabled);

    QString videoFilename() const;
    void setVideoFilename(const QString &fname);

    VideoCodec videoCodec() const;
    void setVideoCodec(VideoCodec codec);

    VideoContainer videoContainer() const;
    void setVideoContainer(VideoContainer container);

    bool recordLossless() const;
    void setRecordLossless(bool lossless);

    int minFluorDisplay() const;
    void setMinFluorDisplay(int value);

    int maxFluorDisplay() const;
    void setMaxFluorDisplay(int value);

    int minFluor() const;
    int maxFluor() const;

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);

    double bgAccumulateAlpha() const;
    void setBgAccumulateAlpha(double value);

    uint recordingSliceInterval() const;
    void setRecordingSliceInterval(uint minutes);

    QString lastError() const;

    milliseconds_t lastRecordedFrameTime() const;

private:
    class Private;
    Q_DISABLE_COPY(Miniscope)
    std::unique_ptr<Private> d;

    bool openCamera();
    void enqueueI2CCommand(long preambleKey, std::vector<quint8> packet);
    void sendCommandsToDevice();
    void addDisplayFrameToBuffer(const cv::Mat& frame, const milliseconds_t &timestamp);
    static void captureThread(void *msPtr);
    void startCaptureThread();
    void finishCaptureThread();
    void statusMessage(const QString &msg);
    void fail(const QString &msg);
};

} // end of MiniScope namespace

#endif // MINISCOPE_H
