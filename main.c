// ============================================================================
//  VEGETABLE NINJA — a Fruit Ninja style slicing game, all-procedural.
//
//  Rendering: raylib / OpenGL 3.3. The whole scene is drawn into a 2x
//  supersampled render target (SSAA) and composited through a GLSL
//  post-process shader (vignette + chromatic aberration + flash), with a
//  second GLSL shader generating the animated dojo background on the GPU.
//  All vegetable art, the splat decal and every sound effect are generated
//  at startup — the exe has zero external assets.
//
//  Controls: hold LEFT MOUSE and swipe to slice. Don't slice bombs.
//            F11 fullscreen, P pause, F fps, ESC menu/quit.
// ============================================================================

#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "font_data.h"
#include "levels.h"
#include "progress.h"

// ----------------------------------------------------------- configuration
#define GAME_W        1600            // design-space resolution (all logic)
#define GAME_H        900
#define RT_SCALE      2               // supersampling factor for the target
#define ART           256             // per-vegetable art canvas size

#define MAX_VEG       48
#define MAX_HALF      96
#define MAX_PART      4096
#define MAX_SPLAT     96
#define MAX_POPUP     24
#define MAX_TRAIL     48
#define MAX_MOTES     70
#define SND_VOICES    5

#define GRAVITY       1500.0f
#define SLICE_SPEED   500.0f          // min blade speed (px/s) to cut

typedef enum { VEG_CARROT, VEG_TOMATO, VEG_BROCCOLI, VEG_EGGPLANT,
               VEG_CORN, VEG_PUMPKIN, VEG_ONION, VEG_CUCUMBER, VEG_COUNT } VegType;

typedef enum { ST_MENU, ST_MAP, ST_PLAY, ST_COMPLETE, ST_FAILED } GameState;

typedef struct { bool active, isBomb; VegType type;
                 Vector2 pos, vel; float rot, rotVel, radius; } Veg;

typedef struct { bool active; VegType type; int side;   // 0 = left half
                 Vector2 pos, vel; float rot, rotVel, life; } Half;

typedef struct { bool active, additive; Vector2 pos, vel;
                 float life, maxLife, size, drag; Color color; } Particle;

typedef struct { bool active; Vector2 pos; float rot, size, life, maxLife;
                 Color color; } Splat;

typedef struct { bool active; char text[40]; Vector2 pos;
                 float life, size; Color color; } Popup;

typedef struct { Vector2 pos; double t; } TrailPt;

// ----------------------------------------------------------------- globals
static const float VEG_RADIUS[VEG_COUNT] = { 56, 54, 62, 62, 60, 70, 52, 62 };
static Color VEG_JUICE[VEG_COUNT];    // filled at init

static RenderTexture2D gVegTex[VEG_COUNT];
static RenderTexture2D gBombTex, gSplatTex, gTarget;
static Shader   gPostShader, gBgShader;
static int      gLocAberr, gLocFlash, gLocBgTime, gLocBgRes;
static Font     gFont;

static Sound gSndSlice[SND_VOICES], gSndSplat[SND_VOICES];
static Sound gSndBomb, gSndCombo, gSndLaunch, gSndThud;
static int   gVoiceSlice = 0, gVoiceSplat = 0;
static bool  gAudioOk = false;

static Veg      gVeg[MAX_VEG];
static Half     gHalf[MAX_HALF];
static Particle gPart[MAX_PART];
static Splat    gSplat[MAX_SPLAT];
static Popup    gPopup[MAX_POPUP];
static TrailPt  gTrail[MAX_TRAIL];
static int      gTrailLen = 0;
static Particle gMotes[MAX_MOTES];

static GameState gState = ST_MENU;
static int   gScore, gLives;
static float gSpawnTimer, gShake, gAberr, gFlash;

// active stage state (all objective logic reads gLevel — never a level id)
static const LevelDef *gLevel = NULL;
static float gStageTime;
static int   gSliced, gSpawnedVeg, gMaxCombo;
static int   gOutcome;              // 0 = playing, 1 = success, 2 = failed
static float gOutcomeTimer;         // lets effects breathe before the screen switch
static int   gStarsEarned;
static float gAccuracy;
static int   gComboCount;  static float gComboTimer; static Vector2 gComboPos;
static bool  gPaused = false, gShowFps = false;
static float gStateTime = 0;          // time since entering current state

static Vector2 gMouse, gMousePrev;    // design-space blade position
static float   gRealDt = 1.0f/60;     // unclamped frame time (blade speed gate)
static bool    gBladeDown = false;
static bool    gPointerPressed = false;  // press edge, this frame
static bool    gSelfTest = false;  static int gSelfFrame = 0;
static bool    gQuit = false;

// ------------------------------------------------------------------ shaders
// Two variants per shader: GLSL 330 for desktop GL 3.3, GLSL ES 100 for
// WebGL. Same math, different dialect (varying/texture2D/gl_FragColor).
#if defined(PLATFORM_WEB)

static const char *POST_FS =
"#version 100\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"varying vec2 fragTexCoord; varying vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform float aberr;\n"
"uniform float flash;\n"
"void main(){\n"
"  vec2 uv = fragTexCoord;\n"
"  vec2 d  = uv - 0.5;\n"
"  float ab = aberr * 0.014;\n"
"  vec3 col;\n"
"  col.r = texture2D(texture0, uv + d*ab).r;\n"
"  col.g = texture2D(texture0, uv).g;\n"
"  col.b = texture2D(texture0, uv - d*ab).b;\n"
"  float vig = smoothstep(1.0, 0.30, length(d)*1.25);\n"
"  col *= mix(0.55, 1.05, vig);\n"
"  col = mix(vec3(dot(col, vec3(0.299,0.587,0.114))), col, 1.12);\n"
"  col += vec3(flash);\n"
"  gl_FragColor = vec4(col, 1.0);\n"
"}\n";

static const char *BG_FS =
"#version 100\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"uniform float time;\n"
"uniform vec2  resolution;\n"
"float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }\n"
"void main(){\n"
"  vec2 uv = gl_FragCoord.xy / resolution;\n"
"  vec3 top = vec3(0.075, 0.135, 0.115);\n"
"  vec3 bot = vec3(0.022, 0.050, 0.045);\n"
"  vec3 col = mix(bot, top, uv.y);\n"
"  float ar = resolution.x / resolution.y;\n"
"  float d = length((uv - vec2(0.5, 0.78)) * vec2(ar, 1.0));\n"
"  col += vec3(0.09, 0.13, 0.10) * smoothstep(1.0, 0.0, d);\n"
"  float a = atan(uv.x - 0.5, 1.25 - uv.y);\n"
"  float rays = sin(a*9.0 + time*0.33) * sin(a*13.0 - time*0.19);\n"
"  rays = smoothstep(0.15, 1.0, rays);\n"
"  col += vec3(0.040, 0.058, 0.048) * rays * smoothstep(1.25, 0.15, d);\n"
"  float ring = abs(length((uv-vec2(0.5,0.5))*vec2(ar,1.0)) - 0.42);\n"
"  col += vec3(0.020, 0.030, 0.025) * smoothstep(0.05, 0.0, ring);\n"
"  col += (hash(uv * resolution + fract(time)) - 0.5) * 0.014;\n"
"  gl_FragColor = vec4(col, 1.0);\n"
"}\n";

#else  // desktop ------------------------------------------------------------

static const char *POST_FS =
"#version 330\n"
"in vec2 fragTexCoord; in vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform float aberr;\n"
"uniform float flash;\n"
"out vec4 finalColor;\n"
"void main(){\n"
"  vec2 uv = fragTexCoord;\n"
"  vec2 d  = uv - 0.5;\n"
"  float ab = aberr * 0.014;\n"
"  vec3 col;\n"
"  col.r = texture(texture0, uv + d*ab).r;\n"
"  col.g = texture(texture0, uv).g;\n"
"  col.b = texture(texture0, uv - d*ab).b;\n"
"  float vig = smoothstep(1.0, 0.30, length(d)*1.25);\n"
"  col *= mix(0.55, 1.05, vig);\n"
"  col = mix(vec3(dot(col, vec3(0.299,0.587,0.114))), col, 1.12);\n" // saturation
"  col += vec3(flash);\n"
"  finalColor = vec4(col, 1.0);\n"
"}\n";

