#ifndef GROUND_H_
#define GROUND_H_

#include "Object.hpp"
#include <cmath>
#include <memory>
#include <raymath.h>
#include <string>
#include <vector>

class Ground : public Object {
public:
  Ground(Shader &shader);
  Ground(Shader &shader, int width, int length, Vector3 origin);
  Ground(const Ground &other);
  Ground &operator=(Ground &&ground);
  void unloadResources();
  void draw() const override;
  Vector3 touchesGround(Vector3 objectPos) const;
  float getHeightAt(const float xPosition, const float zPosition) const;
  const Vector3 &getOrigin() const;

private:
  std::unique_ptr<Model> planeModel;
  std::unique_ptr<Mesh> planeMesh;
  std::vector<float> planeHeights;
  int meshWidth;
  int meshLength;
  Vector3 origin;

  int leastGreaterTwo(int n);
  void drawPlaneHeights(int width, int height, int numSamples) const;
  void initModel(Image &perlinNoiseImage);
  void initPlaneHeights(const Image &perlinNoiseImage, const int numSamples);
};

#endif
