#define OLC_PGE_APPLICATION

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;


#include "olcPixelGameEngine.h"

class Asteroids : public olc::PixelGameEngine {
public :
    Asteroids() {
        sAppName = "Asteroids";
    }

private:
    struct sSpaceObject {
        float x;
        float y;
        float dx;
        float dy;
        int nSize;
        float angle;
    };
    
    vector<sSpaceObject> asteroids;
    sSpaceObject ship;
    vector<sSpaceObject> bullets;
    
    vector<pair<float, float>> shipModel;
    vector<pair<float, float>> asteroidModel;
    
    bool bDead = false;
    int score = 0;

protected:
public:
    bool OnUserCreate() override {
        
        ResetGame();
        
        // Create asteroids model
        int verts = 20;
        for (int i = 0; i < verts; i++) {
            float radius = (float) rand() / (float) RAND_MAX * 0.4f + 0.8f;
            // угол поворота в радианах
            // 0; 2Пи * 1/20; 2Пи * 2/20; 2Пи * 3/20; ... 2Пи * 19/20; 2Пи
            float a = ((float) i / (float) verts) * 6.28318f;
            asteroidModel.emplace_back(radius * sinf(a), radius * cosf(a));
        }
        
        
        // Create ship model
        shipModel = {
                {0.0f,  -5.0f},
                {-2.5f, +2.5f},
                {+2.5f, +2.5f}
        }; // простой равнобедренный треугольник
        
        
        return true;
    }
    
    bool OnUserUpdate(float fElapsedTime) override {
        
        if (bDead) ResetGame();
        
        FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
        
        // Steer the ship
        if (GetKey(olc::Key::LEFT).bHeld)
            ship.angle -= 5.0f * fElapsedTime;
        if (GetKey(olc::Key::RIGHT).bHeld)
            ship.angle += 5.0f * fElapsedTime;
        
        // Thrust
        if (GetKey(olc::Key::UP).bHeld) {
            // ACCELERATION changes VELOCITY (with respect to time)
            ship.dx += sin(ship.angle) * 20.0f * fElapsedTime;
            ship.dy += -cos(ship.angle) * 20.0f * fElapsedTime;
        }
        
        // VELOCITY changes POSITION (with respect to time)
        ship.x += ship.dx * fElapsedTime;
        ship.y += ship.dy * fElapsedTime;
        
        // Keep ship in game space
        WrapCoordinates(ship.x, ship.y, ship.x, ship.y);
        
        // Check ship collision with asteroids
        for (auto &asteroid: asteroids)
            if (IsPointInsideTheCircle(asteroid.x, asteroid.y, (float) asteroid.nSize, ship.x, ship.y))
                bDead = true;
        
        
        
        // Draw the ship
        DrawWireFrameModel(shipModel, ship.x, ship.y, ship.angle);
        
        DrawString(2, 2, "SCORE: " + to_string(score));
        
        // Fire bullet in direction the ship is facing
        if (GetKey(olc::Key::SPACE).bPressed)
            bullets.push_back(
                    {ship.x, ship.y, 50.0f * sinf(ship.angle), -50.0f * cosf(ship.angle), 0, 0}
            );
        
        // New asteroids that are born after a bullet hits one of the existing asteroids go here
        vector<sSpaceObject> newAsteroids;
        
        // Update and draw bullets
        for (auto &bullet: bullets) {
            bullet.x += bullet.dx * fElapsedTime;
            bullet.y += bullet.dy * fElapsedTime;
            
            Draw(bullet.x, bullet.y, olc::WHITE);
            
            // Check collision with asteroids
            for (auto &asteroid: asteroids) {
                if (IsPointInsideTheCircle(asteroid.x, asteroid.y, (float) asteroid.nSize, bullet.x, bullet.y)) {
                    // Asteroid is hit
                    
                    // Moves bullet to off screen space for it to be removed by remove_if function
                    // a little bit later
                    bullet.x = -100;
                    
                    if (asteroid.nSize > 4) {
                        // Create 2 child asteroids
                        float randomAngle1 = ((float) rand() / (float) RAND_MAX) * 6.283185f;
                        float randomAngle2 = ((float) rand() / (float) RAND_MAX) * 6.283185f;
                        newAsteroids.push_back(
                                {asteroid.x, asteroid.y,
                                 10.0f * sinf(randomAngle1), 10.0f * cosf(randomAngle1),
                                 (int) asteroid.nSize >> 1, 0.0f
                                }
                        );
                        newAsteroids.push_back(
                                {asteroid.x, asteroid.y,
                                 10.0f * sinf(randomAngle2), 10.0f * cosf(randomAngle2),
                                 (int) asteroid.nSize >> 1, 0.0f
                                }
                        );
                        
                    }
                    asteroid.x = -100;
                    score += 100;
                }
            }
        }
        // Add new asteroids to an existing pool
        asteroids.insert(asteroids.end(), newAsteroids.begin(), newAsteroids.end());
        
        // Remove off screen bullets
        if (!bullets.empty()) {
            auto iterator = remove_if(bullets.begin(), bullets.end(),
                                      [&](sSpaceObject o) {
                                          return o.x < 0 || o.x > ScreenWidth() || o.y < 0 || o.y > ScreenHeight();
                                      });
            if (iterator != bullets.end())
                bullets.erase(iterator);
        }
        // Remove off screen asteroids that where explicitly removed off the screen on bullet hit
        if (!asteroids.empty()) {
            auto iterator = remove_if(asteroids.begin(), asteroids.end(),
                                      [&](sSpaceObject o) { return o.x < 0; });
            if (iterator != asteroids.end())
                asteroids.erase(iterator);
        }
        
        if ( asteroids.empty())
        {
            // Level clear
            score += 1000; // reward
            
            // Add two new Asteroids, to the right and left of the player
            asteroids.push_back({ 30.0f * sinf(ship.angle - 3.14159f / 2.0f) + ship.x,
                                  30.0f * cosf(ship.angle - 3.14159f / 2.0f) + ship.y,
                                  10.0f * sinf(ship.angle),
                                  10.0f * cosf(ship.angle),
                                  (int) 16, 0.0f
                                  });
            asteroids.push_back({ 30.0f * sinf(ship.angle + 3.14159f / 2.0f) + ship.x,
                                  30.0f * cosf(ship.angle + 3.14159f / 2.0f) + ship.y,
                                  10.0f * sinf(-ship.angle),
                                  10.0f * cosf(-ship.angle),
                                  (int) 16, 0.0f
                                  });
        }
        
        
        // Update and draw asteroids
        for (auto &asteroid: asteroids) {
            asteroid.x += asteroid.dx * fElapsedTime;
            asteroid.y += asteroid.dy * fElapsedTime;
            asteroid.angle += 0.5f * fElapsedTime;
            
            WrapCoordinates(asteroid.x, asteroid.y, asteroid.x, asteroid.y);
            
            DrawWireFrameModel(
                    asteroidModel, asteroid.x, asteroid.y, asteroid.angle, (float) asteroid.nSize, olc::YELLOW);
        }
        
        return true;
    }
    