static const char *BG_FS =
"#version 330\n"
"uniform float time;\n"
"uniform vec2  resolution;\n"
"out vec4 finalColor;\n"
"float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }\n"
"void main(){\n"
"  vec2 uv = gl_FragCoord.xy / resolution;\n"
"  vec3 top = vec3(0.075, 0.135, 0.115);\n"
"  vec3 bot = vec3(0.022, 0.050, 0.045);\n"
"  vec3 col = mix(bot, top, uv.y);\n"
"  float ar = resolution.x / resolution.y;\n"
"  float d = length((uv - vec2(0.5, 0.78)) * vec2(ar, 1.0));\n"
"  col += vec3(0.09, 0.13, 0.10) * smoothstep(1.0, 0.0, d);\n"
"  float a = atan(uv.x - 0.5, 1.25 - uv.y);\n"                 // god rays
"  float rays = sin(a*9.0 + time*0.33) * sin(a*13.0 - time*0.19);\n"
"  rays = smoothstep(0.15, 1.0, rays);\n"
"  col += vec3(0.040, 0.058, 0.048) * rays * smoothstep(1.25, 0.15, d);\n"
"  float ring = abs(length((uv-vec2(0.5,0.5))*vec2(ar,1.0)) - 0.42);\n"
"  col += vec3(0.020, 0.030, 0.025) * smoothstep(0.05, 0.0, ring);\n" // dojo circle
"  col += (hash(uv * resolution + fract(time)) - 0.5) * 0.014;\n"     // film grain
"  finalColor = vec4(col, 1.0);\n"
"}\n";

#endif // PLATFORM_WEB

// ------------------------------------------------------------------ helpers
static float frand(float a, float b) {
    return a + (b - a) * ((float)GetRandomValue(0, 100000) / 100000.0f);
}
static float PointSegDist(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 ab = Vector2Subtract(b, a);
    float len2 = Vector2LengthSqr(ab);
    if (len2 < 1e-6f) return Vector2Distance(p, a);
    float t = Clamp(Vector2DotProduct(Vector2Subtract(p, a), ab) / len2, 0, 1);
    return Vector2Distance(p, Vector2Add(a, Vector2Scale(ab, t)));
}
static void DrawTextShadow(const char *s, float x, float y, float size, Color c) {
    DrawTextEx(gFont, s, (Vector2){ x + size*0.05f, y + size*0.05f }, size, 2,
               (Color){ 0, 0, 0, (unsigned char)(c.a*0.6f) });
    DrawTextEx(gFont, s, (Vector2){ x, y }, size, 2, c);
}
static void DrawTextCentered(const char *s, float cx, float y, float size, Color c) {
    Vector2 m = MeasureTextEx(gFont, s, size, 2);
    DrawTextShadow(s, cx - m.x/2, y, size, c);
}

// =========================================================== procedural art
static void ArtCarrot(void) {
    float cx = ART/2.0f;
    for (int i = 0; i < 40; i++) {                       // tapered body
        float t = i / 39.0f;
        float y = 70 + t * 155;
        float r = Lerp(34, 5, t);
        DrawCircleV((Vector2){ cx, y }, r, (Color){ 235, 120, 30, 255 });
    }
    for (int i = 0; i < 5; i++) {                        // ridges
        float y = 95 + i * 26;
        float w = Lerp(30, 10, i / 4.0f);
        DrawLineEx((Vector2){ cx - w, y }, (Vector2){ cx + w, y + 3 }, 4,
                   (Color){ 200, 90, 15, 255 });
    }
    DrawCircle((int)cx - 10, 105, 9, (Color){ 255, 165, 80, 160 }); // shine
    for (int i = -1; i <= 1; i++) {                      // stalk
        Vector2 tip = { cx + i * 22, 18 };
        DrawTriangle((Vector2){ cx - 8, 72 }, (Vector2){ cx + 8, 72 }, tip,
                     (Color){ 70, 160, 60, 255 });
    }
}
static void ArtTomato(void) {
    DrawCircle(ART/2, ART/2 + 10, 88, (Color){ 220, 45, 40, 255 });
    DrawCircle(ART/2, ART/2 + 10, 82, (Color){ 235, 60, 50, 255 });
    DrawEllipse(ART/2 - 32, ART/2 - 22, 26, 16, (Color){ 255, 130, 110, 170 });
    for (int i = 0; i < 5; i++) {                        // calyx star
        float a = -PI/2 + i * (2*PI/5);
        Vector2 c = { ART/2.0f, ART/2.0f - 62 };
        Vector2 tip = { c.x + cosf(a)*34, c.y + sinf(a)*22 };
        DrawTriangle((Vector2){ c.x - 9, c.y }, (Vector2){ c.x + 9, c.y }, tip,
                     (Color){ 60, 140, 55, 255 });
    }
    DrawCircle(ART/2, ART/2 - 62, 10, (Color){ 50, 120, 48, 255 });
}
static void ArtBroccoli(void) {
    DrawRectangleRounded((Rectangle){ ART/2 - 20, 130, 40, 96 }, 0.6f, 8,
                         (Color){ 168, 205, 120, 255 });
    DrawRectangleRounded((Rectangle){ ART/2 - 8, 135, 12, 85 }, 0.6f, 8,
                         (Color){ 143, 180, 100, 255 });
    Vector2 f[9] = { {128,70},{85,92},{171,92},{62,124},{128,102},{194,124},
                     {95,130},{160,130},{128,138} };
    for (int i = 0; i < 9; i++)
        DrawCircleV(f[i], (i==0||i==4)?38:32, (Color){ 55, 125, 55, 255 });
    for (int i = 0; i < 60; i++) {                       // floret texture
        float a = frand(0, 2*PI), r = frand(0, 78);
        Vector2 p = { 128 + cosf(a)*r, 100 + sinf(a)*r*0.55f };
        DrawCircleV(p, frand(2, 5), (Color){ 78, 155, 72, 255 });
    }
}
static void ArtEggplant(void) {
    DrawEllipse(ART/2, ART/2 + 30, 62, 90, (Color){ 90, 40, 130, 255 });
    DrawEllipse(ART/2 - 6, ART/2, 52, 62, (Color){ 110, 55, 155, 255 });
    DrawEllipse(ART/2 - 26, ART/2 - 6, 16, 34, (Color){ 170, 120, 210, 150 });
    for (int i = 0; i < 4; i++) {                        // cap leaves
        float a = PI*0.25f + i * (PI*0.5f/3) + PI*0.12f;
        Vector2 c = { ART/2.0f, 62 };
        Vector2 tip = { c.x + cosf(a + PI/2)*46, c.y + sinf(a + PI/2)*30 + 14 };
        DrawTriangle((Vector2){ c.x - 14, c.y }, (Vector2){ c.x + 14, c.y }, tip,
                     (Color){ 72, 145, 60, 255 });
    }
    DrawCircle(ART/2, 58, 20, (Color){ 72, 145, 60, 255 });
    DrawRectangleRounded((Rectangle){ ART/2 - 6, 26, 12, 34 }, 0.8f, 6,
                         (Color){ 90, 120, 55, 255 });
}
static void ArtCorn(void) {
    DrawEllipse(ART/2, ART/2, 52, 100, (Color){ 245, 200, 60, 255 });
    for (int r = 0; r < 9; r++)                          // kernel grid
        for (int c = 0; c < 5; c++) {
            float y = 40 + r * 20.0f;
            float sq = sqrtf(fmaxf(0.0f, 1 - powf((y - ART/2)/100.0f, 2)));
            float x = ART/2 + (c - 2) * 21.0f * sq;
            if (sq > 0.25f)
                DrawCircleV((Vector2){ x, y }, 8.5f * (0.6f + 0.4f*sq),
                            (Color){ 255, 220, 100, 255 });
        }
    DrawEllipse(ART/2 - 22, ART/2 - 30, 12, 42, (Color){ 255, 240, 160, 120 });
    DrawTriangle((Vector2){ 100, 235 }, (Vector2){ 128, 150 }, (Vector2){ 55, 150 },
                 (Color){ 92, 168, 70, 255 });           // husk leaves
    DrawTriangle((Vector2){ 156, 235 }, (Vector2){ 201, 150 }, (Vector2){ 128, 150 },
                 (Color){ 70, 145, 58, 255 });
}
static void ArtPumpkin(void) {
    DrawEllipse(ART/2, ART/2 + 14, 96, 76, (Color){ 225, 115, 30, 255 });
    for (int i = -2; i <= 2; i++) {                      // lobes
        DrawEllipse(ART/2 + i * 34, ART/2 + 14, 26, 72,
                    (i % 2) ? (Color){ 240, 135, 45, 255 } : (Color){ 210, 100, 25, 255 });
    }
    DrawEllipse(ART/2 - 40, ART/2 - 12, 16, 30, (Color){ 255, 180, 110, 120 });
    DrawRectangleRounded((Rectangle){ ART/2 - 10, 36, 20, 40 }, 0.6f, 6,
                         (Color){ 110, 85, 45, 255 });
    DrawRectangleRounded((Rectangle){ ART/2 - 2, 30, 18, 14 }, 0.8f, 6,
                         (Color){ 95, 72, 38, 255 });
}
static void ArtOnion(void) {
    DrawCircle(ART/2, ART/2 + 8, 80, (Color){ 155, 80, 150, 255 });
    for (int i = 1; i <= 3; i++)                         // rings
        DrawRing((Vector2){ ART/2, ART/2 + 8 }, 78 - i*20, 82 - i*20, -70, 250, 40,
                 (Color){ 190, 115, 185, 200 });
    DrawEllipse(ART/2 - 30, ART/2 - 18, 18, 26, (Color){ 225, 170, 220, 150 });
    DrawTriangle((Vector2){ ART/2 - 14, ART/2 - 66 }, (Vector2){ ART/2 + 14, ART/2 - 66 },
                 (Vector2){ ART/2, ART/2 - 100 }, (Color){ 165, 95, 160, 255 });
    for (int i = -2; i <= 2; i++)                        // roots
        DrawLineEx((Vector2){ ART/2 + i*8.0f, ART/2 + 84 },
                   (Vector2){ ART/2 + i*14.0f, ART/2 + 104 }, 3,
                   (Color){ 210, 190, 160, 255 });
}
static void ArtCucumber(void) {
    DrawRectangleRounded((Rectangle){ ART/2 - 34, 30, 68, 196 }, 1.0f, 16,
                         (Color){ 60, 130, 50, 255 });
    for (int i = 0; i < 4; i++)                          // stripes
        DrawRectangleRounded((Rectangle){ ART/2 - 26 + i*16, 42, 7, 172 }, 1.0f, 6,
                             (Color){ 45, 105, 40, 255 });
    for (int i = 0; i < 26; i++)                         // speckles
        DrawCircleV((Vector2){ frand(ART/2.0f - 26, ART/2.0f + 26), frand(45, 210) },
                    frand(1.5f, 3), (Color){ 150, 200, 120, 200 });
    DrawEllipse(ART/2 - 18, 90, 8, 40, (Color){ 140, 200, 120, 110 });
}
static void ArtBomb(void) {
    DrawCircle(ART/2, ART/2 + 12, 80, (Color){ 32, 34, 40, 255 });
    DrawCircle(ART/2, ART/2 + 12, 72, (Color){ 48, 52, 62, 255 });
    DrawEllipse(ART/2 - 28, ART/2 - 14, 22, 15, (Color){ 120, 130, 150, 130 });
    DrawRing((Vector2){ ART/2, ART/2 + 12 }, 74, 80, 0, 360, 48,
             (Color){ 180, 40, 40, 255 });               // warning ring
    DrawRectangleRounded((Rectangle){ ART/2 - 12, 48, 24, 26 }, 0.4f, 6,
                         (Color){ 70, 74, 86, 255 });    // collar
    DrawLineEx((Vector2){ ART/2, 52 }, (Vector2){ ART/2 + 30, 22 }, 7,
               (Color){ 150, 120, 80, 255 });            // fuse
    DrawCircle(ART/2 + 32, 20, 8, (Color){ 255, 200, 60, 255 });
}
static void ArtSplat(void) {
    Vector2 c = { ART/2, ART/2 };
    DrawCircleV(c, 58, WHITE);
    for (int i = 0; i < 14; i++) {
        float a = frand(0, 2*PI), d = frand(30, 92);
        DrawCircleV((Vector2){ c.x + cosf(a)*d, c.y + sinf(a)*d }, frand(7, 26), WHITE);
    }
    for (int i = 0; i < 12; i++) {                       // droplets
        float a = frand(0, 2*PI), d = frand(85, 118);
        DrawCircleV((Vector2){ c.x + cosf(a)*d, c.y + sinf(a)*d }, frand(3, 7), WHITE);
    }
}

