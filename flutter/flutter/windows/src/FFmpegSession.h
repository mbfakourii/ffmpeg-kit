#ifndef FFMPEG_KIT_FFMPEG_SESSION_H
#define FFMPEG_KIT_FFMPEG_SESSION_H

#include "AbstractSession.h"
#include "StatisticsCallback.h"
#include "FFmpegSessionCompleteCallback.h"

namespace ffmpeg_kit_flutter {

    /**
     * <p>An FFmpeg session.
     */
    class FFmpegSession : public AbstractSession {
    public:

        /**
         * Builds a new FFmpeg session.
         *
         * @param arguments command arguments
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
        create(const std::list <std::string> &arguments);

        /**
         * Builds a new FFmpeg session.
         *
         * @param arguments         command arguments
         * @param completeCallback  session specific complete callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
        create(const std::list <std::string> &arguments,
               ffmpeg_kit_flutter::FFmpegSessionCompleteCallback completeCallback);

        /**
         * Builds a new FFmpeg session.
         *
         * @param arguments             command arguments
         * @param completeCallback      session specific complete callback
         * @param logCallback           session specific log callback
         * @param statisticsCallback    session specific statistics callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
        create(const std::list <std::string> &arguments,
               ffmpeg_kit_flutter::FFmpegSessionCompleteCallback completeCallback,
               ffmpeg_kit_flutter::LogCallback logCallback,
               ffmpeg_kit_flutter::StatisticsCallback statisticsCallback);

        /**
         * Builds a new FFmpeg session.
         *
         * @param arguments               command arguments
         * @param completeCallback        session specific complete callback
         * @param logCallback             session specific log callback
         * @param statisticsCallback      session specific statistics callback
         * @param logRedirectionStrategy  session specific log redirection strategy
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
        create(const std::list <std::string> &arguments,
               ffmpeg_kit_flutter::FFmpegSessionCompleteCallback completeCallback,
               ffmpeg_kit_flutter::LogCallback logCallback,
               ffmpeg_kit_flutter::StatisticsCallback statisticsCallback,
               ffmpeg_kit_flutter::LogRedirectionStrategy logRedirectionStrategy);

        /**
         * Returns the session specific statistics callback.
         *
         * @return session specific statistics callback
         */
        ffmpeg_kit_flutter::StatisticsCallback getStatisticsCallback();

        /**
         * Returns the session specific complete callback.
         *
         * @return session specific complete callback
         */
        ffmpeg_kit_flutter::FFmpegSessionCompleteCallback getCompleteCallback();

        /**
         * Returns all statistics entries generated for this session. If there are asynchronous
         * messages that are not delivered yet, this method waits for them until the given timeout.
         *
         * @param waitTimeout wait timeout for asynchronous messages in milliseconds
         * @return list of statistics entries generated for this session
         */
        std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

        getAllStatisticsWithTimeout(const int waitTimeout);

        /**
         * Returns all statistics entries generated for this session. If there are asynchronous
         * messages that are not delivered yet, this method waits for them until
         * AbstractSessionDefaultTimeoutForAsynchronousMessagesInTransmit expires.
         *
         * @return list of statistics entries generated for this session
         */
        std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

        getAllStatistics();

        /**
         * Returns all statistics entries delivered for this session. Note that if there are
         * asynchronous messages that are not delivered yet, this method will not wait for
         * them and will return immediately.
         *
         * @return list of statistics entries received for this session
         */
        std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

        getStatistics();

        /**
         * Returns the last received statistics entry.
         *
         * @return the last received statistics entry or nullptr if there are not any statistics entries
         * received
         */
        std::shared_ptr <ffmpeg_kit_flutter::Statistics> getLastReceivedStatistics();

        /**
         * Adds a new statistics entry for this session. It is invoked internally by <code>FFmpegKit</code> library methods.
         * Must not be used by user applications.
         *
         * @param statistics statistics entry
         */
        void addStatistics(const std::shared_ptr <ffmpeg_kit_flutter::Statistics> statistics);

        /**
         * Returns whether it is an <code>FFmpeg</code> session or not.
         *
         * @return true if it is an <code>FFmpeg</code> session, false otherwise
         */
        bool isFFmpeg() const override;

        /**
         * Returns whether it is an <code>FFprobe</code> session or not.
         *
         * @return true if it is an <code>FFprobe</code> session, false otherwise
         */
        bool isFFprobe() const override;

        /**
         * Returns whether it is a <code>MediaInformation</code> session or not.
         *
         * @return true if it is a <code>MediaInformation</code> session, false otherwise
         */
        bool isMediaInformation() const override;

    private:

        struct PublicFFmpegSession;

        /**
         * Builds a new FFmpeg session.
         *
         * @param arguments               command arguments
         * @param completeCallback        session specific complete callback
         * @param logCallback             session specific log callback
         * @param statisticsCallback      session specific statistics callback
         * @param logRedirectionStrategy  session specific log redirection strategy
         */
        FFmpegSession(const std::list <std::string> &arguments,
                      ffmpeg_kit_flutter::FFmpegSessionCompleteCallback completeCallback,
                      ffmpeg_kit_flutter::LogCallback logCallback,
                      ffmpeg_kit_flutter::StatisticsCallback statisticsCallback,
                      ffmpeg_kit_flutter::LogRedirectionStrategy logRedirectionStrategy);

        ffmpeg_kit_flutter::StatisticsCallback _statisticsCallback;
        FFmpegSessionCompleteCallback _completeCallback;
        std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>
        _statistics;
    };

}

#endif // FFMPEG_KIT_FFMPEG_SESSION_H
