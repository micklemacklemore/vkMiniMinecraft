#pragma once

#include <glm/glm.hpp>

#include <cstdint>

struct Input {
    bool wPressed, aPressed, sPressed, dPressed;
    bool spacePressed;
    int mouseX, mouseY;

    Input()
        : wPressed(false), aPressed(false), sPressed(false),
        dPressed(false), spacePressed(false), mouseX(0.f), mouseY(0.f)
    {
    }

    inline void reset() {
        wPressed = false;
        aPressed = false;
        sPressed = false;
        dPressed = false;
        spacePressed = false;
        mouseX = 0;
        mouseY = 0;
    }
};

class FPSCam {
private:
    glm::vec3 mForward, mRight, mUp;
    glm::vec3 mPosition;
    float mFovy;
    uint32_t mWidth, mHeight;       // Screen dimensions
    float mNearClip;                // Near clip plane distance
    float mFarClip;                 // Far clip plane distance
    float mAspect;                  // Aspect ratio
    // euler Angles
    float mYaw;                     
    float mPitch;                   
    // camera options
    float mMovementSpeed;           
    float mMouseSensitivity;        
public:
    // constructors
    FPSCam(uint32_t width, uint32_t height, glm::vec3 pos);

    void        setCameraWidthHeight(uint32_t w, uint32_t h);
    glm::mat4   getViewProjectionMatrix();
    void        processInput(Input input, float dt);
};