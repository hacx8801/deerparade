#ifndef OBJECT_H_
#define OBJECT_H_

#include <raylib.h>
#include <rlgl.h>

class Object {
  public:
    // pure virtual function
    virtual void draw() const = 0;
    virtual void setShader(Shader shader);
    virtual Shader getShader() const;
    virtual void setPosition(Vector3 position);
    virtual Vector3 getPosition() const;
  private:
    Vector3 position;
    Shader shader;
};

#endif
