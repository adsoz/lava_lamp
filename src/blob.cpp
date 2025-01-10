#include "blob.h"

int Blob::n = 0;
const float Blob::speed = 0.25f;

void Blob::update(float deltaTime) {
        
        pos.y += vel.y * deltaTime;
        if (pos.y < -yRange) {
            pos.y = -yRange;
            vel.y = -vel.y;
        }
        if (pos.y > yRange) {
            pos.y = yRange;
            vel.y = -vel.y;
        }
    
    }