#ifndef GROUND_H_ 
#define GROUND_H_ 

#include <vector>
#include <algorithm>
#include <memory>
#include <string>   
#include <cmath>
#include "Object.hpp"
#include <raymath.h>

class Ground : public Object {
  public:
    Ground(std::string vsPath, std::string fsPath);
    void draw() const override;
    Vector3 touchesGround(Vector3 objectPos) const;
  private:
    Model planeModel;
    std::vector<float> planeHeights;
    void drawPlaneHeights(int width, int height, int numSamples) const;
    void initModel(std::string vsPath, std::string fsPath, Image& perlinNoiseImage);
    void initPlaneHeights(const Image& perlinNoiseImage, const int numSamples);
};

#endif
