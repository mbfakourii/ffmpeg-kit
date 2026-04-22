#include "StreamInformation.h"

ffmpeg_kit_flutter::StreamInformation::StreamInformation(
        std::shared_ptr <rapidjson::Value> streamInformationValue) : _streamInformationValue{
        streamInformationValue} {
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::StreamInformation::getIndex() {
    return getNumberProperty(KeyIndex);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getType() {
    return getStringProperty(KeyType);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getCodec() {
    return getStringProperty(KeyCodec);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getCodecLong() {
    return getStringProperty(KeyCodecLong);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getFormat() {
    return getStringProperty(KeyFormat);
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::StreamInformation::getWidth() {
    return getNumberProperty(KeyWidth);
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::StreamInformation::getHeight() {
    return getNumberProperty(KeyHeight);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getBitrate() {
    return getStringProperty(KeyBitRate);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getSampleRate() {
    return getStringProperty(KeySampleRate);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getSampleFormat() {
    return getStringProperty(KeySampleFormat);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getChannelLayout() {
    return getStringProperty(KeyChannelLayout);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getSampleAspectRatio() {
    return getStringProperty(KeySampleAspectRatio);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getDisplayAspectRatio() {
    return getStringProperty(KeyDisplayAspectRatio);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getAverageFrameRate() {
    return getStringProperty(KeyAverageFrameRate);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getRealFrameRate() {
    return getStringProperty(KeyRealFrameRate);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getTimeBase() {
    return getStringProperty(KeyTimeBase);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::StreamInformation::getCodecTimeBase() {
    return getStringProperty(KeyCodecTimeBase);
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::StreamInformation::getTags() {
    return getProperty(KeyTags);
}

std::shared_ptr <std::string>
ffmpeg_kit_flutter::StreamInformation::getStringProperty(const char *key) {
    if (_streamInformationValue->HasMember(key)) {
        return std::make_shared<std::string>((*_streamInformationValue)[key].GetString());
    } else {
        return nullptr;
    }
}

std::shared_ptr <int64_t>
ffmpeg_kit_flutter::StreamInformation::getNumberProperty(const char *key) {
    if (_streamInformationValue->HasMember(key)) {
        return std::make_shared<int64_t>((*_streamInformationValue)[key].GetInt64());
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value>
ffmpeg_kit_flutter::StreamInformation::getProperty(const char *key) {
    if (_streamInformationValue->HasMember(key)) {
        auto value = std::make_shared<rapidjson::Value>();
        *value = (*_streamInformationValue)[key];
        return value;
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::StreamInformation::getAllProperties() {
    if (_streamInformationValue != nullptr) {
        auto all = std::make_shared<rapidjson::Value>();
        *all = (*_streamInformationValue);
        return all;
    } else {
        return nullptr;
    }
}
