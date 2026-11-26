#include "GroundTiler.hpp"
#include <algorithm>

/* GroundTile class member implementations */
GroundTile::GroundTile(int width, int length, Vector3 origin, Shader &shader)
    : tile{Ground(shader, width, length, origin)}, origin{origin},
      startX{origin.x - (width / 2.0f)}, endX{origin.x + (width / 2.0f)},
      startY{origin.y - (length / 2.0f)}, endY{origin.y + (length / 2.0f)} {}

const Vector3 &GroundTile::getOrigin() const { return this->tile.getOrigin(); }

/* GroundTiler class member implementations */
GroundTiler::GroundTiler(Shader &shader)
    : GroundTiler(shader, DEFAULT_WIDTH, DEFAULT_LENGTH) {}
GroundTiler::GroundTiler(Shader &shader, int width, int length)
    : Object{shader}, meshWidth{width}, meshLength{length}, arrayWidth{5},
      xExtent{meshWidth * arrayWidth}, zExtent{meshLength * arrayWidth} {
  // get the world coordinate offset for the top left of the tile grid
  float xOffset = floor(arrayWidth / 2.0f);
  float zOffset = floor(arrayWidth / 2.0f);

  // start pos for tiles, origin coordinates for top left grid tile
  Vector3 startPos =
      (Vector3){meshWidth * -xOffset, 0.0f, meshLength * -zOffset};

  // initialize the GroundTile array
  for (int z = 0; z < arrayWidth; z++) {
    for (int x = 0; x < arrayWidth; x++) {

      Vector3 origin =
          (Vector3){(float)x * meshWidth, 0.0f, (float)z * meshLength};

      origin.x += startPos.x;
      origin.z += startPos.z;

      this->tiles.emplace_back(std::make_unique<GroundTile>(
          GroundTile(meshWidth, meshLength, origin, shader)));
    }
  }
}

void GroundTiler::unloadResources() {
  /* std::for_each(this->tiles.begin(), this->tiles.end(), */
  /*               ([](std::unique_ptr<GroundTile> o) -> void { */
  /*                 o->tile.unloadResources(); */
  /*               })); */
  for (unsigned int i = 0; i < tiles.size(); i++) {
    tiles.at(i)->tile.unloadResources();
  }

  /* this->tiles.at(0).tile.unloadResources(); */
}

void GroundTiler::draw() const {
  for (unsigned int i = 0; i < tiles.size(); i++) {
    tiles.at(i)->tile.draw();
  }
}

Vector3 touchesGround(Vector3 objectPos) { return {0.0f, 0.0f, 0.0f}; }

const float GroundTiler::getMaxX() { return this->xExtent; }

const float GroundTiler::getMaxZ() { return this->zExtent; }

const float GroundTiler::getHeightAt(const float rawXPosition,
                                     const float rawZPosition) const {

  const float adjustedXPosition =
      rawXPosition + (arrayWidth / 2.0f) * meshWidth;
  const float adjustedZPosition =
      rawZPosition + (arrayWidth / 2.0f) * meshLength;

  const int widthIndex = floor(adjustedXPosition / meshWidth);
  const int lengthIndex = floor(adjustedZPosition / meshLength);

  // subtract the length of the previous tiles to get the local position
  const float localTileZ =
      adjustedZPosition - (meshLength * lengthIndex) - (meshLength * 0.5);
  const float localTileX =
      adjustedXPosition - (meshWidth * widthIndex) - (meshWidth * 0.5);

  // access the local tile and return the height
  int index = std::max(0, std::min(lengthIndex * arrayWidth + widthIndex,
                                   (int)arrayWidth * arrayWidth - 1));
  return tiles.at(index)->tile.getHeightAt(localTileX, localTileZ);
}
