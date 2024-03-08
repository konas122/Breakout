#include "game.h"
#include "ball.h"
#include "object.h"
#include "resource_manager.h"
#include "sprite_renderer.h"


BallObject      *Ball;
GameObject      *Player;
SpriteRenderer  *Renderer;

const float PLAYER_VELOCITY(500.0f);
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);

const float BALL_RADIUS = 20.0f;
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}


Game::~Game() {
    delete Renderer;
    delete Player;
    delete Ball;
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

    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}


void Game::Update(float dt) {
    Ball->Move(dt, this->Width);
    this->DoCollisions();
}


void Game::ProcessInput(float dt) {
   if (this->State == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->Keys[GLFW_KEY_A] || this->Keys[GLFW_KEY_LEFT]) {
            if (Player->Position.x >= 0.0f) {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D] || this->Keys[GLFW_KEY_RIGHT]) {
            if (Player->Position.x <= this->Width - Player->Size.x) {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
   }
}


void Game::Render() {
    if (this->State == GAME_ACTIVE) {
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), 
            glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Ball->Draw(*Renderer);
    }
}


bool CheckCollision(GameObject &a, GameObject &b) {
    bool collisionX = a.Position.x + a.Size.x >= b.Position.x && b.Position.x + b.Size.x >= a.Position.x;
    bool collisionY = a.Position.y + a.Size.y >= b.Position.y && b.Position.y + b.Size.y >= a.Position.y;
    return collisionX && collisionY;
}

bool CheckCollision(BallObject &a, GameObject &b) {
    glm::vec2 center(a.Position + a.Radius);

    glm::vec2 aabb_half_extents(b.Size.x / 2, b.Size.y / 2);
    glm::vec2 aabb_center(
        b.Position.x + aabb_half_extents.x,
        b.Position.y + aabb_half_extents.y
    );

    glm::vec2 diff = center - aabb_center;
    glm::vec2 clamped = glm::clamp(diff, -aabb_half_extents, aabb_half_extents);
    glm::vec2 closest = aabb_center + clamped;
    return glm::length(center - closest) < a.Radius;
}


void Game::DoCollisions() {
    for (auto &obj : this->Levels[this->Level].Bricks) {
        if (!obj.Destroyed) {
            bool res = CheckCollision(*Ball, obj);
            if (res) {
                if (!obj.IsSolid)
                    obj.Destroyed = true;
            }
        }
    }
}
