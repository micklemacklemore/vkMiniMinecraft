#include "camera_fps.h"

#include <iostream>

CameraFPS::CameraFPS(uint32_t width, uint32_t height, glm::vec3 pos)
    : mForward(0, 0, -1),
    mRight(1, 0, 0),
    mUp(0, 1, 0),
    mPosition(pos),
    mFovy(45.f),
    mWidth(width),
    mHeight(height),
    mNearClip(0.1f),
    mFarClip(10000.f),
    mAspect(width / static_cast<float>(height)),
    mYaw(-90.f),
    mPitch(0.0f),
    mMovementSpeed(10.f),
    mMouseSensitivity(0.1f) {
}

void CameraFPS::setCameraWidthHeight(uint32_t w, uint32_t h) {
    mWidth = w;
    mHeight = h;
    mAspect = w / static_cast<float>(h);
}

glm::mat4 CameraFPS::getViewProjectionMatrix() {
    glm::mat4 persp = glm::perspective(glm::radians(mFovy), mAspect, mNearClip, mFarClip); 
    glm::mat4 view = glm::lookAt(mPosition, mPosition + mForward, mUp);
    persp[1][1] *= -1; // flip the Y for vulkan compatible
    return persp * view; 
}

void CameraFPS::processInput(Input input, float dt) {
    // std::cout << "xrel: " << input.mouseX << " yrel: " << input.mouseY << "\n"; 

    if (input.mouseX|| input.mouseY) {
        float xoffset = input.mouseX * mMouseSensitivity;
        float yoffset = input.mouseY * mMouseSensitivity;

        mYaw += xoffset;
        mPitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (mPitch > 89.0f)
            mPitch = 89.0f;
        if (mPitch < -89.0f)
            mPitch = -89.0f;

        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
        front.y = sin(glm::radians(mPitch));
        front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
        mForward = glm::normalize(front);
        // also re-calculate the Right and Up vector
        mRight = glm::normalize(glm::cross(mForward, glm::vec3(0., 1., 0.)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        mUp = glm::normalize(glm::cross(mRight, mForward));
    };

    float velocity = mMovementSpeed * dt; // velocity as a function of dt
    if (input.wPressed)
        mPosition += mForward * velocity;
    if (input.sPressed)
        mPosition -= mForward * velocity;
    if (input.aPressed)
        mPosition -= mRight * velocity;
    if (input.dPressed)
        mPosition += mRight * velocity;
    if (input.ePressed)
        mPosition += mUp * velocity; 
    if (input.qPressed)
        mPosition -= mUp * velocity; 
}
