/**
 * @file main.c
 * @brief Demo Raylib 3D: aeronave, alvo e HUD com ângulos esféricos.
 *
 * Visualiza uma aeronave e um alvo em 3D, calcula azimute/elevação do alvo e do
 * vetor de frente da aeronave e deriva os ângulos esféricos j, J, E, F e G para
 * desenhar um HUD. Pressione H para alternar rótulos/anotações didáticas.
 */
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @defgroup config Constantes de configuração
 * @brief Parâmetros globais de HUD, janela e interação.
 * @{ 
 */
/** Largura padrão da janela (px). */
static const int DEFAULT_SCREEN_WIDTH = 1280;
/** Altura padrão da janela (px). */
static const int DEFAULT_SCREEN_HEIGHT = 720;
/** Velocidade de movimento (unid/s) para aeronave e alvo. */
static const float MOVE_SPEED = 5.0f;
/** Velocidade de rotação (graus/s). */
static const float ROT_SPEED_DEG = 45.0f;
/** Velocidade de rotação (rad/s). */
static const float ROT_SPEED = (float)(ROT_SPEED_DEG * M_PI / 180.0);
/** Fator de pixels por radiano para o HUD (raio do marcador). */
static const float HUD_PIXELS_PER_RAD = 220.0f;
/** @} */

/**
 * @brief Converte graus para radianos.
 * @param deg Valor em graus.
 * @return Valor em radianos.
 */
static inline float rad(float deg) { return deg*(float)M_PI/180.0f; }

/**
 * @brief Converte radianos para graus.
 * @param radv Valor em radianos.
 * @return Valor em graus.
 */
static inline float deg(float radv) { return radv*180.0f/(float)M_PI; }

/**
 * @brief Versão segura de acosf com clamp do argumento em [-1, 1].
 */
static inline float safe_acos(float x) {
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    return acosf(x);
}

/**
 * @brief Calcula azimute e elevação do alvo T em relação à aeronave A.
 *
 * Fórmulas:
 *  - Az = atan2( X_T - X_A, Y_T - Y_A )
 *  - El = atan2( Z_T - Z_A, sqrt((X_T-X_A)^2 + (Y_T-Y_A)^2) )
 *
 * @param A Posição da aeronave.
 * @param T Posição do alvo.
 * @param Az [out] Azimute (rad).
 * @param El [out] Elevação (rad).
 */
static void ComputeAzEl(Vector3 A, Vector3 T, float *Az, float *El)
{
    Vector3 d = { T.x - A.x, T.y - A.y, T.z - A.z };
    float horiz = sqrtf(d.x*d.x + d.y*d.y);
    float az = atan2f(d.x, d.y); // note order per user's formula
    float el = atan2f(d.z, horiz);
    if (Az) *Az = az;
    if (El) *El = el;
}

/**
 * @brief Calcula o vetor de frente a partir de yaw/pitch/roll.
 *
 * Mundo Z-up; yaw em Z, pitch em X, roll em Y. O vetor base é +Y do corpo.
 * @param yaw Rotação yaw (rad).
 * @param pitch Rotação pitch (rad).
 * @param roll Rotação roll (rad).
 * @return Vetor unitário frente da aeronave.
 */
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

/**
 * @brief Calcula azimute/elevação de um vetor no espaço.
 * @param v Vetor 3D (não precisa ser unitário).
 * @param Az [out] Azimute (rad).
 * @param El [out] Elevação (rad).
 */
static void ComputeAzElFromVector(Vector3 v, float *Az, float *El)
{
    float horiz = sqrtf(v.x*v.x + v.y*v.y);
    float az = atan2f(v.x, v.y);
    float el = atan2f(v.z, horiz);
    if (Az) *Az = az;
    if (El) *El = el;
}

/**
 * @brief Deriva ângulos esféricos a partir de Az/El do alvo (T) e do vetor frente (R).
 *
 * Calcula j e G, além dos intermediários E, F, J usados no HUD.
 * @param AzT Azimute do alvo (rad).
 * @param ElT Elevação do alvo (rad).
 * @param AzR Azimute do vetor frente (rad).
 * @param ElR Elevação do vetor frente (rad).
 * @param out_j [out] Ângulo j (rad).
 * @param out_G [out] Ângulo G (rad).
 * @param out_E [out] Ângulo E (rad).
 * @param out_F [out] Ângulo F (rad).
 * @param out_J [out] Ângulo J (rad).
 */
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
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
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

/**
 * @brief Desenha um modelo simples de aeronave como uma seta em A com yaw/pitch/roll.
 * @param A Posição da aeronave.
 * @param yaw Yaw (rad).
 * @param pitch Pitch (rad).
 * @param roll Roll (rad).
 * @param col Cor do corpo.
 */
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

/**
 * @brief Projeta ponto 3D para tela e desenha texto próximo a ele.
 */
static void DrawTextAt3D(Camera3D cam, Vector3 p, const char *text, int fontSize, Color col, int screenW, int screenH)
{
    Vector2 s = GetWorldToScreenEx(p, cam, screenW, screenH);
    DrawText(text, (int)s.x + 6, (int)s.y - fontSize - 2, fontSize, col);
}

/**
 * @brief Desenha um arco 3D no plano definido por u e v, centrado em originC.
 * @param originC Centro do arco.
 * @param u Vetor unitário inicial.
 * @param v Vetor unitário final.
 * @param angleJ Ângulo entre u e v (rad).
 * @param radius Raio do arco.
 * @param col Cor do arco.
 */
static void DrawArc3D(Vector3 originC, Vector3 u, Vector3 v, float angleJ, float radius, Color col)
{
    if (angleJ <= 1e-5f) return;
    // plane normal
    Vector3 n = Vector3CrossProduct(u, v);
    float nn = Vector3Length(n);
    if (nn < 1e-6f) return; // nearly colinear
    n = Vector3Scale(n, 1.0f/nn);
    // w = normalized (n x u) lies on the plane and orthogonal to u
    Vector3 w = Vector3CrossProduct(n, u);
    float wn = Vector3Length(w);
    if (wn < 1e-6f) return;
    w = Vector3Scale(w, 1.0f/wn);

    const int steps = 32;
    float dt = angleJ/steps;
    Vector3 prev = Vector3Add(originC, Vector3Scale(Vector3Add(Vector3Scale(u, cosf(0)), Vector3Scale(w, sinf(0))), radius));
    for (int i = 1; i <= steps; ++i)
    {
        float t = dt * i;
        Vector3 curDir = Vector3Add(Vector3Scale(u, cosf(t)), Vector3Scale(w, sinf(t)));
        Vector3 cur = Vector3Add(originC, Vector3Scale(curDir, radius));
        DrawLine3D(prev, cur, col);
        prev = cur;
    }
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

    bool showAnn = true; // toggle annotations

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
        if (IsKeyPressed(KEY_H))  showAnn = !showAnn; // toggle annotations

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

        // Annotations in 3D: forward vector and arc j
        Vector3 noseLineEnd = Vector3Add(A, Vector3Scale(fwd, 4.0f));
        DrawLine3D(A, noseLineEnd, BLUE);
        if (showAnn)
        {
            // unit vectors from A
            Vector3 u = fwd; // already unit
            Vector3 dAT = Vector3Subtract(T, A);
            float dn = Vector3Length(dAT);
            if (dn > 1e-6f)
            {
                Vector3 v = Vector3Scale(dAT, 1.0f/dn);
                DrawArc3D(A, u, v, j, 1.5f, PURPLE);
            }
        }

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

        DrawText("Controls: Aircraft I/K J/L U/O, Target W/S A/D Q/E, Yaw/Pitch Arrows, Roll Z/X, Orbit Cam RMB, Toggle labels H",
                 16, screenHeight-28, 16, DARKGRAY);

        // 2D annotations projected from 3D if enabled
        if (showAnn)
        {
            // Labels for A and T
            DrawTextAt3D(cam, A, "A (aeronave)", 16, DARKBLUE, screenWidth, screenHeight);
            DrawTextAt3D(cam, T, "T (alvo)", 16, MAROON, screenWidth, screenHeight);

            // Label for forward vector R at its end
            DrawTextAt3D(cam, Vector3Add(A, Vector3Scale(fwd, 4.2f)), "R (eixo de rolagem)", 16, BLUE, screenWidth, screenHeight);

            // Midpoint along arc j for label
            Vector3 dAT = Vector3Subtract(T, A);
            float dn = Vector3Length(dAT);
            if (dn > 1e-6f && j > 1e-3f)
            {
                Vector3 u = fwd;
                Vector3 v = Vector3Scale(dAT, 1.0f/dn);
                Vector3 n = Vector3Normalize(Vector3CrossProduct(u, v));
                Vector3 w = Vector3Normalize(Vector3CrossProduct(n, u));
                float tmid = j*0.5f;
                Vector3 midDir = Vector3Add(Vector3Scale(u, cosf(tmid)), Vector3Scale(w, sinf(tmid)));
                Vector3 midPos = Vector3Add(A, Vector3Scale(midDir, 1.6f));
                DrawTextAt3D(cam, midPos, "j", 18, PURPLE, screenWidth, screenHeight);

                // Show Az/El near the A->T line midpoint
                Vector3 midAT = Vector3Add(A, Vector3Scale(dAT, 0.5f));
                char lab[128];
                snprintf(lab, sizeof(lab), "AzT=%.0f° ElT=%.0f°", deg(AzT), deg(ElT));
                DrawTextAt3D(cam, midAT, lab, 16, MAROON, screenWidth, screenHeight);

                // Show AzR/ElR near forward vector end
                snprintf(lab, sizeof(lab), "AzR=%.0f° ElR=%.0f°", deg(AzR), deg(ElR));
                DrawTextAt3D(cam, Vector3Add(A, Vector3Scale(fwd, 4.6f)), lab, 16, BLUE, screenWidth, screenHeight);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
