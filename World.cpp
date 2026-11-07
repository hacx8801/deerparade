#include "World.hpp"

void World::draw() const {
  std::for_each(this->objects.begin(), 
      this->objects.end(), 
      ([](const std::unique_ptr<Object>& o) -> void { o->draw(); }));
}

void World::add (std::unique_ptr<Object> object) {
  this->objects.push_back(std::move(object));
}

