#include "Ground.hpp"
#include "GroundTiler.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

// movement constraints
#define GRAVITY 32.0f
#define MAX_SPEED 20.0f
#define CROUCH_SPEED 5.0f
#define JUMP_FORCE 12.0f
#define MAX_ACCEL 150.0f

// shorthand
#define uint unsigned int
#define null NULL

// grounded drag
#define FRICTION 0.86f
// increasing drag for air to increase strafing speed
#define AIR_DRAG 0.98f

#define CONTROL 15.0f
#define CROUCH_HEIGHT 0.0f
#define STAND_HEIGHT 1.0f
#define BOTTOM_HEIGHT 0.5f

#define NORMALIZE_INPUT 0

#define TREE_DENSITY 100

float getDistance(float x1, float z1, float x2, float z2);

int randInRange(int min, int max) {
  return min + (int)(rand() / (double)(RAND_MAX) * (max - min + 1));
}

bool checkValidTreePlacement(const Vector3 &oldPos, const Vector3 &newPos,
                             const float MIN_DIST) {
  float dist = getDistance(oldPos.x, oldPos.z, newPos.x, newPos.z);
  return dist > MIN_DIST;
}

void initTreeLocations(std::vector<std::pair<Vector3, float>> &positions,
                       GroundTiler &ground, float treeDensity, int xMax,
                       int zMax, const int MAX_TREES) {

  const float heightOffset = 2.5f;
  const float fracBottom = 100.0f;
  treeDensity = std::max(0.0f, std::min(fracBottom, treeDensity));

  // generate a value in (0, 1)
  const float treeFactor = treeDensity / fracBottom;

  // O(n^2)...
  for (int i = 0; i < MAX_TREES;) {
    float randval = randInRange(0, 1);
    if (randval < treeFactor) {
      i++;
      float xPos = randInRange(-xMax / 2.0f, xMax / 2.0f);
      float zPos = randInRange(-zMax / 2.0f, zMax / 2.0f);
      float yPos = ground.getHeightAt(xPos, zPos) + heightOffset;

      Vector3 pos = (Vector3){xPos, yPos, zPos};

      bool flag = true;
      for (unsigned int j = 0; j < positions.size(); j++) {
        flag = checkValidTreePlacement(positions.at(j).first, pos, 20.0f);
      }

      /* std::cout << "flag is " << flag << std::endl; */
      if (flag) {
        positions.emplace_back(
            std::make_pair<Vector3, float>(std::move(pos), -1.0f));
      }
    }
  }
}

float getDistance(float x1, float z1, float x2, float z2) {
  return std::sqrt((x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2));
}

void drawTreeLocations(Texture &texTree,
                       std::vector<std::pair<Vector3, float>> &positions,
                       Camera &camera, Shader &treeShader) {
  const int MIN_TREE_SIZE = 8;
  const int MAX_TREE_SIZE = 16;

  int len = positions.size();
  for (int i = 0; i < len; i++) {
    positions.at(i).second =
        getDistance(camera.position.x, camera.position.z,
                    positions.at(i).first.x, positions.at(i).first.z);
  }

  auto cmp = [](const std::pair<Vector3, float> &a,
                const std::pair<Vector3, float> &b) {
    if (fabs(a.second - b.second) < 0.1) {
      return a.first.y > b.first.y;
    }
    return a.second > b.second;
  };

  std::sort(positions.begin(), positions.end(), cmp);

  BeginShaderMode(treeShader);
  for (int i = 0; i < len; i++) {
    float size = 10.0f; //(float)randInRange(MIN_TREE_SIZE, MAX_TREE_SIZE);
    DrawBillboard(camera, texTree, positions.at(i).first, size, WHITE);
  }
  EndShaderMode();
}

// body structure
typedef struct {
  Vector3 position;
  Vector3 velocity;
  Vector3 dir;
  bool isGrounded;
} Body;

// global variable definitions
static Vector2 sensitivity = {0.001f, 0.001f};

static Body player = {0};
static Vector2 lookRotation = {0};
static float headTimer = 0.0f;
static float walkLerp = 0.0f;
static float headLerp = STAND_HEIGHT;
static Vector2 lean = {0};

// module function delcarations
static void UpdateCameraFPS(Camera *camera);
static void UpdateBody(Body *body, float rot, char side, char forward,
                       bool jumpPressed, bool crouchHold, float floorHeight);
void DrawLevel(Ground &ground, std::vector<Vector3> terrain, int arrSize);
void DrawLevel(GroundTiler &ground, std::vector<Vector3> terrain, int arrSize);
float GetRandom(unsigned int scalar);
void GenerateTerrainArray(std::vector<Vector3> &arr, int arrSize, int width,
                          int height);

