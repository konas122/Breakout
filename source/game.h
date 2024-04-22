#ifndef GAME_H
#define GAME_H

#include <tuple>
#include <vector>
#include <GLFW/glfw3.h>

#include "level.h"
#include "power_up.h"


enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

using Collision = std::tuple<bool, Direction, glm::vec2>;


class Game {
public:
    GameState               State;	

    bool                    Keys[1024];
    unsigned int            Width, Height;
    std::vector<GameLevel>  Levels;
    unsigned int            Level;
    std::vector<PowerUp>    PowerUps;

    Game(unsigned int width, unsigned int height);
    ~Game();

    void Init();

    // game loop
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void DoCollisions();

    // reset
    void ResetLevel();
    void ResetPlayer();

    // power up
    void SpawnPowerUps(GameObject &block);
    void UpdatePowerUps(GLfloat dt);
};

#endif
