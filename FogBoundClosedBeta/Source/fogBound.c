/*
 * =============================================================================
 *                              Custom License
 * =============================================================================
 *
 * Copyright (c) 2024 Lk10
 *
 * All rights reserved.
 *
 * Permission is hereby granted to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to use the Software solely for personal or educational purposes,
 * subject to the following conditions:
 *
 * 1. Ownership and Attribution:
 *    All content, code, and functionality processed by this game are owned by Lk10.
 *    Any use, reproduction, or distribution of the Software or its derivatives must
 *    include proper attribution to Lk10. This includes, but is not limited to,
 *    crediting Lk10 in any documentation, about sections, promotional materials,
 *    as well as in any videos and presentations related to the game.
 *    Specifically, the owner's GitHub profile (https://github.com/GuestAUser)
 *    must be prominently mentioned in such media.
 *
 * 2. Restrictions:
 *    Users are not permitted to copy, modify, merge, publish, distribute,
 *    sublicense, and/or sell copies of the Software, or any portions thereof,
 *    without explicit written permission from Lk10.
 *
 * 3. Contact Information:
 *    All rights are reserved to Lk10's GitHub profile.
 *    Methods of contact are available there for inquiries and permissions.
 *    GitHub Profile: https://github.com/GuestAUser
 *
 * 4. Liability:
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM,
 *    DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR
 *    OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR
 *    THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * =============================================================================
 *                           Image Assets Information
 * =============================================================================
 *
 * All image assets used in this game are currently free to use.
 * They are sourced from reputable free asset libraries and are licensed
 * for both personal and commercial use without the need for attribution.
 * However, it's always good practice to verify the licensing terms of each
 * asset to ensure compliance.
 *
 * =============================================================================
 */


#include <stddef.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"

#define MAX_RPS 550
#define MAX_CLDS 40
#define CLOUD_FRMS 6
#define MX_FG_PRTCLS 110

typedef struct Rdp {
    Vector2 position;
    float spd;
} Rdp;

typedef struct Cloud {
    Vector2 position;
    float s;
    bool isForeground;
    int frameIndex;
} Cloud;

typedef struct FogParticle {
    Vector2 position;
    float spD;
    float size;
    float alpha;
    float offset; 
} FogParticle;

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_ENDING,
    GAME_STATE_EXIT
} GameState;

Rdp raindrops[MAX_RPS];
Cloud clds[MAX_CLDS];
FogParticle fgParticle[MX_FG_PRTCLS];
Rectangle cRects[CLOUD_FRMS];

Texture2D pW;
Texture2D cTx;
Texture2D hTx;
Texture2D spotlightTexture;

Camera2D cM = {0};
Vector2 pPos = {400, 300};
float rSpd = 300.0f;
Color fColor = {200, 200, 200, 150};

GameState cStateX = GAME_STATE_MENU;

const int sWidth = 1280;
const int sHeight = 800;

const float wWidth = sWidth * 2.4;
const float wHeight = sHeight * 2.4;

Vector2 hPos;

float cW;
float cH;

void InitRn();
void UpdateRn();
void DrawRn();
void InitCd();
void UpdTCd();
void DrawCd(Cloud cloud);
void InitFP();
void UpdateFP();
void DrawFP();
void UpGame();
void DrawGame();
void UpdateMenu();
void DMn();
void UpEnd();
void DwE();
void CreateSpotlightTexture();
void UnldRsrcs();

