#pragma once

// Utility functions

float randf(float min, float max) {

  return ((float)rand() * (max-min) / (float)RAND_MAX) + min;

}