static RenderTexture2D BakeArt(void (*fn)(void)) {
    RenderTexture2D rt = LoadRenderTexture(ART, ART);
    BeginTextureMode(rt);
    ClearBackground(BLANK);
    fn();
    EndTextureMode();
    SetTextureFilter(rt.texture, TEXTURE_FILTER_BILINEAR);
    GenTextureMipmaps(&rt.texture);
    return rt;
}

static void InitArt(void) {
    void (*fns[VEG_COUNT])(void) = { ArtCarrot, ArtTomato, ArtBroccoli,
        ArtEggplant, ArtCorn, ArtPumpkin, ArtOnion, ArtCucumber };
    for (int i = 0; i < VEG_COUNT; i++) gVegTex[i] = BakeArt(fns[i]);
    gBombTex  = BakeArt(ArtBomb);
    gSplatTex = BakeArt(ArtSplat);

    VEG_JUICE[VEG_CARROT]   = (Color){ 240, 130, 35, 255 };
    VEG_JUICE[VEG_TOMATO]   = (Color){ 225, 55, 45, 255 };
    VEG_JUICE[VEG_BROCCOLI] = (Color){ 90, 160, 75, 255 };
    VEG_JUICE[VEG_EGGPLANT] = (Color){ 130, 70, 175, 255 };
    VEG_JUICE[VEG_CORN]     = (Color){ 250, 215, 85, 255 };
    VEG_JUICE[VEG_PUMPKIN]  = (Color){ 235, 130, 40, 255 };
    VEG_JUICE[VEG_ONION]    = (Color){ 195, 120, 190, 255 };
    VEG_JUICE[VEG_CUCUMBER] = (Color){ 120, 185, 90, 255 };
}

// ========================================================= procedural audio
static Sound MakeSound(short *buf, int frames) {
    Wave w = { .frameCount = (unsigned int)frames, .sampleRate = 44100,
               .sampleSize = 16, .channels = 1, .data = buf };
    Sound s = LoadSoundFromWave(w);
    free(buf);
    return s;
}
static short Clip(float v) {
    v = Clamp(v, -1.0f, 1.0f);
    return (short)(v * 32000);
}
static void InitSounds(void) {
    InitAudioDevice();
    gAudioOk = IsAudioDeviceReady();
    if (!gAudioOk) return;
    const int SR = 44100;

    int n = (int)(SR * 0.22f);                            // blade swoosh
    short *b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    float ph = 0;
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR, T = 0.22f;
        float env = fminf(t / 0.02f, 1.0f) * expf(-t * 14);
        float f = 900 - 700 * (t / T);
        ph += 2*PI*f/SR;
        float noise = frand(-1, 1);
        b[i] = Clip((noise * 0.55f + sinf(ph) * 0.35f) * env * 0.8f);
    }
    Sound slice = MakeSound(b, n);

    n = (int)(SR * 0.18f);                                // juicy splat
    b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR;
        float env = expf(-t * 26);
        b[i] = Clip((frand(-1,1) * 0.75f * env + sinf(2*PI*85*t) * 0.55f * expf(-t*16)));
    }
    Sound splat = MakeSound(b, n);

    n = (int)(SR * 0.8f);                                 // bomb explosion
    b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    float brown = 0;
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR;
        brown = (brown + frand(-1,1) * 0.22f) * 0.985f;
        float env = expf(-t * 4.5f);
        b[i] = Clip(brown * 2.6f * env + sinf(2*PI*44*t) * 0.7f * expf(-t*3.5f));
    }
    gSndBomb = MakeSound(b, n);

    n = (int)(SR * 0.34f);                                // combo arpeggio
    b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    float notes[3] = { 523.25f, 659.25f, 783.99f };
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR;
        int k = (int)(t / 0.11f); if (k > 2) k = 2;
        float lt = t - k * 0.11f;
        float env = fminf(lt / 0.01f, 1.0f) * expf(-lt * 12);
        b[i] = Clip(sinf(2*PI*notes[k]*t) * env * 0.5f);
    }
    gSndCombo = MakeSound(b, n);

    n = (int)(SR * 0.10f);                                // launch pop
    b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    ph = 0;
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR;
        ph += 2*PI*(250 + 4500*t)/SR;
        b[i] = Clip(sinf(ph) * expf(-t * 26) * 0.4f);
    }
    gSndLaunch = MakeSound(b, n);

    n = (int)(SR * 0.15f);                                // life-lost thud
    b = malloc(n * sizeof(short));
    if (!b) { gAudioOk = false; return; }
    for (int i = 0; i < n; i++) {
        float t = (float)i / SR;
        b[i] = Clip(sinf(2*PI*(120 - 60*t)*t) * expf(-t * 18) * 0.8f);
    }
    gSndThud = MakeSound(b, n);

    for (int i = 0; i < SND_VOICES; i++) {                // polyphony aliases
        gSndSlice[i] = LoadSoundAlias(slice);
        gSndSplat[i] = LoadSoundAlias(splat);
    }
}
static void PlaySlice(void) {
    if (!gAudioOk) return;
    Sound s = gSndSlice[gVoiceSlice = (gVoiceSlice + 1) % SND_VOICES];
    SetSoundPitch(s, frand(0.85f, 1.2f));
    PlaySound(s);
}
static void PlaySplat(void) {
    if (!gAudioOk) return;
    Sound s = gSndSplat[gVoiceSplat = (gVoiceSplat + 1) % SND_VOICES];
    SetSoundPitch(s, frand(0.8f, 1.25f));
    PlaySound(s);
}