int main(void) {
  // initialize data
  const int screenWidth = 1600;
  const int screenHeight = 900;

  // seed for random terrain height
  srand((uint)time(NULL));

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "DEER/parade");
  // initialize camera variables to pass to UpdateCameraFPS()
  Camera camera = {0};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;
  camera.position = (Vector3){
      player.position.x,
      player.position.y + (BOTTOM_HEIGHT + headLerp),
      player.position.z,
  };
  uint width = 50;
  uint height = 50;

  /* Vector3 origin = (Vector3){0.0f, 0.0f, 0.0f}; */
  /* FIXME: allocate shader outside of groundtile to pass by reference */
  /* GroundTiler ground = GroundTiler("terrain_shader_grass_vs.glsl", */
  /*                                  "terrain_shader_grass_fs.glsl"); */

  UpdateCameraFPS(&camera);
  DisableCursor();

  SetTargetFPS(60);

  float ratio = 6.0f / 10;
  uint arrSize = (int)floor((float)width * ratio);

  std::vector<Vector3> terrain;
  GenerateTerrainArray(terrain, arrSize, width, height);

  // billboard testing
  Image tree = LoadImage("./tree.png");
  ImageResize(&tree, tree.width * 3, tree.height * 3);

  Texture texBillboard = LoadTextureFromImage(tree); // Our billboard

  Shader shader = LoadShader("terrain_shader_grass_vs.glsl",
                             "terrain_shader_grass_fs.glsl");

  Shader treeShader = LoadShader(NULL, "tree_shader_fs.glsl");

  GroundTiler ground = GroundTiler(shader);
  Vector3 billPositionStatic = {0.0f, 5.1f,
                                0.0f}; // Position of static billboard

  std::vector<std::pair<Vector3, float>> treePositions;
  initTreeLocations(treePositions, ground, TREE_DENSITY, ground.getMaxX(),
                    ground.getMaxZ(), 200);

  // minimize z fighting in billboards

  // main game loop
  while (!WindowShouldClose()) {
    // get delta for mouse movement
    Vector2 mouseDelta = GetMouseDelta();

    // change look rotation based on mouse movement delta
    lookRotation.x -= mouseDelta.x * sensitivity.x;
    lookRotation.y += mouseDelta.y * sensitivity.y;

    float height = 0;
    height = ground.getHeightAt(player.position.x, player.position.z);
    // get inputs
    char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
    UpdateBody(&player, lookRotation.x, sideway, forward,
               IsKeyPressed(KEY_SPACE), crouching, height);

    float delta = GetFrameTime();

    // update headLerp to converge on crouch vs stand status
    headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT),
                    20.0f * delta);

    // update camera to match player's current position
    camera.position = (Vector3){
        player.position.x,
        player.position.y + (BOTTOM_HEIGHT + headLerp),
        player.position.z,
    };

    // lerp fov and set head bob timer when moving
    if (player.isGrounded && ((forward != 0) || (sideway != 0))) {
      headTimer += delta * 3.0f;
      walkLerp = Lerp(walkLerp, 1.0f, 10.0f * delta);
      camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f * delta);
    }
    // lerp to zero if not
    else {
      walkLerp = Lerp(walkLerp, 0.0f, 10.0f * delta);
      camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f * delta);
    }

    lean.x = Lerp(lean.x, sideway * 0.02f, 10.0f * delta);
    lean.y = Lerp(lean.y, forward * 0.015f, 10.0f * delta);

    UpdateCameraFPS(&camera);

    // start drawing
    BeginDrawing();

    ClearBackground(SKYBLUE);

    BeginMode3D(camera);

    DrawLevel(ground, terrain, arrSize);
    drawTreeLocations(texBillboard, treePositions, camera, treeShader);

    EndMode3D();

    // draw the info box
    DrawRectangle(5, 5, 330, 75, Fade(YELLOW, 0.5f));
    DrawRectangleLines(5, 5, 330, 75, BLACK);

    DrawText("Camera controls:", 15, 15, 10, BLACK);
    DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
    DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
    DrawText(TextFormat("- Velocity Len: (%06.3f)",
                        Vector2Length(
                            (Vector2){player.velocity.x, player.velocity.y})),
             15, 60, 10, BLACK);

    EndDrawing();
  }

  // de-init window and ogl context
  ground.unloadResources();

  if (IsShaderValid(shader)) {
    UnloadShader(shader);
  }

  CloseWindow();

  return 0;
}

