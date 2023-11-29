#include <stdio.h>
#include "raylib.h"

#define GRID_SIZE 50
#define MAX_SQUARES 10000
int playerX = 50;
int playerY = 300;
int gravity = 10;
int jump = -21;

void player()
{
    DrawRectangle(playerX, playerY, 50, 50, RAYWHITE);
    playerY += 10;
    if (IsKeyDown(KEY_SPACE))
    {
        playerY += jump;
    }
}


typedef struct {
    Rectangle rect;
    Color color;
} GridSquare;

void SaveMap(const char *folder, const char *filename, GridSquare *squares, int squaresCount);
void LoadMap(const char *folder, const char *filename, GridSquare *squares, int *squaresCount);

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "draw and share!");

    GridSquare squares[MAX_SQUARES];
    int squaresCount = 0;

    bool showGrid = true;
    Color gridColor = GRAY;

    Color colorPalette[] = {BLUE, GREEN, ORANGE, PINK, PURPLE, RED, YELLOW, WHITE};
    int currentColorIndex = 0;

    bool pickingColor = false;
    Color pickedColor = WHITE;
    int pickedSquareIndex = -1;

    Camera2D camera = {0};
    camera.target = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    LoadMap("map", "map.txt", squares, &squaresCount);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        camera.target.x += IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
        camera.target.y += IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
        camera.zoom += (GetMouseWheelMove() * 0.05f);
        if (IsKeyPressed(KEY_S)) {
            // Save the map when 'S' is pressed
            SaveMap("maps", "map.txt", squares, squaresCount);
        }
        if (IsKeyPressed(KEY_L)) {
            // Load the map when 'L' is pressed
            LoadMap("maps", "map.txt", squares, &squaresCount);
        }

        if (IsKeyPressed(KEY_G)) {
            showGrid = !showGrid;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePosition = GetMousePosition();
            Vector2 worldMousePos = GetScreenToWorld2D(mousePosition, camera);

            int snappedX = ((int)worldMousePos.x / GRID_SIZE) * GRID_SIZE;
            int snappedY = ((int)worldMousePos.y / GRID_SIZE) * GRID_SIZE;

            if (pickingColor) {
                if (pickedSquareIndex >= 0 && pickedSquareIndex < squaresCount) {
                    squares[pickedSquareIndex].color = pickedColor;
                    pickingColor = false;
                }
            } else {
                squares[squaresCount].rect = (Rectangle){snappedX, snappedY, GRID_SIZE, GRID_SIZE};
                squares[squaresCount].color = colorPalette[currentColorIndex];
                squaresCount = (squaresCount + 1) % MAX_SQUARES;
            }
        }

        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 worldMousePos = GetScreenToWorld2D(mousePos, camera);

            for (int i = 0; i < squaresCount; i++) {
                if (CheckCollisionPointRec(worldMousePos, squares[i].rect)) {
                    for (int j = i; j < squaresCount - 1; j++) {
                        squares[j] = squares[j + 1];
                    }
                    squaresCount--;
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_C)) {
            currentColorIndex = (currentColorIndex + 1) % (sizeof(colorPalette) / sizeof(colorPalette[0]));
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            Vector2 worldMousePos = GetScreenToWorld2D(mousePos, camera);

            for (int i = 0; i < squaresCount; i++) {
                if (CheckCollisionPointRec(worldMousePos, squares[i].rect)) {
                    pickingColor = true;
                    pickedSquareIndex = i;
                    pickedColor = squares[i].color;
                    break;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        if (showGrid) {
            for (int i = 0; i <= screenWidth / GRID_SIZE; i++) {
                DrawLine(i * GRID_SIZE, 0, i * GRID_SIZE, screenHeight, gridColor);
            }
            for (int i = 0; i <= screenHeight / GRID_SIZE; i++) {
                DrawLine(0, i * GRID_SIZE, screenWidth, i * GRID_SIZE, gridColor);
            }
        }

        for (int i = 0; i < squaresCount; i++) {
            DrawRectangleRec(squares[i].rect, squares[i].color);
        }

        if (showGrid) {
            Vector2 mousePos = GetMousePosition();
            Vector2 worldMousePos = GetScreenToWorld2D(mousePos, camera);

            int gridX = ((int)worldMousePos.x / GRID_SIZE) * GRID_SIZE;
            int gridY = ((int)worldMousePos.y / GRID_SIZE) * GRID_SIZE;
            DrawRectangleLines(gridX, gridY, GRID_SIZE, GRID_SIZE, RED);
        }


        EndMode2D();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void SaveMap(const char *folder, const char *filename, GridSquare *squares, int squaresCount) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);

    FILE *file = fopen(filepath, "w");
    if (file != NULL) {
        fprintf(file, "%d\n", squaresCount);
        for (int i = 0; i < squaresCount; i++) {
            fprintf(file, "%f %f %f %f %d %d %d %d\n", squares[i].rect.x, squares[i].rect.y,
                    squares[i].rect.width, squares[i].rect.height,
                    squares[i].color.r, squares[i].color.g, squares[i].color.b, squares[i].color.a);
        }
        fclose(file);
    } else {
        TraceLog(LOG_ERROR, "Failed to open file for writing!");
    }
}

void LoadMap(const char *folder, const char *filename, GridSquare *squares, int *squaresCount) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);

    FILE *file = fopen(filepath, "r");
    if (file != NULL) {
        fscanf(file, "%d", squaresCount);
        for (int i = 0; i < *squaresCount; i++) {
            fscanf(file, "%f %f %f %f %d %d %d %d", &squares[i].rect.x, &squares[i].rect.y,
                   &squares[i].rect.width, &squares[i].rect.height,
                   &squares[i].color.r, &squares[i].color.g, &squares[i].color.b, &squares[i].color.a);
        }
        fclose(file);
    } else {
        TraceLog(LOG_WARNING, "No existing map file found. Creating a new map.");
    }
}