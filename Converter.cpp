#include "Converter.h"
#include <exception>

using json = nlohmann::json;

// TODO: Add proper support for actual offsets

// TODO: Add ANIM support

const std::unordered_set<std::string> LCE_BONE_NAMES = {
    "HEAD", "BODY", "ARM0", "ARM1", "LEG0", "LEG1",
    "HEADWEAR", "JACKET", "SLEEVE0", "SLEEVE1", "PANTS0", "PANTS1", "SOCK0", "SOCK1", 
    "BODYARMOR", "WAIST", "BELT", "ARMARMOR0", "ARMARMOR1", "LEGGING0", "LEGGING1", "BOOT0", "BOOT1"
};

bool isHelmet{ false };
float posXOffset{};
float posYOffset{};
float posZOffset{};

void GetLCEBoneOffsets(const std::string& BONE_NAME)
{
    posXOffset = 0;
    posYOffset = 0;
    posZOffset = 0;

    if (BONE_NAME == "ARM0" || BONE_NAME == "ARMARMOR0" || BONE_NAME == "SLEEVE0") {
        posXOffset = 5;
        posYOffset = 22;
    }
    else if (BONE_NAME == "ARM1" || BONE_NAME == "ARMARMOR1" || BONE_NAME == "SLEEVE1") {
        posXOffset = -5;
        posYOffset = 22;
    }
    else if (BONE_NAME == "LEG0" || BONE_NAME == "SOCK0" || BONE_NAME == "BOOT0" || BONE_NAME == "PANTS0") {
        posXOffset = 2.0f;
        posYOffset = 12;
    }
    else if (BONE_NAME == "LEG1" || BONE_NAME == "SOCK1" || BONE_NAME == "BOOT1" || BONE_NAME == "PANTS1") {
        posXOffset = -2.0f;
        posYOffset = 12;
    }
    else if (BONE_NAME == "HEADARMOR" || BONE_NAME == "HEAD" || BONE_NAME == "HEADWEAR" || BONE_NAME == "BODY" ||
        BONE_NAME == "BODYARMOR" || BONE_NAME == "WAIST" || BONE_NAME == "BELT" || BONE_NAME == "JACKET") {
        posYOffset = 24;
    }
}

std::string GetLCEBoneName(const std::string& boneName)
{
    isHelmet = false;
    std::string BONE_NAME;

    for (char c : boneName)
    {
        // lazy upper for loop. C++ seriously doesn't have a built in function for this yet??
        BONE_NAME += std::toupper(c);
    }

    // if Bone name is already valid, then return
    if (LCE_BONE_NAMES.contains(BONE_NAME))
        return BONE_NAME;

    // Call me Toby Fox or else if
    else if (BONE_NAME == "HEADARMOR" || BONE_NAME == "HELMET")
    {
        isHelmet = true;
        return "HEADWEAR";
    }
    else if (BONE_NAME == "CHEST")
    {
        return "BODYARMOR";
    }
    else if (BONE_NAME == "LEFTARMARMOR")
    {
        return "ARMARMOR0";
    }
    else if (BONE_NAME == "LEFTLEGGING")
    {
        return "LEGGING0";
    }
    else if (BONE_NAME == "LEFTBOOT")
    {
        return "BOOT0";
    }
    else if (BONE_NAME == "RIGHTARMARMOR")
    {
        return "ARMARMOR1";
    }
    else if (BONE_NAME == "RIGHTLEGGING")
    {
        return "LEGGING1";
    }
    else if (BONE_NAME == "RIGHTBOOT")
    {
        return "BOOT1";
    }
    else if (BONE_NAME == "LEFT_ARM" || BONE_NAME == "LEFTARM")
    {
        return "ARM0";
    }
    else if (BONE_NAME == "LEFT_SLEEVE" || BONE_NAME == "LEFTSLEEVE")
    {
        return "SLEEVE0";
    }
    else if (BONE_NAME == "RIGHT_ARM" || BONE_NAME == "RIGHTARM")
    {
        return "ARM1";
    }
    else if (BONE_NAME == "RIGHT_SLEEVE" || BONE_NAME == "RIGHTSLEEVE")
    {
        return "SLEEVE1";
    }
    else if (BONE_NAME == "LEFT_LEG" || BONE_NAME == "LEFTLEG")
    {
        return "LEG0";
    }
    else if (BONE_NAME == "LEFT_PANTS" || BONE_NAME == "LEFTPANTS")
    {
        return "PANTS0";
    }
    else if (BONE_NAME == "LEFT_SOCK" || BONE_NAME == "LEFTSOCK")
    {
        return "SOCK0";
    }
    else if (BONE_NAME == "RIGHT_LEG" || BONE_NAME == "RIGHTLEG")
    {
        return "LEG1";
    }
    else if (BONE_NAME == "RIGHT_PANTS" || BONE_NAME == "RIGHTPANTS")
    {
        return "PANTS1";
    }
    else if (BONE_NAME == "RIGHT_SOCK" || BONE_NAME == "RIGHTSOCK")
    {
        return "SOCK1";
    }

    return "NULL";
}

