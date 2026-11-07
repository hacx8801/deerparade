#include "Object.hpp"

void Object::setShader(Shader shader) {
  this->shader = shader;
}

Shader Object::getShader() const {
  return this->shader;
}

void Object::setPosition(Vector3 position) {
  this->position = position;
}

Vector3 Object::getPosition() const  {
  return this->position;
}