// ============================================================ entity spawns
static void SpawnParticle(Vector2 pos, Vector2 vel, float life, float size,
                          Color c, bool additive, float drag) {
    for (int i = 0; i < MAX_PART; i++) if (!gPart[i].active) {
        gPart[i] = (Particle){ true, additive, pos, vel, life, life, size, drag, c };
        return;
    }
}
static void SpawnPopup(Vector2 pos, const char *txt, float size, Color c) {
    for (int i = 0; i < MAX_POPUP; i++) if (!gPopup[i].active) {
        gPopup[i].active = true;
        snprintf(gPopup[i].text, sizeof(gPopup[i].text), "%s", txt);
        gPopup[i].pos = pos; gPopup[i].life = 1.0f;
        gPopup[i].size = size; gPopup[i].color = c;
        return;
    }
}
static void SpawnSplat(Vector2 pos, Color c) {
    for (int i = 0; i < MAX_SPLAT; i++) if (!gSplat[i].active) {
        gSplat[i] = (Splat){ true, pos, frand(0, 360), frand(1.2f, 2.0f),
                             5.0f, 5.0f, c };
        return;
    }
}
static void LaunchOne(bool bomb) {
    if (!(gLevel->vegMask & VEGBIT_ALL)) return;   // never spin on an empty set
    for (int i = 0; i < MAX_VEG; i++) if (!gVeg[i].active) {
        VegType t;
        do { t = (VegType)GetRandomValue(0, VEG_COUNT - 1); }
        while (!(gLevel->vegMask & (1u << t)));
        if (!bomb) gSpawnedVeg++;
        float x = frand(GAME_W * 0.15f, GAME_W * 0.85f);
        float vx = (GAME_W/2 - x) * frand(0.15f, 0.55f) + frand(-60, 60);
        float vy = -frand(1050, 1380);
        gVeg[i] = (Veg){ true, bomb, t, { x, GAME_H + 80 }, { vx, vy },
                         frand(0, 360), frand(-160, 160),
                         bomb ? 58 : VEG_RADIUS[t] };
        if (gAudioOk) { SetSoundPitch(gSndLaunch, frand(0.9f, 1.15f)); PlaySound(gSndLaunch); }
        return;
    }
}
static void SpawnWave(void) {
    int count = 1 + GetRandomValue(0, gLevel->maxWave - 1);
    for (int i = 0; i < count; i++)
        LaunchOne(frand(0, 1) < gLevel->bombChance);
}

// ============================================================== game events
static void DecideOutcome(bool success) {
    if (gOutcome) return;                 // first decision wins
    gOutcome = success ? 1 : 2;
    gOutcomeTimer = 1.0f;
    if (success) {
        gAccuracy = gSpawnedVeg > 0 ? (float)gSliced / (float)gSpawnedVeg : 1.0f;
        float metric = (gLevel->starMetric == METRIC_SCORE) ? (float)gScore : gAccuracy;
        gStarsEarned = 1 + (metric >= gLevel->star2) + (metric >= gLevel->star3);
        Progress_CompleteLevel(gLevel->id, gStarsEarned);
        if (gAudioOk) PlaySound(gSndCombo);           // little fanfare
    }
}

static void LoseLife(void) {
    if (gOutcome) return;                 // outcome already decided
    gLives--;
    if (gAudioOk) PlaySound(gSndThud);
    if (gLives <= 0) DecideOutcome(false);
}

static void SliceVeg(int vi, Vector2 bladeDir) {
    Veg *v = &gVeg[vi];
    v->active = false;
    float a = atan2f(bladeDir.y, bladeDir.x);
    Vector2 perp = { -sinf(a), cosf(a) };
    Color juice = VEG_JUICE[v->type];

    for (int side = 0; side < 2; side++)                  // two halves
        for (int i = 0; i < MAX_HALF; i++) if (!gHalf[i].active) {
            float s = side ? 1.0f : -1.0f;
            gHalf[i] = (Half){ true, v->type, side,
                Vector2Add(v->pos, Vector2Scale(perp, s * 8)),
                { v->vel.x * 0.5f + perp.x * s * frand(160, 280) + bladeDir.x * 110,
                  v->vel.y * 0.5f + perp.y * s * frand(160, 280) + bladeDir.y * 110 - 120 },
                a * RAD2DEG + 90, s * frand(70, 220), 1.6f };
            break;
        }

    for (int i = 0; i < 46; i++) {                        // juice spray
        float pa = a + PI/2 * (GetRandomValue(0,1) ? 1 : -1) + frand(-0.9f, 0.9f);
        float sp = frand(120, 620);
        Color c = juice;
        c.r = (unsigned char)Clamp(c.r + GetRandomValue(-25, 25), 0, 255);
        c.g = (unsigned char)Clamp(c.g + GetRandomValue(-25, 25), 0, 255);
        SpawnParticle(v->pos, (Vector2){ cosf(pa)*sp + v->vel.x*0.3f,
                                         sinf(pa)*sp + v->vel.y*0.3f },
                      frand(0.5f, 1.1f), frand(3, 9), c, false, 1.5f);
    }
    for (int i = 0; i < 10; i++)                          // sparkle flecks
        SpawnParticle(v->pos, (Vector2){ frand(-360, 360), frand(-360, 360) },
                      frand(0.2f, 0.45f), frand(2, 4),
                      (Color){ 255, 255, 230, 255 }, true, 2.5f);

    SpawnSplat(v->pos, juice);
    PlaySplat(); PlaySlice();
    gShake = fmaxf(gShake, 0.12f);

    // results freeze the moment the outcome is decided: slicing in the
    // brief window before the outcome screen stays juicy but is cosmetic
    // (see .docflow/adr/0003-typed-finite-stage-objectives.md, AC 7)
    if (gOutcome) return;
    gScore += 10;
    SpawnPopup(v->pos, "+10", 34, (Color){ 255, 255, 255, 255 });
    gSliced++;
    gComboCount++;
    if (gComboCount > gMaxCombo) gMaxCombo = gComboCount;
    gComboTimer = 0.42f;
    gComboPos = v->pos;
}

static void ExplodeBomb(Vector2 pos) {
    if (gAudioOk) PlaySound(gSndBomb);
    gShake = 1.0f; gAberr = 1.0f; gFlash = 0.85f;
    for (int i = 0; i < 170; i++) {
        float a = frand(0, 2*PI), sp = frand(80, 950);
        Color c = (GetRandomValue(0, 2) == 0) ? (Color){ 255, 235, 140, 255 }
                                              : (Color){ 255, 130, 40, 255 };
        SpawnParticle(pos, (Vector2){ cosf(a)*sp, sinf(a)*sp },
                      frand(0.3f, 1.0f), frand(3, 10), c, true, 2.2f);
    }
    for (int i = 0; i < 60; i++) {                        // smoke
        float a = frand(0, 2*PI), sp = frand(30, 260);
        unsigned char g = (unsigned char)GetRandomValue(60, 110);
        SpawnParticle(pos, (Vector2){ cosf(a)*sp, sinf(a)*sp - 60 },
                      frand(0.8f, 1.6f), frand(10, 26),
                      (Color){ g, g, g, 200 }, false, 2.8f);
    }
    for (int i = 0; i < MAX_VEG; i++)                     // knock everything away
        if (gVeg[i].active && !gVeg[i].isBomb) {
            Vector2 d = Vector2Subtract(gVeg[i].pos, pos);
            float dist = fmaxf(Vector2Length(d), 40);
            float push = fminf(900.0f, 220000.0f / dist);
            gVeg[i].vel.x += d.x / dist * push;
            gVeg[i].vel.y += d.y / dist * push - 250;
            gVeg[i].rotVel += frand(-300, 300);
        }
    SpawnPopup(pos, "BOOM!", 64, (Color){ 255, 90, 60, 255 });
    LoseLife();
}

static void ClearStageEntities(void) {
    memset(gVeg, 0, sizeof(gVeg));   memset(gHalf, 0, sizeof(gHalf));
    memset(gPart, 0, sizeof(gPart)); memset(gSplat, 0, sizeof(gSplat));
    memset(gPopup, 0, sizeof(gPopup));
    gTrailLen = 0;
}

static void GoToMap(void) {
    ClearStageEntities();                 // nothing lingers behind the map
    gPaused = false;
    gState = ST_MAP; gStateTime = 0;
}

