#ifndef FFMPEG_KIT_MEDIA_INFORMATION_SESSION_H
#define FFMPEG_KIT_MEDIA_INFORMATION_SESSION_H

#include "AbstractSession.h"
#include "MediaInformation.h"
#include "MediaInformationSessionCompleteCallback.h"

namespace ffmpeg_kit_flutter {

    /**
     * <p>A custom FFprobe session, which produces a <code>MediaInformation</code> object using the
     * FFprobe output.
     */
    class MediaInformationSession : public AbstractSession {
    public:

        /**
         * Creates a new media information session.
         *
         * @param arguments command arguments
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
        create(const std::list <std::string> &arguments);

        /**
         * Creates a new media information session.
         *
         * @param arguments        command arguments
         * @param completeCallback session specific complete callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
        create(const std::list <std::string> &arguments,
               ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback);

        /**
         * Creates a new media information session.
         *
         * @param arguments        command arguments
         * @param completeCallback session specific complete callback
         * @param logCallback      session specific log callback
         * @return created session
         */
        static std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
        create(const std::list <std::string> &arguments,
               ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback,
               ffmpeg_kit_flutter::LogCallback logCallback);

        /**
         * Returns the media information extracted in this session.
         *
         * @return media information extracted or nullptr if the command failed or the output can not be
         * parsed
         */
        std::shared_ptr <ffmpeg_kit_flutter::MediaInformation> getMediaInformation();

        /**
         * Sets the media information extracted in this session.
         *
         * @param mediaInformation media information extracted
         */
        void setMediaInformation(
                const std::shared_ptr <ffmpeg_kit_flutter::MediaInformation> mediaInformation);

        /**
         * Returns the session specific complete callback.
         *
         * @return session specific complete callback
         */
        ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback getCompleteCallback();

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

        struct PublicMediaInformationSession;

        /**
         * Creates a new media information session.
         *
         * @param arguments        command arguments
         * @param completeCallback session specific complete callback
         * @param logCallback      session specific log callback
         */
        MediaInformationSession(const std::list <std::string> &arguments,
                                ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback,
                                ffmpeg_kit_flutter::LogCallback logCallback);

        ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback _completeCallback;
        std::shared_ptr <ffmpeg_kit_flutter::MediaInformation> _mediaInformation;
    };

}

#endif // FFMPEG_KIT_MEDIA_INFORMATION_SESSION_H