bool JSON2CSM(SDL_Window& window, const std::string& inpath)
{
    std::ifstream file(inpath);
    if (!file) {
        std::cerr << "Failed to open file: " << inpath << std::endl;
        return false;
    }

    json root;
    try {
        file >> root;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return false;
    }

    file.close();

    bool legacyFormat{ false };

    // Access geometry array
    auto& rootArray = root["minecraft:geometry"];
    std::string rootName{""};
    if (!rootArray.is_array())
    {
        // check if it's the legacy bedrock format
        for (auto& el : root.items()) {
            if (el.key().starts_with("geometry.")) {
                rootArray = el.value();
                rootName = el.key();
                legacyFormat = true;
                break;
            }
        }
    }

    std::stringstream* fileData = new std::stringstream();

    std::cout << rootName << std::endl;

    try {
        if (legacyFormat)
        {
            std::string identifier = rootName;
            const auto& model = rootArray;

            int texWidth = model["texturewidth"].get<int>();
            int texHeight = model["textureheight"].get<int>();

            if (model.contains("bones"))
            {
                for (const auto& bone : model["bones"])
                {
                    std::string boneName = bone["name"].get<std::string>();

                    if (bone.contains("cubes")) {
                        for (const auto& cube : bone["cubes"]) {
                            std::string finalBoneName = GetLCEBoneName(boneName);

                            GetLCEBoneOffsets(finalBoneName);

                            if (finalBoneName == "NULL")
                            {
                                std::cerr << "INVALID BONE, SKIPPED\n";
                                continue;
                            }

                            auto origin = cube["origin"];
                            auto size = cube["size"];
                            auto uv = cube["uv"];
                            bool isMirrored = cube.contains("mirror") && cube["mirror"].get<bool>();
                            float scale = cube.contains("inflate") ? cube["inflate"].get<float>() : 0.0f;

                            float PosX = origin[0] + posXOffset;
                            float PosY = -1 * (origin[1] - posYOffset + size[1]);
                            float PosZ = origin[2] + posZOffset;

                            *fileData << "BOX " << finalBoneName << " " << PosX << " " << PosY << " " << PosZ << " " << size[0] << " " << size[1] << " " << size[2] << " " << uv[0] << " " << uv[1] << " " << isHelmet << " " << isMirrored << " " << scale << "\n";

                            std::cout << "  Cube Origin: [" << PosX << ", " << PosY << ", " << PosZ << "]\n";
                            std::cout << "  Cube Size: [" << size[0] << ", " << size[1] << ", " << size[2] << "]\n";
                            std::cout << "  UV: [" << uv[0] << ", " << uv[1] << "]\n";

                            if (cube.contains("mirror"))
                                std::cout << "  Mirrored: " << isMirrored << "\n";
                            if (cube.contains("inflate"))
                                std::cout << "  Inflate: " << scale << "\n";
                        }
                    }
                }
            }
        }
        else
        {
            for (const auto& geo : rootArray)
            {
                auto identifier = geo["description"]["identifier"].get<std::string>();
                int texWidth = geo["description"]["texture_width"].get<int>();
                int texHeight = geo["description"]["texture_height"].get<int>();

                std::cout << "Model: " << identifier << " (" << texWidth << "x" << texHeight << ")\n";

                if (geo.contains("bones"))
                {
                    for (const auto& bone : geo["bones"])
                    {
                        std::string boneName = bone["name"].get<std::string>();
                        std::cout << " Bone: " << boneName << std::endl;

                        if (bone.contains("cubes")) {
                            for (const auto& cube : bone["cubes"]) {
                                std::string finalBoneName = GetLCEBoneName(boneName);

                                GetLCEBoneOffsets(finalBoneName);

                                if (finalBoneName == "NULL")
                                {
                                    std::cerr << "INVALID BONE, SKIPPED\n";
                                    continue;
                                }

                                auto origin = cube["origin"];
                                auto size = cube["size"];
                                auto uv = cube["uv"];
                                bool isMirrored = cube.contains("mirror") && cube["mirror"].get<bool>();
                                float scale = cube.contains("inflate") ? cube["inflate"].get<float>() : 0.0f;

                                float PosX = origin[0] + posXOffset;
                                float PosY = -1 * (origin[1] - posYOffset + size[1]);
                                float PosZ = origin[2] + posZOffset;

                                *fileData << "BOX " << finalBoneName << " " << PosX << " " << PosY << " " << PosZ << " " << size[0] << " " << size[1] << " " << size[2] << " " << uv[0] << " " << uv[1] << " " << isHelmet << " " << isMirrored << " " << scale << "\n";

                                std::cout << "  Cube Origin: [" << PosX << ", " << PosY << ", " << PosZ << "]\n";
                                std::cout << "  Cube Size: [" << size[0] << ", " << size[1] << ", " << size[2] << "]\n";
                                std::cout << "  UV: [" << uv[0] << ", " << uv[1] << "]\n";

                                if (cube.contains("mirror"))
                                    std::cout << "  Mirrored: " << isMirrored << "\n";
                                if (cube.contains("inflate"))
                                    std::cout << "  Inflate: " << scale << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        // handle standard exceptions
        std::cerr << "Exception: " << e.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error. Operation Aborted.", "Something went wrong. Please ensure that the model is a valid Bedrock Entity model JSON.", &window);
        return false;
    }

    csmSaveFile(&window, fileData);

    return true;
}