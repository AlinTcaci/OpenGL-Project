#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;

        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
        
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD)
            this->cameraPosition += cameraFrontDirection * speed;
        if (direction == MOVE_BACKWARD)
            this->cameraPosition -= cameraFrontDirection * speed;
        if (direction == MOVE_RIGHT)
            this->cameraPosition += cameraRightDirection * speed;
        if (direction == MOVE_LEFT)
            this->cameraPosition -= cameraRightDirection * speed;
        if (direction == MOVE_UP)
            this->cameraPosition += cameraUpDirection * speed;
        if (direction == MOVE_DOWN)
            this->cameraPosition -= cameraUpDirection * speed;
    }

    //reset camera position
    void Camera::reset() {
        this->cameraPosition = glm::vec3(22.244263f, 12.364824f, 11.728957f);
        this->cameraTarget = glm::vec3(0.849834f, -0.347633f, 0.396149f);
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    }
   
    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        this->cameraFrontDirection = glm::normalize(direction);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));

    }

    void Camera::scenePresentation(float angle) {
        // set the camera
        this -> cameraPosition = glm::vec3(-8.0, 15, 40.0);

        // rotate with specific angle around Y axis
        glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // compute the new position of the camera
        // previous position * rotation matrix
        this -> cameraPosition = glm::vec4(r * glm::vec4(this -> cameraPosition, 1));
        this -> cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
}