static void StartLevel(const LevelDef *lvl) {
    ClearStageEntities();
    gLevel = lvl;
    gScore = 0; gLives = lvl->lives; gSpawnTimer = 0.8f;
    gStageTime = 0; gSliced = 0; gSpawnedVeg = 0; gMaxCombo = 0;
    gOutcome = 0; gOutcomeTimer = 0; gStarsEarned = 0; gAccuracy = 0;
    gComboCount = 0; gComboTimer = 0;
    gShake = gAberr = gFlash = 0;
    gPaused = false;
    gState = ST_PLAY; gStateTime = 0;
}

// ================================================================== update
static void UpdatePlay(float dt) {
    gStageTime += dt;
    if (!gOutcome) {                                  // stop spawning once decided
        gSpawnTimer -= dt;
        float k = Clamp(gStageTime / gLevel->timeLimit, 0, 1);
        float interval = gLevel->spawnInterval * Lerp(1.0f, gLevel->spawnRamp, k);
        if (gSpawnTimer <= 0) { SpawnWave(); gSpawnTimer = interval * frand(0.8f, 1.2f); }
    }

    // ---- blade slicing
    // The speed gate divides by REAL frame time, not the physics-clamped dt:
    // across a frame hitch the clamped dt would inflate a slow movement's
    // measured speed ~(real/clamped)x and slice the whole hitch-spanning
    // segment (see .docflow/adr/0006-blade-cut-integrity.md, AC 4).
    Vector2 bladeVec = Vector2Subtract(gMouse, gMousePrev);
    float bladeSpeed = Vector2Length(bladeVec) / fmaxf(gRealDt, 1e-5f);
    bool cutting = gBladeDown && bladeSpeed > SLICE_SPEED;
    Vector2 bladeDir = cutting ? Vector2Normalize(bladeVec) : (Vector2){ 1, 0 };

    for (int i = 0; i < MAX_VEG; i++) {
        Veg *v = &gVeg[i];
        if (!v->active) continue;
        v->vel.y += GRAVITY * dt;
        v->pos = Vector2Add(v->pos, Vector2Scale(v->vel, dt));
        v->rot += v->rotVel * dt;

        if (v->isBomb) {                                  // fuse sparks
            // fuse tip in art canvas is (+32,-120) from bomb center = -75 deg,
            // 124 art px away; art->world scale is radius*2.55/ART
            float fa = (v->rot - 75.0f) * DEG2RAD;
            Vector2 tip = { v->pos.x + cosf(fa) * v->radius * 1.24f,
                            v->pos.y + sinf(fa) * v->radius * 1.24f };
            SpawnParticle(tip, (Vector2){ frand(-60, 60), frand(-80, 20) },
                          frand(0.15f, 0.35f), frand(2, 4),
                          (Color){ 255, 220, 90, 255 }, true, 1.0f);
        }
        if (cutting && PointSegDist(v->pos, gMousePrev, gMouse) < v->radius) {
            if (v->isBomb) {
                v->active = false;
                ExplodeBomb(v->pos);
                if (gLevel->objective == OBJ_NO_BOMB) DecideOutcome(false);
            } else SliceVeg(i, bladeDir);
            continue;
        }
        if (v->pos.y > GAME_H + 140 && v->vel.y > 0) {    // fell off screen
            v->active = false;
            if (!v->isBomb) {
                LoseLife();
                SpawnPopup((Vector2){ Clamp(v->pos.x, 60, GAME_W - 60), GAME_H - 60 },
                           "MISS", 40, (Color){ 255, 80, 80, 255 });
            }
        }
    }

    // ---- combo resolution
    if (gComboTimer > 0) {
        gComboTimer -= dt;
        if (gComboTimer <= 0) {
            if (gComboCount >= 3 && !gOutcome) {   // no bonuses after the freeze
                int bonus = gComboCount * 10;
                gScore += bonus;
                SpawnPopup(gComboPos, TextFormat("COMBO x%d  +%d", gComboCount, bonus),
                           52, (Color){ 255, 210, 60, 255 });
                if (gAudioOk) PlaySound(gSndCombo);
                gShake = fmaxf(gShake, 0.3f);
            }
            gComboCount = 0;
        }
    }

    // ---- objective evaluation (generic over the LevelDef)
    if (!gOutcome) {
        switch (gLevel->objective) {
        case OBJ_SLICE_COUNT:  if (gSliced   >= gLevel->objTarget) DecideOutcome(true); break;
        case OBJ_SCORE_TARGET: if (gScore    >= gLevel->objTarget) DecideOutcome(true); break;
        case OBJ_COMBO:        if (gMaxCombo >= gLevel->objTarget) DecideOutcome(true); break;
        case OBJ_SURVIVE_TIME:
        case OBJ_NO_BOMB:      break;     // decided by the clock below
        }
        if (!gOutcome && gStageTime >= gLevel->timeLimit) {
            bool timeIsGoal = (gLevel->objective == OBJ_SURVIVE_TIME ||
                               gLevel->objective == OBJ_NO_BOMB);
            DecideOutcome(timeIsGoal);
        }
    }
    if (gOutcome) {                                   // brief pause, then screen
        gOutcomeTimer -= dt;
        if (gOutcomeTimer <= 0) {
            gState = (gOutcome == 1) ? ST_COMPLETE : ST_FAILED;
            gStateTime = 0;
        }
    }
}

static void UpdateCommon(float dt) {
    for (int i = 0; i < MAX_HALF; i++) if (gHalf[i].active) {
        Half *h = &gHalf[i];
        h->vel.y += GRAVITY * dt;
        h->pos = Vector2Add(h->pos, Vector2Scale(h->vel, dt));
        h->rot += h->rotVel * dt;
        h->life -= dt;
        if (h->life <= 0 || h->pos.y > GAME_H + 200) h->active = false;
    }
    for (int i = 0; i < MAX_PART; i++) if (gPart[i].active) {
        Particle *p = &gPart[i];
        p->vel.y += GRAVITY * 0.55f * dt;
        p->vel = Vector2Scale(p->vel, 1.0f - p->drag * dt);
        p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, dt));
        p->life -= dt;
        if (p->life <= 0) p->active = false;
    }
    for (int i = 0; i < MAX_SPLAT; i++) if (gSplat[i].active) {
        gSplat[i].life -= dt;
        if (gSplat[i].life <= 0) gSplat[i].active = false;
    }
    for (int i = 0; i < MAX_POPUP; i++) if (gPopup[i].active) {
        gPopup[i].life -= dt * 0.8f;
        gPopup[i].pos.y -= 55 * dt;
        if (gPopup[i].life <= 0) gPopup[i].active = false;
    }
    for (int i = 0; i < MAX_MOTES; i++) {                 // ambient dust
        Particle *m = &gMotes[i];
        m->pos = Vector2Add(m->pos, Vector2Scale(m->vel, dt));
        if (m->pos.y < -10) { m->pos.y = GAME_H + 10; m->pos.x = frand(0, GAME_W); }
        if (m->pos.x < -10) m->pos.x = GAME_W + 10;
        if (m->pos.x > GAME_W + 10) m->pos.x = -10;
    }
    gShake = fmaxf(0, gShake - dt * 2.2f);
    gAberr = fmaxf(0, gAberr - dt * 1.6f);
    gFlash = fmaxf(0, gFlash - dt * 2.8f);

    // blade trail bookkeeping
    double now = GetTime();
    if (gBladeDown) {
        if (gTrailLen < MAX_TRAIL &&
            (gTrailLen == 0 || Vector2Distance(gTrail[gTrailLen-1].pos, gMouse) > 3)) {
            gTrail[gTrailLen++] = (TrailPt){ gMouse, now };
        }
    }
    int w = 0;                                            // expire old points
    for (int i = 0; i < gTrailLen; i++)
        if (now - gTrail[i].t < 0.16) gTrail[w++] = gTrail[i];
    gTrailLen = w;
}