Shader spotlightShader;

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(sWidth, sHeight, "FogBound: Where the Rain Remains");

    SearchAndSetResourceDir("resources");

    pW = LoadTexture("wabbit_alpha.png");
    cTx = LoadTexture("clouds.png");
    hTx = LoadTexture("house.png");

    CreateSpotlightTexture();

    int cCols = 2;
    int cRows = 3;
    cW = cTx.width / cCols;
    cH = cTx.height / cRows;

    int frameIndex = 0;
    for (int y = 0; y < cRows; y++) {
        for (int x = 0; x < cCols; x++) {
            if (frameIndex < CLOUD_FRMS) {
                cRects[frameIndex] = (Rectangle){
                    x * cW, y * cH,
                    cW, cH };
                frameIndex++;
            }
        }
    }

    InitRn();
    InitCd();
    InitFP();

    cM.offset = (Vector2){ sWidth / 2.0f, screenHeight / 2.0f };
    cM.target = pPos;
    cM.rotation = 0.0f;
    cM.zoom = 1.0f;

    float mDist_fp = 2500.0f; 
    do {
        hPos = (Vector2){
            GetRandomValue(100, (int)wWidth - 100),
            GetRandomValue(100, (int)wHeight - 100)
        };
    } while (Vector2Distance(hPos, pPos) < mDist_fp);

    SetTargetFPS(60);

    while (!WindowShouldClose() && cStateX != GAME_STATE_EXIT)
    {
        switch (cStateX) {
            case GAME_STATE_MENU:
                UpdateMenu();
                break;
            case GAME_STATE_PLAYING:
                UpGame();
                break;
            case GAME_STATE_ENDING:
                UpEnd();
                break;
            default:
                break;
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        switch (cStateX) {
            case GAME_STATE_MENU:
                DMn();
                break;
            case GAME_STATE_PLAYING:
                DrawGame();
                break;
            case GAME_STATE_ENDING:
                DwE();
                break;
            default:
                break;
        }

        EndDrawing();
    }

    UnldRsrcs();
    CloseWindow();
    return 0;
}

void InitRn() {
    for (int i = 0; i < MAX_RPS; i++) {
        raindrops[i].position = (Vector2){
            GetRandomValue(0, (int)wWidth),
            GetRandomValue(-200, (int)wHeight) };
        raindrops[i].spd = GetRandomValue(200, 500) / 100.0f;
    }
}

void UpdateRn() {
    float dTm = GetFrameTime();
    for (int i = 0; i < MAX_RPS; i++) {
        raindrops[i].position.y += raindrops[i].spd * dTm * 60.0f;
        if (raindrops[i].position.y > wHeight) {
            raindrops[i].position = (Vector2){
                GetRandomValue(0, (int)wWidth),
                GetRandomValue(-200, 0) };
            raindrops[i].spd = GetRandomValue(200, 500) / 100.0f;
        }
    }
}

void DrawRn() {
    for (int i = 0; i < MAX_RPS; i++) {
        DrawLineEx(raindrops[i].position,
                   (Vector2){ raindrops[i].position.x,
                              raindrops[i].position.y + 10 },
                   1.0f, LIGHTGRAY);
    }
}

void InitCd() {
    for (int i = 0; i < MAX_CLDS; i++) {
        Vector2 pos;
        float distance;
        int attempts = 0;
        do {
            pos.x = GetRandomValue(0, (int)wWidth);
            pos.y = GetRandomValue(0, (int)wHeight -
                                   (int)(cH * 0.25f));
            distance = Vector2Distance(pos, pPos);
            attempts++;
        } while (distance < 200.0f && attempts < 10);
        clds[i].position = pos;
        clds[i].s = GetRandomValue(80, 120) / 100.0f;
        clds[i].isForeground = GetRandomValue(0, 1);
        clds[i].frameIndex = GetRandomValue(0, CLOUD_FRMS - 1);
    }
}

void UpdTCd() {
    float dTm = GetFrameTime();
    for (int i = 0; i < MAX_CLDS; i++) {
        clds[i].position.x -= clds[i].s * dTm * 60.0f;
        if (clds[i].position.x < -cRects[clds[i].frameIndex].width *
            0.25f) {
            clds[i].position.x = wWidth + GetRandomValue(0, 200);
            int attempts = 0;
            float distance;
            do {
                clds[i].position.y = GetRandomValue(0,
                    (int)wHeight - (int)(cH * 0.25f));
                distance = Vector2Distance(clds[i].position,
                                           pPos);
                attempts++;
            } while (distance < 200.0f && attempts < 10);
            clds[i].s = GetRandomValue(80, 120) / 100.0f;
            clds[i].isForeground = GetRandomValue(0, 1);
            clds[i].frameIndex = GetRandomValue(0, CLOUD_FRMS - 1);
        }
    }
}

