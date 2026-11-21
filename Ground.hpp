#ifndef GROUND_H_ 
#define GROUND_H_ 

#include <vector>
#include <algorithm>
#include <memory>
#include <string>   
#include <cmath>
#include <iostream>
#include "Object.hpp"
#include <raymath.h>

class Ground : public Object {
  public:
    Ground(std::string vsPath, std::string fsPath);
    Ground(std::string vsPath, std::string fsPath, int width, int length,
        Vector3 origin);
    Ground(const Ground& other);
    Ground& operator=(Ground&& ground);
    void unloadResources();
    void draw() const override;
    Vector3 touchesGround(Vector3 objectPos) const;
    float getHeightAt(const float xPosition, const float zPosition) const;
    const Vector3& getOrigin() const;
  private:
    std::unique_ptr<Model> planeModel;
    std::unique_ptr<Mesh> planeMesh;
    std::vector<float> planeHeights;
    int meshWidth;
    int meshLength;
    Vector3 origin;

    int leastGreaterTwo(int n);
    void drawPlaneHeights(int width, int height, int numSamples) const;
    void initModel(std::string vsPath, std::string fsPath, Image& perlinNoiseImage);
    void initPlaneHeights(const Image& perlinNoiseImage, const int numSamples);
};

#endif
