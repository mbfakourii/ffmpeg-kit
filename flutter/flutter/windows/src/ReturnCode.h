#ifndef FFMPEG_KIT_RETURN_CODE_H
#define FFMPEG_KIT_RETURN_CODE_H

#include <memory>
#include <iostream>

namespace ffmpeg_kit_flutter {

    class ReturnCode {
    public:
        static constexpr int Success = 0;
        static constexpr int Cancel = 255;

        static bool isSuccess(const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> value);

        static bool isCancel(const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> value);

        ReturnCode(const int value);

        int getValue() const;

        bool isValueSuccess() const;

        bool isValueError() const;

        bool isValueCancel() const;

        friend std::ostream &
        operator<<(std::ostream &out, const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> &o);

    private:
        int _value;
    };

}

#endif // FFMPEG_KIT_RETURN_CODE_H
