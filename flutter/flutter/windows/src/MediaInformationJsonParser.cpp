#include "MediaInformationJsonParser.h"
//// OVERRIDING THE MACRO TO PREVENT APPLICATION TERMINATION
//#define RAPIDJSON_ASSERT(x)

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <memory>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static const char *MediaInformationJsonParserKeyStreams = "streams";
static const char *MediaInformationJsonParserKeyChapters = "chapters";

std::shared_ptr <ffmpeg_kit_flutter::MediaInformation>
ffmpeg_kit_flutter::MediaInformationJsonParser::from(const std::string &ffprobeJsonOutput) {
    try {
        return fromWithError(ffprobeJsonOutput);
    } catch (const std::exception &exception) {
        std::cout << "MediaInformation parsing failed: " << exception.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformation>
ffmpeg_kit_flutter::MediaInformationJsonParser::fromWithError(
        const std::string &ffprobeJsonOutput) {
    std::shared_ptr <rapidjson::Document> document = std::make_shared<rapidjson::Document>();

    document->Parse(ffprobeJsonOutput.c_str());

    if (document->HasParseError()) {
        throw std::runtime_error(GetParseError_En(document->GetParseError()));
    } else {
        std::shared_ptr < std::vector < std::shared_ptr <
        ffmpeg_kit_flutter::StreamInformation>>> streams = std::make_shared < std::vector <
                                                           std::shared_ptr <
                                                           ffmpeg_kit_flutter::StreamInformation>>>();
        std::shared_ptr < std::vector < std::shared_ptr < ffmpeg_kit_flutter::Chapter>>> chapters =
                                                                                                 std::make_shared <
                                                                                                 std::vector <
                                                                                                 std::shared_ptr <
                                                                                                 ffmpeg_kit_flutter::Chapter>>>();

        if (document->HasMember(MediaInformationJsonParserKeyStreams)) {
            rapidjson::Value &streamArray = (*document)[MediaInformationJsonParserKeyStreams];
            if (streamArray.IsArray()) {
                for (rapidjson::SizeType i = 0; i < streamArray.Size(); i++) {
                    auto streamValue = std::make_shared<rapidjson::Value>(
                            streamArray[i],
                            document->GetAllocator()
                    );
                    streams->push_back(
                            std::make_shared<ffmpeg_kit_flutter::StreamInformation>(streamValue)
                    );
                }
            }
        }

        if (document->HasMember(MediaInformationJsonParserKeyChapters)) {
            rapidjson::Value &chapterArray = (*document)[MediaInformationJsonParserKeyChapters];
            if (chapterArray.IsArray()) {
                for (rapidjson::SizeType i = 0; i < chapterArray.Size(); i++) {
                    auto chapterValue = std::make_shared<rapidjson::Value>(
                            chapterArray[i],
                            document->GetAllocator()
                    );
                    chapters->push_back(
                            std::make_shared<ffmpeg_kit_flutter::Chapter>(chapterValue)
                    );
                }
            }
        }

        return std::make_shared<ffmpeg_kit_flutter::MediaInformation>(
                std::static_pointer_cast<rapidjson::Value>(document), streams, chapters);
    }
}