// ==================================================================== draw
static void DrawVegSprite(VegType t, Vector2 pos, float rot, float radius, Color tint) {
    float sz = radius * 2.55f;
    DrawTexturePro(gVegTex[t].texture,
        (Rectangle){ 0, 0, ART, -ART },
        (Rectangle){ pos.x, pos.y, sz, sz },
        (Vector2){ sz/2, sz/2 }, rot, tint);
}
static void DrawHalf(Half *h) {
    float radius = VEG_RADIUS[h->type];
    float sz = radius * 2.55f;
    unsigned char a = (unsigned char)(255 * Clamp(h->life / 0.5f, 0, 1));
    Rectangle src = h->side == 0
        ? (Rectangle){ 0, 0, ART/2, -ART }
        : (Rectangle){ ART/2, 0, ART/2, -ART };
    Vector2 origin = h->side == 0 ? (Vector2){ sz/2, sz/2 } : (Vector2){ 0, sz/2 };
    DrawTexturePro(gVegTex[h->type].texture, src,
        (Rectangle){ h->pos.x, h->pos.y, sz/2, sz },
        origin, h->rot, (Color){ 255, 255, 255, a });
}
static void DrawTrail(void) {
    if (gTrailLen < 2) return;
    double now = GetTime();
    BeginBlendMode(BLEND_ADDITIVE);
    for (int pass = 0; pass < 2; pass++) {
        float baseW = pass == 0 ? 26.0f : 9.0f;
        Color col  = pass == 0 ? (Color){ 90, 220, 160, 90 }
                               : (Color){ 235, 255, 245, 230 };
        for (int i = 1; i < gTrailLen; i++) {
            float age0 = 1.0f - (float)((now - gTrail[i-1].t) / 0.16);
            float age1 = 1.0f - (float)((now - gTrail[i].t) / 0.16);
            float w0 = baseW * Clamp(age0, 0, 1);
            float w1 = baseW * Clamp(age1, 0, 1);
            DrawLineEx(gTrail[i-1].pos, gTrail[i].pos, fmaxf((w0+w1)*0.5f, 1), col);
            DrawCircleV(gTrail[i].pos, w1 * 0.5f, col);
        }
    }
    EndBlendMode();
}
static void DrawWorld(void) {
    // background (GPU generated)
    float res[2] = { GAME_W * RT_SCALE, GAME_H * RT_SCALE };
    float t = (float)GetTime();
    SetShaderValue(gBgShader, gLocBgTime, &t, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gBgShader, gLocBgRes, res, SHADER_UNIFORM_VEC2);
    BeginShaderMode(gBgShader);
    DrawRectangle(0, 0, GAME_W, GAME_H, WHITE);
    EndShaderMode();

    BeginBlendMode(BLEND_ADDITIVE);                       // dust motes
    for (int i = 0; i < MAX_MOTES; i++)
        DrawCircleV(gMotes[i].pos, gMotes[i].size,
                    (Color){ 160, 220, 190, (unsigned char)(gMotes[i].maxLife) });
    EndBlendMode();

    for (int i = 0; i < MAX_SPLAT; i++) if (gSplat[i].active) {
        Splat *s = &gSplat[i];
        float sz = ART * s->size * 0.55f;
        unsigned char a = (unsigned char)(70 * Clamp(s->life / s->maxLife, 0, 1));
        DrawTexturePro(gSplatTex.texture, (Rectangle){ 0, 0, ART, -ART },
            (Rectangle){ s->pos.x, s->pos.y, sz, sz },
            (Vector2){ sz/2, sz/2 }, s->rot, (Color){ s->color.r, s->color.g, s->color.b, a });
    }

    for (int i = 0; i < MAX_HALF; i++) if (gHalf[i].active) DrawHalf(&gHalf[i]);

    for (int i = 0; i < MAX_VEG; i++) if (gVeg[i].active) {
        Veg *v = &gVeg[i];
        if (v->isBomb) {
            float sz = v->radius * 2.55f;
            DrawTexturePro(gBombTex.texture, (Rectangle){ 0, 0, ART, -ART },
                (Rectangle){ v->pos.x, v->pos.y, sz, sz },
                (Vector2){ sz/2, sz/2 }, v->rot, WHITE);
        } else DrawVegSprite(v->type, v->pos, v->rot, v->radius, WHITE);
    }

    for (int i = 0; i < MAX_PART; i++) if (gPart[i].active) {
        Particle *p = &gPart[i];
        float k = p->life / p->maxLife;
        Color c = p->color; c.a = (unsigned char)(c.a * k);
        if (p->additive) BeginBlendMode(BLEND_ADDITIVE);
        DrawCircleV(p->pos, p->size * (0.5f + 0.5f*k), c);
        if (p->additive) EndBlendMode();
    }

    DrawTrail();
}
static void DrawHud(void) {
    DrawTextShadow(TextFormat("%d", gScore), 34, 18, 72, (Color){ 255, 240, 200, 255 });
    DrawTextShadow("SCORE", 36, 88, 24, (Color){ 190, 210, 190, 200 });
    for (int i = 0; i < gLevel->lives; i++) {             // tomato lives
        Vector2 p = { GAME_W - 60.0f - i * 66, 52 };
        Color tint = i < gLives ? WHITE : (Color){ 70, 70, 70, 160 };
        DrawVegSprite(VEG_TOMATO, p, 0, 24, tint);
    }
    if (gState == ST_PLAY) {                              // objective + clock
        int rem = (int)ceilf(fmaxf(0, gLevel->timeLimit - gStageTime));
        DrawTextCentered(TextFormat("%d-%d  %s", gLevel->world, gLevel->stage,
                         gLevel->name), GAME_W/2, 14, 24, (Color){ 180, 205, 185, 210 });
        const char *obj = "";
        switch (gLevel->objective) {
        case OBJ_SLICE_COUNT:  obj = TextFormat("SLICE  %d / %d", gSliced, gLevel->objTarget); break;
        case OBJ_SURVIVE_TIME: obj = "SURVIVE!"; break;
        case OBJ_SCORE_TARGET: obj = TextFormat("SCORE  %d / %d", gScore, gLevel->objTarget); break;
        case OBJ_NO_BOMB:      obj = "DON'T SLICE A BOMB!"; break;
        case OBJ_COMBO:        obj = TextFormat("LAND A COMBO x%d   best x%d",
                                                gLevel->objTarget, gMaxCombo); break;
        }
        DrawTextCentered(obj, GAME_W/2, 44, 38, (Color){ 255, 250, 235, 255 });
        Color clock = rem <= 5 ? (Color){ 255, 90, 70, 255 } : (Color){ 200, 220, 200, 230 };
        DrawTextCentered(TextFormat("%d:%02d", rem/60, rem%60), GAME_W/2, 92, 32, clock);
    }
    for (int i = 0; i < MAX_POPUP; i++) if (gPopup[i].active) {
        Popup *p = &gPopup[i];
        Color c = p->color;
        c.a = (unsigned char)(255 * Clamp(p->life * 2, 0, 1));
        Vector2 m = MeasureTextEx(gFont, p->text, p->size, 2);
        DrawTextShadow(p->text, p->pos.x - m.x/2, p->pos.y - m.y/2, p->size, c);
    }
    if (gPaused) {
        DrawRectangle(0, 0, GAME_W, GAME_H, (Color){ 0, 0, 0, 140 });
        DrawTextCentered("PAUSED", GAME_W/2, GAME_H/2 - 60, 90, WHITE);
        DrawTextCentered("press P to resume", GAME_W/2, GAME_H/2 + 40, 30,
                         (Color){ 200, 220, 200, 255 });
    }
}
static void DrawMenu(void) {
    float t = (float)GetTime();
    DrawVegSprite(VEG_CARROT, (Vector2){ GAME_W/2 - 430, 250 + sinf(t*1.3f)*16 },
                  -20 + sinf(t)*10, 80, WHITE);
    DrawVegSprite(VEG_EGGPLANT, (Vector2){ GAME_W/2 + 430, 260 + sinf(t*1.1f+2)*16 },
                  15 + sinf(t*0.8f)*10, 80, WHITE);
    DrawVegSprite(VEG_TOMATO, (Vector2){ GAME_W/2 - 300, 560 + sinf(t*1.6f+1)*12 },
                  sinf(t*0.9f)*14, 52, WHITE);
    DrawVegSprite(VEG_BROCCOLI, (Vector2){ GAME_W/2 + 300, 570 + sinf(t*1.4f+3)*12 },
                  sinf(t*1.1f+2)*14, 56, WHITE);

    DrawTextCentered("VEGETABLE", GAME_W/2, 130, 130, (Color){ 170, 230, 120, 255 });
    DrawTextCentered("NINJA", GAME_W/2, 250, 170, (Color){ 255, 245, 225, 255 });

    float pulse = 0.75f + 0.25f * sinf(t * 3.5f);
    DrawTextCentered("hold LEFT MOUSE and swipe to slice!", GAME_W/2, 500, 40,
                     (Color){ 255, 255, 255, (unsigned char)(255*pulse) });
    DrawTextCentered("don't slice the bombs...", GAME_W/2, 555, 28,
                     (Color){ 255, 120, 100, 220 });
    DrawTextCentered("CLICK or press SPACE to play", GAME_W/2, 660, 36,
                     (Color){ 255, 210, 90, 255 });
#if defined(PLATFORM_WEB)
    DrawTextCentered("P pause   F fps", GAME_W/2, 840, 22,
                     (Color){ 150, 170, 155, 180 });
#else
    DrawTextCentered("F11 fullscreen   P pause   F fps   ESC quit", GAME_W/2, 840, 22,
                     (Color){ 150, 170, 155, 180 });
#endif
}
// ui helpers ----------------------------------------------------------------
static void DrawStarShape(Vector2 c, float r, Color col) {
    Vector2 pts[12];
    pts[0] = c;
    for (int i = 0; i <= 10; i++) {
        float a = -PI/2 - i * PI/5;      // counter-clockwise so quads face front
        float rr = (i % 2 == 0) ? r : r * 0.45f;
        pts[i + 1] = (Vector2){ c.x + cosf(a)*rr, c.y + sinf(a)*rr };
    }
    DrawTriangleFan(pts, 12, col);
}
static void DrawStarRow(Vector2 c, float r, float gap, int earned, float t) {
    for (int i = 0; i < 3; i++) {
        Vector2 p = { c.x + (i - 1) * gap, c.y };
        DrawStarShape((Vector2){ p.x + r*0.06f, p.y + r*0.08f }, r,
                      (Color){ 0, 0, 0, 90 });                       // shadow
        if (i < earned) {
            float pop = Clamp((t - 0.25f - i * 0.3f) * 4.0f, 0, 1);  // pop-in
            if (pop > 0)
                DrawStarShape(p, r * (0.6f + 0.4f * pop), (Color){ 255, 200, 60, 255 });
            else
                DrawStarShape(p, r, (Color){ 55, 65, 58, 255 });
        } else DrawStarShape(p, r, (Color){ 55, 65, 58, 255 });
    }
}
static Rectangle BtnRect(float cx) { return (Rectangle){ cx - 115, 640, 230, 74 }; }
static void DrawButton(Rectangle r, const char *label, bool enabled) {
    bool hover = enabled && CheckCollisionPointRec(gMouse, r);
    Color fill = !enabled ? (Color){ 38, 48, 44, 200 }
               : hover    ? (Color){ 96, 175, 80, 255 }
                          : (Color){ 56, 110, 60, 255 };
    DrawRectangleRounded(r, 0.35f, 8, (Color){ 10, 24, 18, 200 });
    DrawRectangleRounded((Rectangle){ r.x + 3, r.y + 3, r.width - 6, r.height - 6 },
                         0.35f, 8, fill);
    Vector2 m = MeasureTextEx(gFont, label, 36, 2);
    DrawTextShadow(label, r.x + (r.width - m.x)/2, r.y + (r.height - m.y)/2, 36,
                   enabled ? WHITE : (Color){ 130, 140, 132, 200 });
}

