#ifndef FFMPEG_KIT_MEDIA_INFORMATION_SESSION_COMPLETE_CALLBACK_H
#define FFMPEG_KIT_MEDIA_INFORMATION_SESSION_COMPLETE_CALLBACK_H

#include <iostream>
#include <memory>
#include <functional>

namespace ffmpeg_kit_flutter {

    class MediaInformationSession;

    /**
     * <p>Callback function that is invoked when an asynchronous <code>MediaInformation</code> session
     * has ended.
     * <p>Session has either SessionStateCompleted or SessionStateFailed state when
     * the callback is invoked.
     * <p>If it has SessionStateCompleted state, <code>ReturnCode</code> should be checked to
     * see the execution result.
     * <p>If <code>getState</code> returns SessionStateFailed then
     * <code>getFailStackTrace</code> should be used to get the failure reason.
     * <pre>
     *  switch (session->getState()) {
     *      case SessionStateCompleted:
     *          auto returnCode = session->getReturnCode();
     *          break;
     *      case SessionStateFailed:
     *          auto failStackTrace = session->getFailStackTrace();
     *          break;
     *  }
     * </pre>
     *
     * @param session session of the completed execution
     */
    typedef std::function<void(
            const std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> session)> MediaInformationSessionCompleteCallback;

#include "MediaInformationSession.h"

}

#endif // FFMPEG_KIT_MEDIA_INFORMATION_SESSION_COMPLETE_CALLBACK_H
