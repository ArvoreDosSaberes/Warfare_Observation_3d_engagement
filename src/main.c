#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float rad(float deg) { return deg*(float)M_PI/180.0f; }
static inline float deg(float radv) { return radv*180.0f/(float)M_PI; }

// Compute atan2-like safe acos
static inline float safe_acos(float x) {
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    return acosf(x);
}

// Computes azimuth and elevation per spec:
// Az = atan2( (X_T - X_A), (Y_T - Y_A) )
// El = atan2( (Z_T - Z_A), sqrt((X_T-X_A)^2 + (Y_T-Y_A)^2) )
static void ComputeAzEl(Vector3 A, Vector3 T, float *Az, float *El)
{
    Vector3 d = { T.x - A.x, T.y - A.y, T.z - A.z };
    float horiz = sqrtf(d.x*d.x + d.y*d.y);
    float az = atan2f(d.x, d.y); // note order per user's formula
    float el = atan2f(d.z, horiz);
    if (Az) *Az = az;
    if (El) *El = el;
}

// Forward vector from yaw/pitch/roll (Z-up world; yaw about Z, pitch about X', roll about Y'')
// We define aircraft body forward along +Y_body before rotation.
static Vector3 ForwardFromYPR(float yaw, float pitch, float roll)
{
    // Build rotation matrices and apply to forward = (0,1,0).
    // Yaw around Z
    float cy = cosf(yaw), sy = sinf(yaw);
    // Pitch around X
    float cp = cosf(pitch), sp = sinf(pitch);
    // Roll around Y (does not affect forward magnitude but included for completeness)
    float cr = cosf(roll), sr = sinf(roll);

    // Compose R = Rz(yaw) * Rx(pitch) * Ry(roll)
    // Apply to forward v=(0,1,0)
    // First Ry(roll) on v:
    Vector3 v = { 0.0f*cr + 0.0f*sr, 1.0f, -0.0f*sr + 0.0f*cr }; // still (0,1,0)
    // Then Rx(pitch):
    Vector3 v2 = { v.x, cp*v.y - sp*v.z, sp*v.y + cp*v.z };
    // Then Rz(yaw):
    Vector3 v3 = { cy*v2.x - sy*v2.y, sy*v2.x + cy*v2.y, v2.z };
    // Normalize
    float n = sqrtf(v3.x*v3.x + v3.y*v3.y + v3.z*v3.z);
    if (n > 0) { v3.x/=n; v3.y/=n; v3.z/=n; }
    return v3;
}

// Compute az/el for roll axis (i.e., forward vector) relative to world
static void ComputeAzElFromVector(Vector3 v, float *Az, float *El)
{
    float horiz = sqrtf(v.x*v.x + v.y*v.y);
    float az = atan2f(v.x, v.y);
    float el = atan2f(v.z, horiz);
    if (Az) *Az = az;
    if (El) *El = el;
}

// From spec relations in spherical trigonometry
// Given AzT, ElT and AzR, ElR, compute j and G (also provide intermediates for HUD display)
static void ComputeSphericalAngles(float AzT, float ElT, float AzR, float ElR,
                                   float *out_j, float *out_G,
                                   float *out_E, float *out_F, float *out_J)
{
    float cf = cosf(AzT)*cosf(ElT);
    float f = safe_acos(cf);

    float ch = cosf(AzR)*cosf(ElR);
    float h = safe_acos(ch);

    // ctn(C) = sin(AzT)/tan(ElT) => C = atan2(tan(ElT), sin(AzT))
    float C = atan2f(tanf(ElT), sinf(AzT));
    // ctn(D) = sin(AzR)/tan(ElR) => D = atan2(tan(ElR), sin(AzR))
    float D = atan2f(tanf(ElR), sinf(AzR));

    float J = (float)M_PI - C - D;

    // cos(j) = cos(f)cos(h) + sin(f)sin(h)cos(J)
    float j = safe_acos(cosf(f)*cosf(h) + sinf(f)*sinf(h)*cosf(J));

    // E from ctn(E) = sin(ElR)/tan(AzR) => E = atan2(tan(AzR), sin(ElR))
    float E = atan2f(tanf(AzR), sinf(ElR));

    // F via sin(F) = sin(J)*sin(f)/sin(j)
    float denom = sinf(j);
    float F = 0.0f;
    if (fabsf(denom) > 1e-6f) {
        float s = sinf(J)*sinf(f)/denom;
        if (s > 1.0f) s = 1.0f; if (s < -1.0f) s = -1.0f;
        F = asinf(s);
    } else {
        F = 0.0f;
    }

    float G = (float)M_PI - E - F;

    if (out_j) *out_j = j;
    if (out_G) *out_G = G;
    if (out_E) *out_E = E;
    if (out_F) *out_F = F;
    if (out_J) *out_J = J;
}