    void ResetGame() {
        
        asteroids.clear();
        bullets.clear();
        
        // Create asteroids
        asteroids.push_back({20.0f, 20.0f, 8.0f, -6.0f, (int) 16, 0.0f});
        asteroids.push_back({100.0f, 20.0f, -5.0f, 3.0f, (int) 16, 0.0f});
        
        // Create ship
        ship.x = (float) ScreenWidth() / 2;
        ship.y = (float) ScreenHeight() / 2;
        ship.dx = 0.0f;
        ship.dy = 0.0f;
        ship.angle = 0.0f;
        
        bDead = false;
        score = 0;
    }
    
    static bool IsPointInsideTheCircle(float circleX, float circleY, float radius, float x, float y) {
        return sqrt((circleX - x) * (circleX - x) + (circleY - y) * (circleY - y)) < radius;
    }
    
    void DrawWireFrameModel(
            const vector<pair<float, float>> &modelCoordinates,
            float xPosition,
            float yPosition,
            float rotationAngle = 0.0f,
            float scalingFactor = 1.0f,
            olc::Pixel pixel = olc::WHITE
    ) {
        // pair.first - x coordinate of a point in the model
        // pair.second - y coordinate of a point in the model
        
        // Create translated (moved) model - a vector of coordinate pairs
        vector<pair<float, float>> transformedCoordinates;
        int nVertices = modelCoordinates.size();
        transformedCoordinates.resize(nVertices);
        
        // Rotate
        for (int i = 0; i < nVertices; i++) {
            transformedCoordinates[i].first =
                    modelCoordinates[i].first * cosf(rotationAngle) - modelCoordinates[i].second * sinf(rotationAngle);
            transformedCoordinates[i].second =
                    modelCoordinates[i].first * sinf(rotationAngle) + modelCoordinates[i].second * cosf(rotationAngle);
        }
        
        // Scale
        for (int i = 0; i < nVertices; i++) {
            transformedCoordinates[i].first *= scalingFactor;
            transformedCoordinates[i].second *= scalingFactor;
        }
        
        // Translate
        for (int i = 0; i < nVertices; i++) {
            transformedCoordinates[i].first += xPosition;
            transformedCoordinates[i].second += yPosition;
        }
        
        // Draw
        for (int i = 0; i < nVertices; i++) {
            int j = i + 1;
            DrawLine(
                    transformedCoordinates[i % nVertices].first,
                    transformedCoordinates[i % nVertices].second,
                    transformedCoordinates[j % nVertices].first,
                    transformedCoordinates[j % nVertices].second,
                    pixel
            );
        }
        
        
    }
    
    void WrapCoordinates(float ix, float iy, float &ox, float &oy) {
        ox = ix;
        oy = iy;
        
        if (ix < 0.0f)
            ox = ix + (float) ScreenWidth();
        if (ix > (float) ScreenWidth())
            ox = ix - (float) ScreenWidth();
        
        if (iy < 0.0f)
            oy = iy + (float) ScreenHeight();
        if (iy > (float) ScreenHeight())
            oy = iy - (float) ScreenHeight();
    }
    
    bool Draw(int32_t x, int32_t y, olc::Pixel p) override {
        float fx, fy;
        WrapCoordinates(x, y, fx, fy);
        return PixelGameEngine::Draw(x, y, p);
    }
};

int main() {
    Asteroids game;
    if (game.Construct(200, 130, 5, 5))
        game.Start();
    
    return 0;
}
