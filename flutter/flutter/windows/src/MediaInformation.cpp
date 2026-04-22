#include "MediaInformation.h"

ffmpeg_kit_flutter::MediaInformation::MediaInformation(
        std::shared_ptr <rapidjson::Value> mediaInformationValue,
        std::shared_ptr <std::vector<std::shared_ptr < ffmpeg_kit_flutter::StreamInformation>>

> streams, std::shared_ptr <std::vector<std::shared_ptr < ffmpeg_kit_flutter::Chapter>>> chapters) :
_mediaInformationValue{
mediaInformationValue}, _streams{
streams}, _chapters{
chapters} {
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getFilename() {
    return getStringFormatProperty(KeyFilename);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getFormat() {
    return getStringFormatProperty(KeyFormat);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getLongFormat() {
    return getStringFormatProperty(KeyFormatLong);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getStartTime() {
    return getStringFormatProperty(KeyStartTime);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getDuration() {
    return getStringFormatProperty(KeyDuration);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getSize() {
    return getStringFormatProperty(KeySize);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::MediaInformation::getBitrate() {
    return getStringFormatProperty(KeyBitRate);
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::MediaInformation::getTags() {
    auto formatProperties = getFormatProperties();
    if (formatProperties->HasMember(KeyTags)) {
        auto tags = std::make_shared<rapidjson::Value>();
        *tags = (*formatProperties)[KeyTags];
        return tags;
    } else {
        return nullptr;
    }
}

std::shared_ptr <std::vector<std::shared_ptr < ffmpeg_kit_flutter::StreamInformation>>>

ffmpeg_kit_flutter::MediaInformation::getStreams() {
    return _streams;
}

std::shared_ptr <std::vector<std::shared_ptr < ffmpeg_kit_flutter::Chapter>>>

ffmpeg_kit_flutter::MediaInformation::getChapters() {
    return _chapters;
}

std::shared_ptr <std::string>
ffmpeg_kit_flutter::MediaInformation::getStringProperty(const char *key) {
    auto allProperties = getAllProperties();
    if (allProperties->HasMember(key)) {
        return std::make_shared<std::string>((*allProperties)[key].GetString());
    } else {
        return nullptr;
    }
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::MediaInformation::getNumberProperty(const char *key) {
    auto allProperties = getAllProperties();
    if (allProperties->HasMember(key)) {
        return std::make_shared<int64_t>((*allProperties)[key].GetInt64());
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value>
ffmpeg_kit_flutter::MediaInformation::getProperty(const char *key) {
    auto allProperties = getAllProperties();
    if (allProperties->HasMember(key)) {
        auto value = std::make_shared<rapidjson::Value>();
        *value = (*allProperties)[key];
        return value;
    } else {
        return nullptr;
    }
}

std::shared_ptr <std::string>
ffmpeg_kit_flutter::MediaInformation::getStringFormatProperty(const char *key) {
    auto formatProperties = getFormatProperties();
    if (formatProperties->HasMember(key)) {
        return std::make_shared<std::string>((*formatProperties)[key].GetString());
    } else {
        return nullptr;
    }
}

std::shared_ptr <int64_t>
ffmpeg_kit_flutter::MediaInformation::getNumberFormatProperty(const char *key) {
    auto formatProperties = getFormatProperties();
    if (formatProperties->HasMember(key)) {
        return std::make_shared<int64_t>((*formatProperties)[key].GetInt64());
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value>
ffmpeg_kit_flutter::MediaInformation::getFormatProperty(const char *key) {
    auto formatProperties = getFormatProperties();
    if (formatProperties->HasMember(key)) {
        auto value = std::make_shared<rapidjson::Value>();
        *value = (*formatProperties)[key];
        return value;
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::MediaInformation::getFormatProperties() {
    if (_mediaInformationValue->HasMember(KeyFormatProperties)) {
        auto mediaProperties = std::make_shared<rapidjson::Value>();
        *mediaProperties = (*_mediaInformationValue)[KeyFormatProperties];
        return mediaProperties;
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::MediaInformation::getAllProperties() {
    if (_mediaInformationValue != nullptr) {
        auto all = std::make_shared<rapidjson::Value>();
        *all = (*_mediaInformationValue);
        return all;
    } else {
        return nullptr;
    }
}