void DrawCd(Cloud cloud) {
    float d = Vector2Distance(cloud.position, pPos);
    float mD = 600.0f;

    float lumni0Fac = 1.0f - (d / mD);
    lumni0Fac = Clamp(lumni0Fac, 0.0f, 1.0f);
    lumni0Fac = powf(lumni0Fac, 1.5f);

    Color blendedFogColor = fColor;
    blendedFogColor.a = (unsigned char)(fColor.a * lumni0Fac);

    Rectangle frame = cRects[cloud.frameIndex];
    float scale = 0.25f;
    DrawTexturePro(cTx, frame,
        (Rectangle){ cloud.position.x, cloud.position.y,
                     frame.width * scale, frame.height * scale },
        (Vector2){ 0, 0 }, 0.0f, blendedFogColor);
}

void InitFP() {
    for (int i = 0; i < MX_FG_PRTCLS; i++) {
        fgParticle[i].position = (Vector2){
            GetRandomValue(0, (int)wWidth),
            GetRandomValue(0, (int)wHeight) };
        fgParticle[i].spD = GetRandomValue(10, 30) / 10.0f;
        fgParticle[i].size = GetRandomValue(3, 10); 
        fgParticle[i].alpha = GetRandomValue(50, 100) / 100.0f;
        fgParticle[i].offset = GetRandomValue(0, 360); 
    }
}

void UpdateFP() {
    float dTm = GetFrameTime();
    for (int i = 0; i < MX_FG_PRTCLS; i++) {
        fgParticle[i].position.x -= fgParticle[i].spD * dTm * 60.0f;
        fgParticle[i].position.y += sinf((GetTime() * fgParticle[i].spD) + fgParticle[i].offset) * dTm * 20.0f; 

        if (fgParticle[i].position.x < -50) {
            fgParticle[i].position.x = wWidth + GetRandomValue(0, 200);
            fgParticle[i].position.y = GetRandomValue(0, (int)wHeight);
            fgParticle[i].spD = GetRandomValue(10, 30) / 10.0f;
            fgParticle[i].size = GetRandomValue(3, 10);
            fgParticle[i].alpha = GetRandomValue(50, 100) / 100.0f;
            fgParticle[i].offset = GetRandomValue(0, 360);
        }
    }
}

void DrawFP() {
    for (int i = 0; i < MX_FG_PRTCLS; i++) {
        float distance = Vector2Distance(fgParticle[i].position,
                                         pPos);
        float maxDistance = 500.0f;
        float alphaFactor = 1.0f - (distance / maxDistance);
        alphaFactor = Clamp(alphaFactor, 0.0f, 1.0f);
        alphaFactor *= fgParticle[i].alpha;

        Color color = WHITE;
        color.a = (unsigned char)(255 * alphaFactor * 0.5f);

        DrawCircleV(fgParticle[i].position, fgParticle[i].size, color);
    }
}

void CreateSpotlightTexture() {
    int spotlightRadius = 300;
    Image spotlightImage = GenImageGradientRadial(spotlightRadius * 2, spotlightRadius * 2, 0.0f, WHITE, BLANK);
    spotlightTexture = LoadTextureFromImage(spotlightImage);
    UnloadImage(spotlightImage);
}

