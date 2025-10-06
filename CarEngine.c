#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 600

#define MAX_GEARS 8

#define UPPER_LIMIT_THRESHOLD 100

#define MAX_RPM 8000

typedef enum
{
    PEDAL_MODE_SLOW,
    PEDAL_MODE_MID,
    PEDAL_MODE_SPORT
} PedalMode;

int AutomaticChooseGear(int gear, int rpm, int pedal)
{
    PedalMode pedalMode = PEDAL_MODE_SLOW;

    if (pedal > 200)
    {
        PEDAL_MODE_SPORT;
    }
    else if (pedal > 100)
    {
        PEDAL_MODE_MID;
    }
    else
    {
        PEDAL_MODE_SLOW;
    }

    int newGear = gear;

    switch (pedalMode)
    {
    case PEDAL_MODE_SPORT:
        break;
    case PEDAL_MODE_MID:
        break;
    case PEDAL_MODE_SLOW:
        break;
    default:
        return gear;
    }

    if (rpm > MAX_RPM - UPPER_LIMIT_THRESHOLD)
    {
        newGear = gear + 1;
    }
    else if (rpm < 1200)
    {
        newGear = gear - 1;
    }
    else if (pedal > 200)
    {
        for (int i = 1; i <= gear; i++)
        {
            if (rpm + 2000 * i > MAX_RPM - UPPER_LIMIT_THRESHOLD)
            {
                newGear = gear - i + 1;
                break;
            }
        }
    }
    else
    {
        if (rpm > 5000)
        {
            newGear = gear + 1;
        }
        else
        {
            newGear = gear;
        }
    }

    if (newGear > 0 && newGear <= MAX_GEARS)
    {
        return newGear;
    }
    else
    {
        return gear;
    }
}

void Pedal(int *pedal, Vector2 mousePos)
{
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        *pedal = mousePos.y;
        if (*pedal < 300)
        {
            *pedal = 300;
        }
        else if (*pedal > 550)
        {
            *pedal = 550;
        }
        *pedal -= 300;
    }
    else
    {
        *pedal -= 1;
        if (*pedal < 0)
        {
            *pedal = 0;
        }
        else if (*pedal > 250)
        {
            *pedal = 250;
        }
    }

    DrawRectangle(700, *pedal + 300, 100, HEIGHT - *pedal - 300, GRAY);

    DrawLine(670, 300, 830, 300, RED);

    DrawLine(670, 550, 830, 550, RED);
}

void Transmission(int *gear, int *rpm, int pedal, bool isAutomatic, bool isStarted)
{
    if(!isStarted){
        *rpm = 0;
        *gear = 1;
    }

    if (isAutomatic)
    {
        int newGear = AutomaticChooseGear(*gear, *rpm, pedal);

        *rpm += (*gear - newGear) * 2000;

        *gear = newGear;
    }
    else
    {
        if (IsKeyPressed(KEY_M))
        {
            (*gear)++;
            if (*gear > MAX_GEARS)
            {
                (*gear)--;
            }
            else
            {
                *rpm -= 2000;
            }
        }
        else if (IsKeyPressed(KEY_N))
        {
            (*gear)--;
            if (*gear < 1)
            {
                (*gear)++;
            }
            else
            {
                *rpm += 2000;
            }
        }
    }
}

void Engine(int gear, int *rpm, int pedal)
{
    if (pedal != 0)
    {
        *rpm += pedal / (10 * gear);
        if (*rpm > 500 / pedal)
        {
            *rpm -= 500 / pedal;
        }
    }
    else
    {
        *rpm -= 10;
    }

    if (*rpm < 1000)
    {
        *rpm = 1000;
    }
    else if (*rpm > MAX_RPM)
    {
        *rpm = MAX_RPM + GetRandomValue(-50, 50);
    }
}

void Exhaust(Music *engineSound, int rpm, bool isStarted)
{
    UpdateMusicStream(*engineSound);
    float rpmNorm = (rpm - 1000) / 7000.0f;
    SetMusicPitch(*engineSound, 1.2f + rpmNorm * 1.8f);
    SetMusicVolume(*engineSound, isStarted ? 0.1f + rpmNorm * 0.9f : 0.0f);
}

void StartStopButton(bool *isStarted, int *rpm, Vector2 mousePos)
{
    if (CheckCollisionPointCircle(mousePos, (Vector2){WIDTH * 5 / 6, 100}, 50))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            *isStarted = !(*isStarted);
        }
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_ARROW);
    }

    DrawCircle(WIDTH * 5 / 6, 100, 52, WHITE);
    DrawCircle(WIDTH * 5 / 6, 100, 50, RED);
    DrawText("Start\nStop", WIDTH * 5 / 6 - 30, 75, 25, BLACK);
}

void Gauge(int rpm)
{
    float angleDeg = 180 - ((rpm / (float)MAX_RPM) * (180 - 30));
    float angleRad = angleDeg * (PI / 180.0f);

    Vector2 center = {WIDTH / 4, HEIGHT * 3 / 4};
    float radius = 250.0f;
    Vector2 needleEnd = {center.x + cos(angleRad) * radius, center.y - sin(angleRad) * radius};

    Color rpmTextColor = {255, 255 - rpm / 120, 255 - rpm / 60, 255};

    DrawText(TextFormat("%d RPM", rpm), center.x - MeasureText(TextFormat("%d RPM", rpm), 50) / 2, center.y - 200, 50, rpmTextColor);

    DrawCircleV(center, 5, WHITE);
    DrawCircleLinesV(center, 260, GRAY);
    DrawLineEx(center, needleEnd, 4.0f, RED);
}

int main()
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(WIDTH, HEIGHT, "Car Engine");
    InitAudioDevice();
    Music engineSound = LoadMusicStream("engineSound.mp3");
    engineSound.looping = true;
    PlayMusicStream(engineSound);
    SetTargetFPS(240);

    bool isStarted = false;
    int pedal = 0;
    int rpm = 0;
    int gear = 1;
    bool isAutomatic = true;

    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition();

        BeginDrawing();
        ClearBackground((Color){20, 20, 20, 255});

        DrawRectangleRounded((Rectangle){20, 20, 70 + 30 * !isAutomatic, 30}, 0.4f, 4, WHITE);
        DrawText(isAutomatic ? "AUTO" : "MANUAL", 25, 25, 22, RED);
        if(CheckCollisionPointRec(mousePos, (Rectangle){20, 20, 70 + 30 * !isAutomatic, 30})){
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                isAutomatic = !isAutomatic;
            }
        }
        else{
            SetMouseCursor(MOUSE_CURSOR_ARROW);
        }

        StartStopButton(&isStarted, &rpm, mousePos);

        DrawText(isStarted ? TextFormat("%d", gear) : "P", WIDTH / 2 - MeasureText(TextFormat("%d", gear), 100) / 2, HEIGHT / 8, 100, RED);

        Pedal(&pedal, mousePos);

        Transmission(&gear, &rpm, pedal, isAutomatic, isStarted);

        if (isStarted)
        {
            Engine(gear, &rpm, pedal);
        }

        Exhaust(&engineSound, rpm, isStarted);

        Gauge(rpm);

        EndDrawing();
    }

    CloseWindow();

    UnloadMusicStream(engineSound);
    CloseAudioDevice();
}