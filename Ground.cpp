#include "Ground.hpp"
#include <cassert>

Ground::Ground(std::string vsPath, std::string fsPath)
    : Ground(vsPath, fsPath, 50, 50, (Vector3){0.0f, 0.0f, 0.0f}) {}

Ground::Ground(std::string vsPath, std::string fsPath, int width, int length,
               Vector3 origin)
    : meshWidth{width}, meshLength{length}, origin{origin} {
  // https://www.raylib.com/examples/shaders/loader.html?name=shaders_vertex_displacement
  this->setShader(LoadShader(vsPath.c_str(), fsPath.c_str()));
  assert(IsShaderValid(shader));

  // create perlin noise to use as texture
  int texWidth = leastGreaterTwo(width * 10);
  int texLength = leastGreaterTwo(length * 10);
  /* std::cout << "size w is: " << texWidth << std::endl; */
  /* std::cout << "size l is: " << texLength << std::endl; */

  Image perlinNoiseImage =
      GenImagePerlinNoise(texWidth, texLength, 10, 10, 1.0f);
  this->initModel(vsPath, fsPath, perlinNoiseImage);
  this->initPlaneHeights(perlinNoiseImage, 10);
  UnloadImage(perlinNoiseImage);
}

Ground::Ground(const Ground &other) {
  planeModel = nullptr;
  planeMesh = nullptr;

  planeModel = std::make_unique<Model>(*(other.planeModel));
  planeMesh = std::make_unique<Mesh>(*(other.planeMesh));
  planeHeights = other.planeHeights;

  meshWidth = other.meshWidth;
  meshLength = other.meshLength;
  origin = other.origin;
}

Ground &Ground::operator=(Ground &&other) {

  if (this != &other) {
    planeModel = nullptr;
    planeMesh = nullptr;

    planeModel = std::move(other.planeModel);
    planeMesh = std::move(other.planeMesh);
    planeHeights = std::move(other.planeHeights);

    meshWidth = other.meshWidth;
    meshLength = other.meshLength;
    origin = other.origin;

    other.planeModel = nullptr;
    other.planeMesh = nullptr;
  }

  return *this;
}

void Ground::unloadResources() {

  if (IsShaderValid(shader)) {
    UnloadShader(this->shader);
    this->shader.id = -1;
  }
  UnloadModel(*(this->planeModel));
}

void Ground::draw() const {

  BeginShaderMode(shader);

  DrawModel(*(this->planeModel), this->origin, 1.0f, (Color){0, 0, 0});

  EndShaderMode();
}

int Ground::leastGreaterTwo(int n) {
  if (n <= 1) {
    return 1;
  }

  int num = 1;
  while ((n & 0xf) != 0xf) {
    n |= (n >> num);
    num *= 2;
  }

  return n + 1;
}

// for debugging
void Ground::drawPlaneHeights(int width, int height, int numSamples) const {
  width = meshWidth;
  height = meshLength;

  float sampleWidthX = (float)width / (float)numSamples;
  float sampleWidthZ = (float)height / (float)numSamples;

  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numSamples; j++) {

      Vector3 position =
          (Vector3){(float)j * sampleWidthX - (float)width / 2.0f, 0.0f,
                    (float)i * sampleWidthZ - (float)height / 2.0f};

      float height = planeHeights.at(i * (numSamples) + j);

      Vector3 correction = (Vector3){sampleWidthX / 2.0f, height / 2.0f - 2.0f,
                                     sampleWidthZ / 2.0f};

      DrawCubeWires(Vector3Add(position, correction), sampleWidthX, height,
                    sampleWidthZ, RED);
    }
  }
}

void Ground::initModel(std::string vsPath, std::string fsPath,
                       Image &perlinNoiseImage) {
  Texture perlinNoiseMap = LoadTextureFromImage(perlinNoiseImage);

  rlEnableShader(shader.id);
  int perlinNoiseMapLoc = GetShaderLocation(shader, "perlinNoiseMap");
  rlActiveTextureSlot(1);
  rlEnableTexture(perlinNoiseMap.id);
  rlSetUniformSampler(perlinNoiseMapLoc, 1);

  this->planeMesh = std::make_unique<Mesh>(
      GenMeshPlane(meshWidth, meshLength, meshWidth, meshLength));

  this->planeModel =
      std::make_unique<Model>(LoadModelFromMesh(*(this->planeMesh)));
  this->planeModel->materials[0].shader = this->shader;
  rlDisableShader();
}

void Ground::initPlaneHeights(const Image &perlinNoiseImage,
                              const int numSamples) {

  this->planeHeights = std::vector<float>();

  float sampleWidthX = (float)perlinNoiseImage.width / (float)numSamples;
  float sampleWidthY = (float)perlinNoiseImage.height / (float)numSamples;
  sampleWidthX = floor(sampleWidthX);
  sampleWidthY = floor(sampleWidthY);

  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numSamples; j++) {
      // sample the pixel value at the four
      // corners of the bounding box
      Color point1 = GetImageColor(perlinNoiseImage, j * (int)sampleWidthX,
                                   i * (int)sampleWidthY);
      Color point2 = GetImageColor(
          perlinNoiseImage, (j + 1) * (int)sampleWidthX, i * (int)sampleWidthY);
      Color point3 = GetImageColor(perlinNoiseImage, j * (int)sampleWidthX,
                                   (i + 1) * (int)sampleWidthY);
      Color point4 =
          GetImageColor(perlinNoiseImage, (j + 1) * (int)sampleWidthX,
                        (i + 1) * (int)sampleWidthY);

      int red = std::min(255, (point1.r + point2.r + point3.r + point4.r) / 4);
      int green =
          std::min(255, (point1.g + point2.g + point3.g + point4.g) / 4);
      int blue = std::min(255, (point1.b + point2.b + point3.b + point4.b) / 4);

      Color avgColor = (Color){(unsigned char)red, (unsigned char)green,
                               (unsigned char)blue, 255};

      this->planeHeights.push_back(avgColor.r * 7 / 255.0f);
    }
  }
}

float Ground::getHeightAt(const float xPosition, const float zPosition) const {
  float x = xPosition + (meshWidth / 2.0f);
  float z = zPosition + (meshLength / 2.0f);

  const int NUM_SAMPLES_X = 10;
  const int NUM_SAMPLES_Z = 10;

  float convFacX = (float)NUM_SAMPLES_X / meshWidth;
  float convFacZ = (float)NUM_SAMPLES_Z / meshLength;
  x *= convFacX;
  z *= convFacZ;

  int index = std::max(0, std::min((int)this->planeHeights.size() - 1,
                                   (int)z * (NUM_SAMPLES_Z) + (int)x));

  float height = planeHeights.at(index);
  return height;
}

const Vector3 &Ground::getOrigin() const { return this->origin; }
