#include "blob.h"

int Blob::n = 0;
float Blob::speed = 0.25f;

void Blob::update(float deltaTime) {
        
        pos.y += speed * vel.y * deltaTime;
        if (pos.y < -yRange) {
            pos.y = -yRange;
            vel.y = -vel.y;
        }
        if (pos.y > yRange) {
            pos.y = yRange;
            vel.y = -vel.y;
        }
    
    }