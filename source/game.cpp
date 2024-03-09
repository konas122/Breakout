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

float strength = 2.0f;

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

    if (Ball->Position.y >= this->Height) {
        this->ResetLevel();
        this->ResetPlayer();
    }
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


// bool CheckCollision(GameObject &a, GameObject &b) {
//     bool collisionX = a.Position.x + a.Size.x >= b.Position.x && b.Position.x + b.Size.x >= a.Position.x;
//     bool collisionY = a.Position.y + a.Size.y >= b.Position.y && b.Position.y + b.Size.y >= a.Position.y;
//     return collisionX && collisionY;
// }

// bool CheckCollision(BallObject &a, GameObject &b) {
//     glm::vec2 center(a.Position + a.Radius);

//     glm::vec2 aabb_half_extents(b.Size.x / 2, b.Size.y / 2);
//     glm::vec2 aabb_center(
//         b.Position.x + aabb_half_extents.x,
//         b.Position.y + aabb_half_extents.y
//     );

//     glm::vec2 diff = center - aabb_center;
//     glm::vec2 clamped = glm::clamp(diff, -aabb_half_extents, aabb_half_extents);
//     glm::vec2 closest = aabb_center + clamped;
//     return glm::length(center - closest) < a.Radius;
// }

Direction VectorDirection(glm::vec2 target) {
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++) {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max) {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

Collision CheckCollision(BallObject &a, GameObject &b) {
    glm::vec2 center(a.Position + a.Radius);

    glm::vec2 aabb_half_extents(b.Size.x / 2, b.Size.y / 2);
    glm::vec2 aabb_center(
        b.Position.x + aabb_half_extents.x,
        b.Position.y + aabb_half_extents.y
    );

    glm::vec2 diff = center - aabb_center;
    glm::vec2 clamped = glm::clamp(diff, -aabb_half_extents, aabb_half_extents);
    glm::vec2 closest = aabb_center + clamped;
    diff = closest - center;
    if (glm::length(diff) < a.Radius) {
        return std::make_tuple(true, VectorDirection(diff), diff);
    } else {
        return std::make_tuple(false, UP, glm::vec2(0, 0));
    }
}


void Game::DoCollisions() {
    // collision between Ball and Brisks
    for (auto &obj : this->Levels[this->Level].Bricks) {
        if (!obj.Destroyed) {
            Collision res = CheckCollision(*Ball, obj);
            if (std::get<0>(res)) {
                if (!obj.IsSolid)
                    obj.Destroyed = true;

                Direction dir = std::get<1>(res);
                glm::vec2 diff = std::get<2>(res);
                if (dir == LEFT || dir == RIGHT) {
                    Ball->Velocity.x = -Ball->Velocity.x;
                    float penetration = Ball->Radius - std::abs(diff.x);
                    if (dir == LEFT)
                        Ball->Position.x += penetration;
                    else
                        Ball->Position.x -= penetration;
                }
                else {
                    Ball->Velocity.y = -Ball->Velocity.y;
                    float penetration = Ball->Radius - std::abs(diff.y);
                    if (dir == UP)
                        Ball->Position.y -= penetration;
                    else
                        Ball->Position.y += penetration;
                }
            }
        }
    }

    // collision between Ball and Player
    Collision res = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(res)) {
        float center = Player->Position.x + Player->Size.x / 2;
        float distance = (Ball->Position.x + Ball->Radius) - center;
        float percentage = distance / (Player->Size.x / 2);

        glm::vec2 oldV = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldV);
        Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);
    }
}


void Game::ResetLevel() {
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer() {
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}
