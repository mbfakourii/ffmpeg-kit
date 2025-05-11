#include "Chapter.h"

ffmpeg_kit_flutter::Chapter::Chapter(std::shared_ptr <rapidjson::Value> chapterValue)
        : _chapterValue{chapterValue} {
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::Chapter::getId() {
    return getNumberProperty(KeyId);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::Chapter::getTimeBase() {
    return getStringProperty(KeyTimeBase);
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::Chapter::getStart() {
    return getNumberProperty(KeyStart);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::Chapter::getStartTime() {
    return getStringProperty(KeyStartTime);
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::Chapter::getEnd() {
    return getNumberProperty(KeyEnd);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::Chapter::getEndTime() {
    return getStringProperty(KeyEndTime);
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::Chapter::getTags() {
    return getProperty(KeyTags);
}

std::shared_ptr <std::string> ffmpeg_kit_flutter::Chapter::getStringProperty(const char *key) {
    if (_chapterValue->HasMember(key)) {
        return std::make_shared<std::string>((*_chapterValue)[key].GetString());
    } else {
        return nullptr;
    }
}

std::shared_ptr <int64_t> ffmpeg_kit_flutter::Chapter::getNumberProperty(const char *key) {
    if (_chapterValue->HasMember(key)) {
        return std::make_shared<int64_t>((*_chapterValue)[key].GetInt64());
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::Chapter::getProperty(const char *key) {
    if (_chapterValue->HasMember(key)) {
        auto value = std::make_shared<rapidjson::Value>();
        *value = (*_chapterValue)[key];
        return value;
    } else {
        return nullptr;
    }
}

std::shared_ptr <rapidjson::Value> ffmpeg_kit_flutter::Chapter::getAllProperties() {
    if (_chapterValue != nullptr) {
        auto all = std::make_shared<rapidjson::Value>();
        *all = (*_chapterValue);
        return all;
    } else {
        return nullptr;
    }
}
