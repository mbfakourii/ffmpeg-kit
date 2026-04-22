#ifndef FFMPEG_KIT_CHAPTER_H
#define FFMPEG_KIT_CHAPTER_H

//// OVERRIDING THE MACRO TO PREVENT APPLICATION TERMINATION
//#define RAPIDJSON_ASSERT(x)

#include "rapidjson/document.h"
#include <string>
#include <iostream>
#include <memory>

namespace ffmpeg_kit_flutter {

    /**
     * Chapter class.
     */
    class Chapter {
    public:
        static constexpr const char *KeyId = "id";
        static constexpr const char *KeyTimeBase = "time_base";
        static constexpr const char *KeyStart = "start";
        static constexpr const char *KeyStartTime = "start_time";
        static constexpr const char *KeyEnd = "end";
        static constexpr const char *KeyEndTime = "end_time";
        static constexpr const char *KeyTags = "tags";

        Chapter(std::shared_ptr <rapidjson::Value> chapterValue);

        std::shared_ptr <int64_t> getId();

        std::shared_ptr <std::string> getTimeBase();

        std::shared_ptr <int64_t> getStart();

        std::shared_ptr <std::string> getStartTime();

        std::shared_ptr <int64_t> getEnd();

        std::shared_ptr <std::string> getEndTime();

        std::shared_ptr <rapidjson::Value> getTags();

        /**
         * Returns the chapter property associated with the key.
         *
         * @return chapter property as string or nullptr if the key is not found
         */
        std::shared_ptr <std::string> getStringProperty(const char *key);

        /**
         * Returns the chapter property associated with the key.
         *
         * @return chapter property as number or nullptr if the key is not found
         */
        std::shared_ptr <int64_t> getNumberProperty(const char *key);

        /**
         * Returns the chapter property associated with the key.
         *
         * @return chapter property in a Value or nullptr if the key is not found
         */
        std::shared_ptr <rapidjson::Value> getProperty(const char *key);

        /**
         * Returns all chapter properties defined.
         *
         * @return all chapter properties in a Value or nullptr if no properties are defined
         */
        std::shared_ptr <rapidjson::Value> getAllProperties();

    private:
        std::shared_ptr <rapidjson::Value> _chapterValue;
    };

}

#endif // FFMPEG_KIT_CHAPTER_H
