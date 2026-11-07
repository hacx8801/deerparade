#include "Ground.hpp"

Ground::Ground(std::string vsPath, std::string fsPath) {
  // https://www.raylib.com/examples/shaders/loader.html?name=shaders_vertex_displacement
  Shader shader = LoadShader(vsPath.c_str(), fsPath.c_str());
  this->setShader(shader);

  // create perlin noise to use as texture
  Image perlinNoiseImage = GenImagePerlinNoise(512, 512, 0, 0, 1.0f);
  this->initModel(vsPath, fsPath, perlinNoiseImage);
  this->initPlaneHeights(perlinNoiseImage, 10);
  UnloadImage(perlinNoiseImage);

  // set the shader 
}

void Ground::draw() const {
  BeginShaderMode(this->getShader());

    /* DrawModel(planeModel, */ 
    /*     (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, */ 
    /*     (Color){ 255, 255, 255 }); */

    DrawModelWires(planeModel, 
        (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, 
        (Color){ 0, 0, 0 });

  this->drawPlaneHeights(10, 10, 10);
  EndShaderMode();
}

void Ground::drawPlaneHeights(int width, int height, int numSamples) const {
  width = 50;
  height = 50;

  float sampleWidthX = (float) width / (float) numSamples;
  float sampleWidthZ = (float) height / (float) numSamples;

  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numSamples; j++) {
      // sample the pixel value at the four
      // corners of the bounding box
      // void DrawCubeWires(Vector3 position,
      //  float width, 
      //  float height, 
      //  float length, 
      //  Color color);
      
      Vector3 position = (Vector3) {j*sampleWidthX - width/2.0f, 0.0f,  i*sampleWidthZ - height/2.0f};

      float height = planeHeights.at(i*(numSamples) + j) / 255.0f;

      Vector3 correction = (Vector3){sampleWidthX/2.0f, -1.0f, sampleWidthZ/2.0f};

      DrawCube(Vector3Add(position, correction), sampleWidthX, height, sampleWidthZ, RED);

    }
  }
}

void Ground::initModel(std::string vsPath, std::string fsPath, Image& perlinNoiseImage) {
  Texture perlinNoiseMap = LoadTextureFromImage(perlinNoiseImage);

  Shader shader = this->getShader();
    
  int perlinNoiseMapLoc = GetShaderLocation(shader, "perlinNoiseMap");
  rlEnableShader(shader.id);
  rlActiveTextureSlot(1);
  rlEnableTexture(perlinNoiseMap.id);
  rlSetUniformSampler(perlinNoiseMapLoc, 1);

  Mesh planeMesh = GenMeshPlane(50, 50, 50, 50);

  this->planeModel = LoadModelFromMesh(planeMesh);
  this->planeModel.materials[0].shader = shader;
}

void Ground::initPlaneHeights(const Image& perlinNoiseImage, const int numSamples) {
  
  this->planeHeights = std::vector<float>();


  float sampleWidthX = (float) perlinNoiseImage.width / (float) numSamples;
  float sampleWidthY = (float) perlinNoiseImage.height / (float) numSamples;

  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numSamples; j++) {
      // sample the pixel value at the four
      // corners of the bounding box
      Color point1 = GetImageColor(perlinNoiseImage, (int)j*sampleWidthX, (int)i*sampleWidthY);
      Color point2 = GetImageColor(perlinNoiseImage, (int)(j+1)*sampleWidthX, (int)i*sampleWidthY);
      Color point3 = GetImageColor(perlinNoiseImage, (int)j*sampleWidthX, (int)(i+1)*sampleWidthY);
      Color point4 = GetImageColor(perlinNoiseImage, (int)(j+1)*sampleWidthX, (int)(i+1)*sampleWidthY);

      int red = std::min(255, (point1.r+point2.r+point3.r+point4.r)/4);
      int green = std::min(255, (point1.g+point2.g+point3.g+point4.g)/4);
      int blue = std::min(255, (point1.b+point2.b+point3.b+point4.b)/4);

      Color avgColor = (Color) { (unsigned char) red, (unsigned char) green, (unsigned char) blue, 255 };

      this->planeHeights.push_back(avgColor.r*7);

    }
  }
  
}