// Draw simple aircraft as an arrow aligned with yaw/pitch/roll at position A
static void DrawAircraft(Vector3 A, float yaw, float pitch, float roll, Color col)
{
    // Build basis vectors from yaw/pitch/roll: forward, right, up
    Vector3 fwd = ForwardFromYPR(yaw, pitch, roll);

    // Approximate right = normalize( fwd x worldUp ) then up = right x fwd
    Vector3 worldUp = {0,0,1};
    Vector3 right = Vector3CrossProduct(fwd, worldUp);
    float rn = sqrtf(right.x*right.x + right.y*right.y + right.z*right.z);
    if (rn < 1e-6f) right = (Vector3){1,0,0}; else { right.x/=rn; right.y/=rn; right.z/=rn; }
    Vector3 up = Vector3CrossProduct(right, fwd);

    float bodyLen = 3.0f;
    float bodyRad = 0.2f;

    Vector3 nose = { A.x + fwd.x*bodyLen, A.y + fwd.y*bodyLen, A.z + fwd.z*bodyLen };
    // Draw body
    DrawCylinderEx(A, nose, bodyRad, 0.01f, 16, col);
    // Wings
    Vector3 wL = { A.x + right.x*1.2f, A.y + right.y*1.2f, A.z + right.z*1.2f };
    Vector3 wR = { A.x - right.x*1.2f, A.y - right.y*1.2f, A.z - right.z*1.2f };
    DrawCylinderEx(wL, wR, 0.05f, 0.05f, 8, Fade(col, 0.8f));
    // Tail fin
    Vector3 tailTop = { A.x + up.x*0.8f, A.y + up.y*0.8f, A.z + up.z*0.8f };
    DrawCylinderEx(A, tailTop, 0.03f, 0.03f, 8, Fade(col, 0.8f));
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Warfare Observation 3D Engagement - Raylib");
    SetTargetFPS(60);

    // 3D Camera
    Camera3D cam = {0};
    cam.position = (Vector3){ 12.0f, -16.0f, 10.0f };
    cam.target   = (Vector3){ 0.0f, 0.0f, 1.0f };
    cam.up       = (Vector3){ 0.0f, 0.0f, 1.0f };
    cam.fovy     = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    // Entities
    Vector3 A = {0,0,2}; // aircraft position
    Vector3 T = {8,6,4}; // target position

    // Aircraft orientation (radians)
    float yaw = rad(20.0f);
    float pitch = rad(-5.0f);
    float roll = rad(15.0f);

    // Interaction speeds
    const float moveSpeed = 5.0f;
    const float rotSpeed = rad(45.0f); // deg/s

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Controls - Move Aircraft (IJKL + U/O for Z)
        if (IsKeyDown(KEY_I)) A.y += moveSpeed*dt;
        if (IsKeyDown(KEY_K)) A.y -= moveSpeed*dt;
        if (IsKeyDown(KEY_J)) A.x -= moveSpeed*dt;
        if (IsKeyDown(KEY_L)) A.x += moveSpeed*dt;
        if (IsKeyDown(KEY_U)) A.z += moveSpeed*dt;
        if (IsKeyDown(KEY_O)) A.z -= moveSpeed*dt;

        // Controls - Move Target (WASD + Q/E for Z)
        if (IsKeyDown(KEY_W)) T.y += moveSpeed*dt;
        if (IsKeyDown(KEY_S)) T.y -= moveSpeed*dt;
        if (IsKeyDown(KEY_A)) T.x -= moveSpeed*dt;
        if (IsKeyDown(KEY_D)) T.x += moveSpeed*dt;
        if (IsKeyDown(KEY_Q)) T.z += moveSpeed*dt;
        if (IsKeyDown(KEY_E)) T.z -= moveSpeed*dt;

        // Controls - Orientation of aircraft (Arrow keys + Z/X for roll)
        if (IsKeyDown(KEY_LEFT))  yaw -= rotSpeed*dt;
        if (IsKeyDown(KEY_RIGHT)) yaw += rotSpeed*dt;
        if (IsKeyDown(KEY_UP))    pitch += rotSpeed*dt;
        if (IsKeyDown(KEY_DOWN))  pitch -= rotSpeed*dt;
        if (IsKeyDown(KEY_Z))     roll -= rotSpeed*dt;
        if (IsKeyDown(KEY_X))     roll += rotSpeed*dt;

        // Optional: orbit camera with mouse left button
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 d = GetMouseDelta();
            Matrix rotZ = MatrixRotate((Vector3){0,0,1}, -d.x*0.003f);
            Vector3 off = Vector3Subtract(cam.position, A);
            off = Vector3Transform(off, rotZ);
            // elevate
            Matrix rotX = MatrixRotate((Vector3){1,0,0}, d.y*0.003f);
            off = Vector3Transform(off, rotX);
            cam.position = Vector3Add(A, off);
        }
        cam.target = A;

        // Computations
        float AzT=0, ElT=0; ComputeAzEl(A, T, &AzT, &ElT);
        Vector3 fwd = ForwardFromYPR(yaw, pitch, roll);
        float AzR=0, ElR=0; ComputeAzElFromVector(fwd, &AzR, &ElR);

        float j=0, G=0, E=0, F=0, J=0; // radians
        ComputeSphericalAngles(AzT, ElT, AzR, ElR, &j, &G, &E, &F, &J);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(cam);
        DrawGrid(40, 1.0f);
        // axes
        DrawLine3D((Vector3){0,0,0}, (Vector3){5,0,0}, RED);
        DrawLine3D((Vector3){0,0,0}, (Vector3){0,5,0}, GREEN);
        DrawLine3D((Vector3){0,0,0}, (Vector3){0,0,5}, BLUE);

        // Draw aircraft and target
        DrawAircraft(A, yaw, pitch, roll, DARKBLUE);
        DrawSphere(T, 0.4f, MAROON);
        DrawLine3D(A, T, Fade(MAROON, 0.6f));

        EndMode3D();

        // HUD overlay
        int cx = screenWidth/2;
        int cy = screenHeight/2;

        // Radius proportional to j (radians)
        float kpix = 220.0f; // px per radian
        float r = kpix * j;
        if (r > screenHeight*0.45f) r = screenHeight*0.45f;

        // HUD angle = G + roll (roll is body roll about forward; we use same sign)
        float hudAng = G + roll;
        float hx = cx + r * sinf(hudAng);  // screen x grows to right
        float hy = cy - r * cosf(hudAng);  // screen y grows downwards

        // Draw HUD elements
        DrawCircleLines(cx, cy, 12, BLACK);
        DrawCircleLines(cx, cy, (int)(kpix*rad(10)), LIGHTGRAY);
        DrawCircleLines(cx, cy, (int)(kpix*rad(20)), LIGHTGRAY);
        DrawCircleLines(cx, cy, (int)(kpix*rad(30)), LIGHTGRAY);

        DrawLine(cx-20, cy, cx+20, cy, DARKGRAY);
        DrawLine(cx, cy-20, cx, cy+20, DARKGRAY);

        DrawCircle((int)hx, (int)hy, 6, MAROON);
        DrawCircleLines((int)hx, (int)hy, 10, MAROON);

        // Text readouts
        char buf[256];
        snprintf(buf, sizeof(buf),
                 "AzT=%.1f deg  ElT=%.1f deg  AzR=%.1f deg  ElR=%.1f deg",
                 deg(AzT), deg(ElT), deg(AzR), deg(ElR));
        DrawText(buf, 16, 16, 18, BLACK);

        snprintf(buf, sizeof(buf),
                 "j=%.2f deg  J=%.2f deg  E=%.2f deg  F=%.2f deg  G=%.2f deg",
                 deg(j), deg(J), deg(E), deg(F), deg(G));
        DrawText(buf, 16, 40, 18, BLACK);

        DrawText("Controls: Aircraft I/K J/L U/O, Target W/S A/D Q/E, Yaw/Pitch Arrows, Roll Z/X, Orbit Cam RMB",
                 16, screenHeight-28, 16, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
