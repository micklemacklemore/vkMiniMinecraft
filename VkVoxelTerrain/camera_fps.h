#pragma once

#include "types.h"

#include <glm/glm.hpp>
#include <cstdint>

class CameraFPS {
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
    CameraFPS(uint32_t width, uint32_t height, glm::vec3 pos);

    void        setCameraWidthHeight(uint32_t w, uint32_t h);
    glm::mat4   getViewProjectionMatrix();
    void        processInput(Input input, float dt);
};