#ifndef FFMPEG_KIT_FFPROBE_SESSION_H
#define FFMPEG_KIT_FFPROBE_SESSION_H

#include "AbstractSession.h"
#include "FFprobeSessionCompleteCallback.h"

namespace ffmpeg_kit_flutter {

    /**
     * <p>An FFprobe session.
     */
    class FFprobeSession : public AbstractSession {
    public:

        /**
         * Builds a new FFprobe session.
         *
         * @param arguments command arguments
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
        create(const std::list <std::string> &arguments);

        /**
         * Builds a new FFprobe session.
         *
         * @param arguments        command arguments
         * @param completeCallback session specific complete callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
        create(const std::list <std::string> &arguments,
               const FFprobeSessionCompleteCallback completeCallback);

        /**
         * Builds a new FFprobe session.
         *
         * @param arguments        command arguments
         * @param completeCallback session specific complete callback
         * @param logCallback      session specific log callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
        create(const std::list <std::string> &arguments,
               const FFprobeSessionCompleteCallback completeCallback,
               const ffmpeg_kit_flutter::LogCallback logCallback);

        /**
         * Builds a new FFprobe session.
         *
         * @param arguments               command arguments
         * @param completeCallback        session specific complete callback
         * @param logCallback             session specific log callback
         * @param logRedirectionStrategy  session specific log redirection strategy
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
        create(const std::list <std::string> &arguments,
               const FFprobeSessionCompleteCallback completeCallback,
               const ffmpeg_kit_flutter::LogCallback logCallback,
               const LogRedirectionStrategy logRedirectionStrategy);

        /**
         * Returns the session specific complete callback.
         *
         * @return session specific complete callback
         */
        ffmpeg_kit_flutter::FFprobeSessionCompleteCallback getCompleteCallback();

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

        struct PublicFFprobeSession;

        /**
         * Builds a new FFprobe session.
         *
         * @param arguments               command arguments
         * @param completeCallback        session specific complete callback
         * @param logCallback             session specific log callback
         * @param logRedirectionStrategy  session specific log redirection strategy
         */
        FFprobeSession(const std::list <std::string> &arguments,
                       const FFprobeSessionCompleteCallback completeCallback,
                       const ffmpeg_kit_flutter::LogCallback logCallback,
                       const LogRedirectionStrategy logRedirectionStrategy);

        FFprobeSessionCompleteCallback _completeCallback;
    };

}

#endif // FFMPEG_KIT_FFPROBE_SESSION_H
