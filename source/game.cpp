#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"


GameObject      *Player;
SpriteRenderer  *Renderer;

const float PLAYER_VELOCITY(500.0f);
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}


Game::~Game() {
    delete Renderer;
    delete Player;
}


void Game::Init() {
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");

    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // load textures
    ResourceManager::LoadTexture("awesomeface.png", true, "face");
    ResourceManager::LoadTexture("background.jpg", false, "background");
    ResourceManager::LoadTexture("block.png", false, "block");
    ResourceManager::LoadTexture("block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("paddle.png", true, "paddle");

    // load levels
    GameLevel one;      one.Load("levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two;      two.Load("levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three;    three.Load("levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four;     four.Load("levels/four.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;

    // configure game objects
    glm::vec2 playerPos = glm::vec2(
        this->Width / 2 - PLAYER_SIZE.x / 2,
        this->Height - PLAYER_SIZE.y
    );
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
}


void Game::Update(float dt)
{
    
}


void Game::ProcessInput(float dt) {
   if (this->State == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->Keys[GLFW_KEY_A] || this->Keys[GLFW_KEY_LEFT]) {
            if (Player->Position.x >= 0.0f)
                Player->Position.x -= velocity;
        }
        if (this->Keys[GLFW_KEY_D] || this->Keys[GLFW_KEY_RIGHT]) {
            if (Player->Position.x <= this->Width - Player->Size.x)
                Player->Position.x += velocity;
        }
   }
}


void Game::Render() {
    if (this->State == GAME_ACTIVE) {
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), 
            glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
    }
}
