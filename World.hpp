#ifndef WORLD_H_
#define WORLD_H_

#include "Object.hpp"
#include <memory>
#include <vector>

class World : public Object {
public:
  void draw() const override;
  void add(std::unique_ptr<Object> object);

private:
  std::vector<std::unique_ptr<Object>> objects;
};

#endif