// world map ---------------------------------------------------------------
static Vector2 MapNodePos(int index) {
    const LevelDef *l = Levels_ByIndex(index);
    int row = (l->stage - 1) / 5, col = (l->stage - 1) % 5;
    return (Vector2){ GAME_W/2.0f - 2*170 + col*170,
                      (l->world == 1 ? 250.0f : 600.0f) + row*145 };
}
static void DrawMap(void) {
    DrawTextCentered("CHOOSE YOUR STAGE", GAME_W/2, 40, 60, (Color){ 255, 245, 225, 255 });
    for (int w = 1; w <= 2; w++) {
        bool anyUnlocked = false;
        for (int i = 0; i < Levels_Count(); i++)
            if (Levels_ByIndex(i)->world == w && Progress_IsUnlocked(Levels_ByIndex(i)->id))
                anyUnlocked = true;
        Color wc = anyUnlocked ? (Color){ 170, 230, 120, 255 } : (Color){ 110, 125, 115, 180 };
        DrawTextCentered(TextFormat("WORLD %d", w), GAME_W/2, w == 1 ? 140 : 495, 40, wc);
    }
    for (int i = 0; i < Levels_Count(); i++) {
        const LevelDef *l = Levels_ByIndex(i);
        Vector2 p = MapNodePos(i);
        bool unlocked = Progress_IsUnlocked(l->id);
        bool boss = (l->stage == 10);
        bool hover = unlocked && Vector2Distance(gMouse, p) < 54;
        float r = (boss ? 56 : 48) + (hover ? 5 : 0);
        Color fill = unlocked ? (hover ? (Color){ 96, 175, 80, 255 } : (Color){ 70, 140, 62, 255 })
                              : (Color){ 42, 52, 48, 255 };
        DrawCircleV(p, r + 5, (Color){ 12, 26, 20, 200 });
        DrawCircleV(p, r, fill);
        if (boss) DrawRing(p, r + 5, r + 10, 0, 360, 40,
                           unlocked ? (Color){ 255, 190, 60, 255 } : (Color){ 90, 80, 55, 255 });
        if (unlocked) {
            DrawTextCentered(TextFormat("%d", l->stage), p.x, p.y - 26, 46, WHITE);
        } else {                                       // padlock
            DrawRectangleRounded((Rectangle){ p.x - 15, p.y - 6, 30, 24 }, 0.3f, 6,
                                 (Color){ 105, 115, 108, 255 });
            DrawRing((Vector2){ p.x, p.y - 8 }, 7, 11, 180, 360, 16,
                     (Color){ 105, 115, 108, 255 });
        }
        if (boss) DrawTextCentered("BOSS", p.x, p.y - r - 32, 20,
                                   (Color){ 255, 190, 60, unlocked ? (unsigned char)255 : (unsigned char)120 });
        int best = Progress_BestStars(l->id);
        if (best > 0)                                    // best stars, monotonic
            for (int s = 0; s < 3; s++)
                DrawStarShape((Vector2){ p.x + (s - 1) * 26.0f, p.y + r + 20 }, 11,
                              s < best ? (Color){ 255, 200, 60, 255 }
                                       : (Color){ 50, 60, 54, 255 });
        if (hover)
            DrawTextCentered(l->name, GAME_W/2, 862, 30, (Color){ 220, 240, 220, 255 });
    }
}

static const LevelDef *NextLevel(void) {
    return Levels_ByIndex(Levels_IndexOfId(gLevel->id) + 1);
}
static void DrawComplete(void) {
    DrawRectangle(0, 0, GAME_W, GAME_H, (Color){ 0, 25, 10, 150 });
    DrawTextCentered(TextFormat("%d-%d  %s", gLevel->world, gLevel->stage, gLevel->name),
                     GAME_W/2, 110, 28, (Color){ 190, 215, 195, 220 });
    DrawTextCentered("STAGE CLEAR!", GAME_W/2, 150, 100, (Color){ 170, 230, 120, 255 });
    DrawStarRow((Vector2){ GAME_W/2, 350 }, 56, 140, gStarsEarned, gStateTime);
    DrawTextCentered(TextFormat("score  %d", gScore), GAME_W/2, 445, 56, WHITE);
    DrawTextCentered(TextFormat("accuracy  %d%%", (int)(gAccuracy * 100)),
                     GAME_W/2, 525, 30, (Color){ 200, 220, 200, 220 });
    const LevelDef *next = NextLevel();
    if (next) {
        DrawButton(BtnRect(GAME_W/2 - 300), "RETRY", true);
        DrawButton(BtnRect(GAME_W/2),       "NEXT",  Progress_IsUnlocked(next->id));
        DrawButton(BtnRect(GAME_W/2 + 300), "MAP",   true);
    } else {
        DrawButton(BtnRect(GAME_W/2 - 160), "RETRY", true);
        DrawButton(BtnRect(GAME_W/2 + 160), "MAP",   true);
        DrawTextCentered("all stages complete - what a harvest!", GAME_W/2, 760, 30,
                         (Color){ 255, 210, 90, 255 });
    }
}
static void DrawFailed(void) {
    DrawRectangle(0, 0, GAME_W, GAME_H, (Color){ 25, 0, 0, 150 });
    DrawTextCentered(TextFormat("%d-%d  %s", gLevel->world, gLevel->stage, gLevel->name),
                     GAME_W/2, 130, 28, (Color){ 215, 195, 190, 220 });
    DrawTextCentered("STAGE FAILED", GAME_W/2, 180, 100, (Color){ 255, 90, 70, 255 });
    DrawTextCentered(TextFormat("score  %d", gScore), GAME_W/2, 420, 56, WHITE);
    DrawButton(BtnRect(GAME_W/2 - 160), "RETRY", true);
    DrawButton(BtnRect(GAME_W/2 + 160), "MAP", true);
}