void UpGame() {
    float dTm = GetFrameTime();
    UpdateRn();
    UpdTCd();
    UpdateFP();

    Vector2 mvt = { 0 };

    if (IsKeyDown(KEY_RIGHT)) mvt.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)) mvt.x -= 1.0f;
    if (IsKeyDown(KEY_UP)) mvt.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) mvt.y += 1.0f;

    if (mvt.x != 0 || mvt.y != 0) {
        mvt = Vector2Normalize(mvt);
        pPos.x += mvt.x * rSpd * dTm;
        pPos.y += mvt.y * rSpd * dTm;
    }

    pPos.x = Clamp(pPos.x, 0, wWidth);
    pPos.y = Clamp(pPos.y, 0, wHeight);

    float distanceToHouse = Vector2Distance(pPos, hPos);

    if (distanceToHouse < 45.0f) {
        cStateX = GAME_STATE_ENDING;
    }

    float linearProgress = 0.1f;
    cM.target.x += (pPos.x - cM.target.x) * linearProgress;
    cM.target.y += (pPos.y - cM.target.y) * linearProgress;

    float minCameraX = sWidth / 2.0f;
    float maxCameraX = wWidth - sWidth / 2.0f;
    float minCameraY = sHeight / 2.0f;
    float maxCameraY = wHeight - sHeight / 2.0f;

    cM.target.x = Clamp(cM.target.x, minCameraX, maxCameraX);
    cM.target.y = Clamp(cM.target.y, minCameraY, maxCameraY);
}

void DrawGame() {
    BeginMode2D(cM);
    
    DrawTexture(pW, pPos.x, pPos.y, WHITE);

    DrawFP();

    for (int i = 0; i < MAX_CLDS; i++) {
        if (!clds[i].isForeground) {
            DrawCd(clds[i]);
        }
    }
    
    DrawRn();
    
    float time = GetTime();
    float floatOffset = sinf(time * 2.0f) * 5.0f;
    Vector2 floatingHousePos = (Vector2){
        hPos.x, hPos.y + floatOffset };

    float houseScale = 0.5f;
    Vector2 houseDrawPos = {
        floatingHousePos.x - (hTx.width * houseScale) / 2,
        floatingHousePos.y - (hTx.height * houseScale) / 2 };
    DrawTextureEx(hTx, houseDrawPos, 0.0f, houseScale, WHITE);
    
    for (int i = 0; i < MAX_CLDS; i++) {
        if (clds[i].isForeground) {
            DrawCd(clds[i]);
        }
    }
    
    Vector2 spotlightPosition = pPos;
    Rectangle destRect = {
        spotlightPosition.x - spotlightTexture.width / 2 * cM.zoom,
        spotlightPosition.y - spotlightTexture.height / 2 * cM.zoom,
        spotlightTexture.width * cM.zoom,
        spotlightTexture.height * cM.zoom
    };
    DrawTexturePro(spotlightTexture, (Rectangle){0, 0, spotlightTexture.width, spotlightTexture.height}, destRect, (Vector2){0, 0}, 0.0f, Fade(BLACK, 0.3f));

    EndMode2D();

    DrawText("Objective: Go home...", 10, 10, 20, WHITE);
}

void UnldRsrcs() {
    UnloadTexture(pW);
    UnloadTexture(cTx);
    UnloadTexture(hTx);
    UnloadTexture(spotlightTexture);
}

void UpdateMenu() {
    if (IsKeyPressed(KEY_ENTER)) {
        cStateX = GAME_STATE_PLAYING;
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        cStateX = GAME_STATE_EXIT;
    }
}

void DMn() {
    ClearBackground(DARKGRAY);
    DrawText("FogBound", sWidth / 2 - MeasureText("FogBound", 60) / 2,
             sHeight / 2 - 100, 60, WHITE);
    DrawText("Press ENTER to Start",
             sWidth / 2 - MeasureText("Press ENTER to Start", 30) / 2,
             sHeight / 2, 30, LIGHTGRAY);
    DrawText("Press ESC to Exit",
             sWidth / 2 - MeasureText("Press ESC to Exit", 30) / 2,
             sHeight / 2 + 50, 30, LIGHTGRAY);
}

void UpEnd() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        cStateX = GAME_STATE_EXIT;
    }
}

void DwE() {
    ClearBackground(DARKGRAY);
    DrawText("Home was found...(beta closed.)",
             sWidth / 2 - MeasureText("Home was found...(beta closed.)", 40) / 2,
             sHeight / 2 - 50, 40, DARKBLUE);
    DrawText("Press ENTER or ESC to Exit",
             sWidth / 2 - MeasureText("Press ENTER or ESC to Exit", 30) / 2,
             sHeight / 2 + 10, 30, LIGHTGRAY);
}