void UpdateBody(Body *body, float rot, char side, char forward,
                bool jumpPressed, bool crouchHold, float floorHeight) {
  Vector2 input = (Vector2){(float)side, (float)-forward};

#ifdef NORMALIZE_INPUT
  if ((side != 0) && (forward != 0))
    input = Vector2Normalize(input);
#endif

  float delta = GetFrameTime();

  if (body->position.y > floorHeight) {
    body->isGrounded = false;
  }

  // if the body is in the air, account for gravity acceleration
  // m/s each frame is acceleration if (!body->isGrounded) body->velocity.y
  // -= GRAVITY*delta;
  if (!body->isGrounded)
    body->velocity.y -= GRAVITY * delta;

  if (body->isGrounded && jumpPressed) {
    body->velocity.y = JUMP_FORCE;
    body->isGrounded = false;
  }

  // forward and right vector based on camera rotation,
  // we construct these for moving the camera.
  Vector3 front = (Vector3){sinf(rot), 0.f, cosf(rot)};
  Vector3 right = (Vector3){cosf(-rot), 0.f, sinf(-rot)};

  Vector3 desiredDir = (Vector3){input.x * right.x + input.y * front.x, 0.0f,
                                 input.x * right.z + input.y * front.z};

  body->dir = Vector3Lerp(body->dir, desiredDir, CONTROL * delta);

  float decel = (body->isGrounded ? FRICTION : AIR_DRAG);
  Vector3 hvel =
      (Vector3){body->velocity.x * decel, 0.0f, body->velocity.z * decel};

  float hvelLength = Vector3Length(hvel);
  if (hvelLength < (MAX_SPEED * 0.01f))
    hvel = (Vector3){0};

  // this is what creates strafing?
  float speed = Vector3DotProduct(hvel, body->dir);

  float maxSpeed = (crouchHold ? CROUCH_SPEED : MAX_SPEED);
  float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL * delta);
  hvel.x += body->dir.x * accel;
  hvel.z += body->dir.z * accel;

  body->velocity.x = hvel.x;
  body->velocity.z = hvel.z;

  body->position.x += body->velocity.x * delta;
  body->position.y += body->velocity.y * delta;
  body->position.z += body->velocity.z * delta;

  if (body->position.y <= floorHeight) {
    body->position.y = Lerp(body->position.y, floorHeight, CONTROL * delta);
    /* std::cout << "height is " << floorHeight << std::endl; */
    body->velocity.y = 0.0f;
    body->isGrounded = true;
  }
}

void UpdateCameraFPS(Camera *camera) {
  const Vector3 up = (Vector3){0.0f, 1.0f, 0.0f};
  const Vector3 targetOffset = (Vector3){0.0f, 0.0f, -1.0f};

  Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x);

  // clamp view up
  float maxAngleUp = Vector3Angle(up, yaw);
  maxAngleUp -= 0.001f; // avoid numerical errors
  if (-(lookRotation.y) > maxAngleUp) {
    lookRotation.y = -maxAngleUp;
  }

  // clamp view down
  float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
  maxAngleDown *= -1.0f;
  maxAngleDown += 0.001f; // avoid numerical errors
  if (-(lookRotation.y) < maxAngleDown) {
    lookRotation.y = -maxAngleDown;
  }

  // up and down
  Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

  // rotate the view vector around the right axis
  float pitchAngle = -lookRotation.y - lean.y;
  pitchAngle = Clamp(pitchAngle, -PI / 2 + 0.0001f, PI / 2 - 0.0001f);
  Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);

  // head animation
  // rotate up direction around forward axis
  float headSin = sinf(headTimer * PI);
  float headCos = cosf(headTimer * PI);
  const float stepRotation = 0.01f;
  camera->up =
      Vector3RotateByAxisAngle(up, pitch, headSin * stepRotation + lean.x);

  // camera BOB
  const float bobSide = 0.1f;
  const float bobUp = 0.15f;
  Vector3 bobbing = Vector3Scale(right, headSin * bobSide);
  bobbing.y = fabsf(headCos * bobUp);

  camera->position =
      Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
  camera->target = Vector3Add(camera->position, pitch);
}

void GenerateTerrainArray(std::vector<Vector3> &arr, int arrSize, int width,
                          int height) {
  arr = std::vector<Vector3>();

  for (int i = 0; i < arrSize; i++) {
    float ei = (float)i / (float)arrSize;

    for (int j = 0; j < arrSize; j++) {
      float ej = (float)j / (float)arrSize;
      float w = (ej - 0.5f) * (float)width;
      float h = (ei - 0.5f) * (float)height;
      arr.push_back({w, GetRandom(1), h});
    }
  }
}

float GetRandom(unsigned int scalar) {
  float scale = (float)rand() / ((float)RAND_MAX + 1.0f);
  return (scale - 0.5f) + (float)scalar;
}

void printvec3(Vector3 vec) { printf("\t(%f, %f, %f) ", vec.x, vec.y, vec.z); }

void DrawLevel(Ground &ground, std::vector<Vector3> terrain, int arrSize) {
  ground.draw();
}

void DrawLevel(GroundTiler &ground, std::vector<Vector3> terrain, int arrSize) {
  ground.draw();
}