// =============================================================== frame loop
static void UpdateDrawFrame(void) {
    gRealDt = GetFrameTime();
    float dt = fminf(gRealDt, 1.0f/20.0f);   // clamped for physics stability
    gStateTime += dt;

    // ---- letterboxed presentation rect + virtual mouse
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    float scale = fminf(sw / GAME_W, sh / GAME_H);
    if (scale <= 0) scale = 1;   // zero-sized (minimized) window: keep the
                                 // mouse mapping finite until it restores
    Rectangle dst = { (sw - GAME_W*scale)/2, (sh - GAME_H*scale)/2,
                      GAME_W*scale, GAME_H*scale };
    gMousePrev = gMouse;
    gPointerPressed = false;
    if (gSelfTest) {                                  // scripted blade sweep
        gSelfFrame++;
        float ft = gSelfFrame / 60.0f;
        gMouse = (Vector2){ GAME_W/2 + sinf(ft*4.0f) * 600,
                            GAME_H/2 + 60 + sinf(ft*9.0f) * 260 };
        gBladeDown = true;
    } else {
#if defined(PLATFORM_WEB)
        // Pointer state comes from the JS handlers in shell.html: raylib's
        // web backend reports mouse in a stale coordinate system after its
        // canvas auto-resize. CSS px == backbuffer px here (see shell CSS),
        // and this also gives us touch input on mobile.
        Vector2 mp = { (float)EM_ASM_DOUBLE({ return Module.pointer.x; }),
                       (float)EM_ASM_DOUBLE({ return Module.pointer.y; }) };
        bool down = EM_ASM_INT({ return Module.pointer.down; }) != 0;
#else
        Vector2 mp = GetMousePosition();
        bool down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
#endif
        gMouse = (Vector2){ (mp.x - dst.x) / scale, (mp.y - dst.y) / scale };
        gPointerPressed = down && !gBladeDown;
        // a fresh press must not slice along the jump from the last release
        if (gPointerPressed) gMousePrev = gMouse;
        gBladeDown = down;
    }
#if !defined(PLATFORM_WEB)
    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();
#endif
    if (IsKeyPressed(KEY_F))   gShowFps = !gShowFps;

    // ---- state logic
    switch (gState) {
    case ST_MENU:
        if (gPointerPressed || IsKeyPressed(KEY_SPACE)) {
            gState = ST_MAP; gStateTime = 0;
        }
#if !defined(PLATFORM_WEB)
        if (IsKeyPressed(KEY_ESCAPE)) gQuit = true;
#endif
        break;
    case ST_MAP:
        if (IsKeyPressed(KEY_ESCAPE)) { gState = ST_MENU; gStateTime = 0; break; }
        if (gPointerPressed)
            for (int i = 0; i < Levels_Count(); i++) {
                const LevelDef *l = Levels_ByIndex(i);
                if (Progress_IsUnlocked(l->id) &&
                    Vector2Distance(gMouse, MapNodePos(i)) < 56) {
                    StartLevel(l);
                    break;
                }
            }
        break;
    case ST_PLAY:
        if (IsKeyPressed(KEY_P)) gPaused = !gPaused;
        if (IsKeyPressed(KEY_ESCAPE)) { GoToMap(); break; }
        if (!gPaused) UpdatePlay(dt);
        break;
    case ST_COMPLETE:
        if (gStateTime > 0.4f && gPointerPressed) {
            const LevelDef *next = NextLevel();
            if (next) {
                if      (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 - 300)))
                    StartLevel(gLevel);
                else if (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2)) &&
                         Progress_IsUnlocked(next->id))
                    StartLevel(next);
                else if (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 + 300)))
                    GoToMap();
            } else {
                if      (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 - 160)))
                    StartLevel(gLevel);
                else if (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 + 160)))
                    GoToMap();
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) GoToMap();
        break;
    case ST_FAILED:
        if (gStateTime > 0.4f && gPointerPressed) {
            if      (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 - 160)))
                StartLevel(gLevel);
            else if (CheckCollisionPointRec(gMouse, BtnRect(GAME_W/2 + 160)))
                GoToMap();
        }
        if (IsKeyPressed(KEY_ESCAPE)) GoToMap();
        break;
    }
    if (!gPaused) UpdateCommon(dt);

    // ---- render world into supersampled target
    BeginTextureMode(gTarget);
    ClearBackground(BLACK);
    Camera2D cam = { 0 };
    cam.zoom = RT_SCALE;
    cam.offset = (Vector2){ frand(-1, 1) * gShake * 18 * RT_SCALE,
                            frand(-1, 1) * gShake * 18 * RT_SCALE };
    BeginMode2D(cam);
    DrawWorld();
    switch (gState) {
    case ST_MENU:     DrawMenu(); break;
    case ST_MAP:      DrawMap();  break;
    case ST_PLAY:     DrawHud();  break;
    case ST_COMPLETE: DrawHud(); DrawComplete(); break;
    case ST_FAILED:   DrawHud(); DrawFailed();   break;
    }
    EndMode2D();
    EndTextureMode();

    // ---- composite through post shader
    BeginDrawing();
    ClearBackground(BLACK);
    SetShaderValue(gPostShader, gLocAberr, &gAberr, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gPostShader, gLocFlash, &gFlash, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(gPostShader);
    DrawTexturePro(gTarget.texture,
        (Rectangle){ 0, 0, (float)gTarget.texture.width, -(float)gTarget.texture.height },
        dst, (Vector2){ 0, 0 }, 0, WHITE);
    EndShaderMode();
    if (gShowFps) {
        DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 20, GREEN);
        // debug: ring drawn at the game's mapped mouse position — must
        // sit exactly on the OS cursor, at any window size
        Vector2 sp = { gMouse.x * scale + dst.x, gMouse.y * scale + dst.y };
        DrawCircleLinesV(sp, 12, RED);
        DrawCircleV(sp, 3, RED);
    }
    EndDrawing();

    if (gSelfTest) {
        if (gSelfFrame == 50) TakeScreenshot("selftest_menu.png");
        if (gSelfFrame == 52) { gState = ST_MAP; gStateTime = 0; }
        if (gSelfFrame == 58) TakeScreenshot("selftest_map.png");
        if (gSelfFrame == 60) StartLevel(Levels_ByIndex(0));
        if (gSelfFrame == 100 || gSelfFrame == 150) SpawnWave();
        if (gSelfFrame == 190) TakeScreenshot("selftest.png");
        if (gSelfFrame == 200 && gState == ST_PLAY) {
            DecideOutcome(true);
            gOutcomeTimer = 0.02f;   // self-test runs uncapped; skip the pause
        }
        if (gSelfFrame == 380) TakeScreenshot("selftest_complete.png");
        if (gSelfFrame >= 390) gQuit = true;
    }
}

// ===================================================================== main
int main(int argc, char **argv) {
    gSelfTest = (argc > 1 && strcmp(argv[1], "--selftest") == 0);

    {   // fail fast on a bad level row: a degenerate entry must never
        // reach the spawn loop (it would hang or misbehave mid-run)
        int bad = Levels_FirstInvalid();
        if (bad >= 0) {
            TraceLog(LOG_FATAL, "level table row %d (id %d) is invalid",
                     bad, Levels_ByIndex(bad)->id);
            return 1;   // TraceLog(LOG_FATAL) already exits; belt-and-braces
        }
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(GAME_W, GAME_H, "Vegetable Ninja");
    SetExitKey(KEY_NULL);
    SetTargetFPS(240);

    gFont = LoadFontFromMemory(".ttf", FONT_TTF, FONT_TTF_LEN, 160, NULL, 0);
    SetTextureFilter(gFont.texture, TEXTURE_FILTER_BILINEAR);

    InitArt();
    InitSounds();

    gTarget = LoadRenderTexture(GAME_W * RT_SCALE, GAME_H * RT_SCALE);
    SetTextureFilter(gTarget.texture, TEXTURE_FILTER_BILINEAR);
    gPostShader = LoadShaderFromMemory(NULL, POST_FS);
    gLocAberr = GetShaderLocation(gPostShader, "aberr");
    gLocFlash = GetShaderLocation(gPostShader, "flash");
    gBgShader = LoadShaderFromMemory(NULL, BG_FS);
    gLocBgTime = GetShaderLocation(gBgShader, "time");
    gLocBgRes  = GetShaderLocation(gBgShader, "resolution");

    {   // window icon from the tomato art
        Image icon = LoadImageFromTexture(gVegTex[VEG_TOMATO].texture);
        ImageFlipVertical(&icon);
        ImageResize(&icon, 64, 64);
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
#if defined(PLATFORM_WEB)
    // open index.html?debug to show the fps counter + mouse-mapping marker
    gShowFps = EM_ASM_INT({ return location.search.indexOf('debug') >= 0 ? 1 : 0; });
#endif

    for (int i = 0; i < MAX_MOTES; i++)
        gMotes[i] = (Particle){ true, true,
            { frand(0, GAME_W), frand(0, GAME_H) },
            { frand(-14, 14), frand(-30, -8) },
            1, frand(8, 30),                              // maxLife reused as alpha
            frand(1.2f, 3.2f), 0, WHITE };

    if (gSelfTest) gState = ST_MENU;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose() && !gQuit) UpdateDrawFrame();
#endif
    CloseWindow();
    return 0;
}
