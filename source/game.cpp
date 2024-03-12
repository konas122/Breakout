#include "game.h"
#include "ball.h"
#include "object.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "particle.h"
#include "post_process.h"

// #define CHAOS_DEBBUG
// #define CONFUSE_DEBUG


BallObject      *Ball;
GameObject      *Player;
SpriteRenderer  *Renderer;
ParticleGenerator   *Particles;
PostProcessor   *Effects;

const float PLAYER_VELOCITY(500.0f);
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);

const float BALL_RADIUS = 20.0f;
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

float strength = 2.0f;
float ShakeTime = 0.0f;

void ActivatePowerUp(PowerUp &powerUp);


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}


Game::~Game() {
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
}


void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");
    ResourceManager::LoadShader("shaders/post_process.vs", "shaders/post_process.frag", nullptr, "postprocessing");

    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

    ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

    // load textures
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("textures/particle.png", true, "particle");
    ResourceManager::LoadTexture("textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "powerup_passthrough");

    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 1000);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);

#ifdef CHAOS_DEBBUG
    Effects->chaos = true;
#endif
#ifdef CONFUSE_DEBUG
    Effects->confuse = true;
#endif

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
    glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}


void Game::Update(float dt) {
    Ball->Move(dt, this->Width);
    this->DoCollisions();
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
    this->UpdatePowerUps(dt);

    if (ShakeTime > 0.0f) {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f)
            Effects->shake = false;
    }

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
        Effects->BeginRender();

        Renderer->DrawSprite(ResourceManager::GetTexture("background"), 
            glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
        );
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Particles->Draw();
        Ball->Draw(*Renderer);

        for (PowerUp &powerUp : this->PowerUps)
            if (!powerUp.Destroyed)
                powerUp.Draw(*Renderer);

        Effects->EndRender();
        Effects->Render((float)glfwGetTime());
    }
}


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

bool CheckCollision(GameObject &a, GameObject &b) {
    bool collisionX = a.Position.x + a.Size.x >= b.Position.x && b.Position.x + b.Size.x >= a.Position.x;
    bool collisionY = a.Position.y + a.Size.y >= b.Position.y && b.Position.y + b.Size.y >= a.Position.y;
    return collisionX && collisionY;
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
                if (!obj.IsSolid) {
                    obj.Destroyed = true;
                    this->SpawnPowerUps(obj);
                }
                else {
                    // if block is solid, enable shake effect
                    ShakeTime = 0.05f;
                    Effects->shake = true;
                }

                Direction dir = std::get<1>(res);
                glm::vec2 diff = std::get<2>(res);

                if (!(Ball->PassThrough && !obj.IsSolid)) {
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

        Ball->Stuck = Ball->Sticky;
    }

    for (auto &powerUp : this->PowerUps) {
        if (!powerUp.Destroyed) {
            if (powerUp.Position.y >= this->Height) {
                powerUp.Destroyed = true;
                continue;
            }
            
            if (CheckCollision(powerUp, *Player)) {
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
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
    Effects->chaos = Effects->confuse = false;
    Ball->PassThrough = Ball->Sticky = false;
    Player->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f);

    this->PowerUps.clear();
}


bool ShouldSpawn(unsigned int chance) {
    unsigned int random = rand() % chance;
    return random == 0;
}

void Game::SpawnPowerUps(GameObject &block) {
    if (ShouldSpawn(55))
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
    if (ShouldSpawn(55))
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
    if (ShouldSpawn(55))
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
    if (ShouldSpawn(55))
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
}

void ActivatePowerUp(PowerUp &powerUp) {
    if (powerUp.Type == "speed") {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky") {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through") {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase") {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse") {
        if (!Effects->chaos)
            Effects->confuse = true;
    }
    else if (powerUp.Type == "chaos") {
        if (!Effects->confuse)
            Effects->chaos = true;
    }
}

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type) {
    for (const PowerUp &powerUp : powerUps) {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}  

void Game::UpdatePowerUps(float dt) {
    for (PowerUp &powerUp : this->PowerUps) {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated) {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f) {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;

                // deactivate effects
                if (powerUp.Type == "sticky") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky")) {
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through")) {
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                        Effects->confuse = false;
                }
                else if (powerUp.Type == "chaos") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                        Effects->chaos = false;
                }
            }
        }
    }

    this->PowerUps.erase(
        std::remove_if(
            this->PowerUps.begin(),
            this->PowerUps.end(),
            [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
        ),
        this->PowerUps.end()
    );
}
