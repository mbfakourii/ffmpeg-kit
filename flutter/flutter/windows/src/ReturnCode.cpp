#include "ReturnCode.h"

bool ffmpeg_kit_flutter::ReturnCode::isSuccess(
        const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> value) {
    return (value != nullptr) && (value->getValue() == Success);
}

bool ffmpeg_kit_flutter::ReturnCode::isCancel(
        const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> value) {
    return (value != nullptr) && (value->getValue() == Cancel);
}

ffmpeg_kit_flutter::ReturnCode::ReturnCode(const int value) : _value{value} {
}

int ffmpeg_kit_flutter::ReturnCode::getValue() const {
    return _value;
}

bool ffmpeg_kit_flutter::ReturnCode::isValueSuccess() const {
    return (_value == Success);
}

bool ffmpeg_kit_flutter::ReturnCode::isValueError() const {
    return ((_value != Success) && (_value != Cancel));
}

bool ffmpeg_kit_flutter::ReturnCode::isValueCancel() const {
    return (_value == Cancel);
}

namespace ffmpeg_kit_flutter {

    std::ostream &
    operator<<(std::ostream &out, const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> &o) {
        if (o == nullptr) {
            return out;
        } else {
            return out << o->_value;
        }
    }

}
