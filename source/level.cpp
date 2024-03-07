#include <fstream>
#include <sstream>
#include <glad/glad.h>

#include "level.h"


void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight) {
    this->Bricks.clear();
    unsigned int code;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;

    if (!fstream)
        return;
    
    while (std::getline(fstream, line)) {
        std::istringstream sstream(line);
        std::vector<unsigned int> row;
        while (sstream >> code)
            row.push_back(code);
        tileData.push_back(row);
    }
    if (tileData.size() > 0)
        this->init(tileData, levelWidth, levelHeight);
}


void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight) {
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    float unit_width = levelWidth / static_cast<float>(width), unit_height = levelHeight / height; 

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            // solid
            if (tileData[y][x] == 1) {
                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                GameObject obj(pos, size,
                               ResourceManager::GetTexture("block_solid"),
                               glm::vec3(0.8f, 0.8f, 0.7f)
                );
                obj.IsSolid = true;
                this->Bricks.push_back(obj);
            }
            // non-solid; now determine its color based on level data
            else if (tileData[y][x] > 1) {
                // original: white
                glm::vec3 color = glm::vec3(1.0f);

                if (tileData[y][x] == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (tileData[y][x] == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                else if (tileData[y][x] == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                else if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
            }
        }
    }
}


bool GameLevel::IsCompleted() {
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}


void GameLevel::Draw(SpriteRenderer &renderer) {
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);
}
