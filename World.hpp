#ifndef WORLD_H_ 
#define WORLD_H_ 

#include <vector>
#include <algorithm>
#include <memory>
#include "Object.hpp"

class World : public Object {
  public:
    void draw() const override;
    void add(std::unique_ptr<Object> object);
  private:
    std::vector<std::unique_ptr<Object>> objects;
};

#endif
