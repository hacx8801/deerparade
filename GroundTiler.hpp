#ifndef GROUNDTILER_H_
#define GROUNDTILER_H_

#include "Ground.hpp"
#include "Object.hpp"
#include <assert.h>
#include <vector>

class GroundTile {

public:
  GroundTile(int width, int length, Vector3 origin, Shader &shader);
  const Vector3 &getOrigin() const;
  Ground tile;
  Vector3 origin;
  bool isDrawn;
  const float startX;
  const float endX;
  const float startY;
  const float endY;
};

// tiles ground objects based on player position
class GroundTiler : public Object {
public:
  GroundTiler(Shader &shader);
  GroundTiler(Shader &shader, int width, int length);
  void unloadResources();
  void draw() const override;
  Vector3 touchesGround(Vector3 objectPos) const;
  const float getMaxX();
  const float getMaxZ();
  const float getHeightAt(const float rawXPosition,
                          const float rawZPosition) const;

private:
  // default width for each single tile
  static const int DEFAULT_WIDTH = 50;
  // default height for each single tile
  static const int DEFAULT_LENGTH = 50;
  const std::string vsPath;
  const std::string fsPath;
  const int meshWidth;
  const int meshLength;
  // render distance in chunks
  const int arrayWidth;
  // total length and width to be maintained for the land area
  // = meshWidth * arrayWidth
  const int xExtent;
  // = meshLength * arrayWidth
  const int zExtent;
  std::vector<std::unique_ptr<GroundTile>> tiles;

  // determines which tile upon which the player is standing
  // after determining the tile, we convert the global player
  // coordinates to those local to the current tile.
  // once we have the local tile coordinates, we use the tile's getheight fn
  const Ground &getTile(float rawXPosition, float rawZPosition);
};

#endif
