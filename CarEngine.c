#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 600

#define MAX_GEARS 8

#define UPPER_LIMIT_THRESHOLD 100

#define MAX_RPM 8000

int AutomaticChooseGear(int gear, int rpm, int pedal)
{
    typedef enum
    {
        PEDAL_MODE_SLOW,
        PEDAL_MODE_MID,
        PEDAL_MODE_SPORT
    } PedalMode;

    PedalMode pedalMode = PEDAL_MODE_SLOW;

    if (pedal > 200)
    {
        pedalMode = PEDAL_MODE_SPORT;
    }
    else if (pedal > 100)
    {
        pedalMode = PEDAL_MODE_MID;
    }
    else
    {
        pedalMode = PEDAL_MODE_SLOW;
    }

    int newGear = gear;

    switch (pedalMode)
    {
    case PEDAL_MODE_SPORT:
        if (rpm > MAX_RPM - UPPER_LIMIT_THRESHOLD)
        {
            newGear = gear + 1;
            break;
        }
        for (int i = 1; i <= gear; i++)
        {
            if (rpm + 2000 * i > MAX_RPM - UPPER_LIMIT_THRESHOLD)
            {
                newGear = gear - i + 1;
                break;
            }
        }
        break;
    case PEDAL_MODE_MID:
        if (rpm > 6000)
        {
            newGear = gear + 1;
        }
        else if (rpm < 3000)
        {
            newGear = gear - 1;
        }
        else
        {
            newGear = gear;
        }
        break;
    case PEDAL_MODE_SLOW:
        if (rpm > 4000)
        {
            newGear = gear + 1;
        }
        else if (rpm < 1500)
        {
            newGear = gear - 1;
        }
        else
        {
            newGear = gear;
        }
        break;
    default:
        return gear;
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
    else if (IsKeyDown(KEY_W))
    {
        *pedal += 5;
        if (*pedal > 250)
        {
            *pedal = 250;
        }
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
    if (!isStarted)
    {
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
    SetMusicPitch(*engineSound, 0.6f + rpmNorm * 1.4f);
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

    DrawRectangleLines(center.x - 130, center.y - 205, 50, 50, (Color){150, 150, 150, 255});
    DrawRectangleLines(center.x - 80, center.y - 205, 50, 50, (Color){150, 150, 150, 255});
    DrawRectangleLines(center.x - 30, center.y - 205, 50, 50, (Color){150, 150, 150, 255});
    DrawRectangleLines(center.x + 20, center.y - 205, 50, 50, (Color){150, 150, 150, 255});

    DrawText(TextFormat("%d", rpm / 1000), center.x - 118, center.y - 202, 50, rpmTextColor);
    DrawText(TextFormat("%d", rpm / 100 % 10), center.x - 68, center.y - 202, 50, rpmTextColor);
    DrawText(TextFormat("%d", rpm / 10 % 10), center.x - 18, center.y - 202, 50, rpmTextColor);
    DrawText(TextFormat("%d", rpm % 10), center.x + 32, center.y - 202, 50, rpmTextColor);

    DrawText(TextFormat("RPM"), center.x + 80, center.y - 190, 40, rpmTextColor);

    DrawCircleV(center, 5, WHITE);
    DrawCircleLinesV(center, 260, GRAY);
    DrawLineEx(center, needleEnd, 4.0f, RED);
}

void GearIndicator(bool isStarted, int gear)
{

    DrawRectangleRounded((Rectangle){WIDTH / 2 - 70, HEIGHT / 8 - 20, 140, 140}, 0.6f, 4, (Color){30, 30, 30, 255});

    DrawText(isStarted ? TextFormat("%d", gear) : "P", WIDTH / 2 - MeasureText(isStarted ? TextFormat("%d", gear) : "P", 100) / 2, HEIGHT / 8 + 5, 100, RED);
}

void Spedometer(float *speed, int rpm, int gear)
{
    *speed = (rpm / 2000.0f) * gear * 10.0f;
    DrawText(TextFormat("%.0f km/h", *speed), WIDTH / 2 - MeasureText(TextFormat("%.0f km/h", *speed), 20) / 2, 10, 20, WHITE);
}

void TransmissionSwitch(bool *isAutomatic, Vector2 mousePos)
{
    DrawRectangleRounded((Rectangle){16, 16, 78 + 30 * !*isAutomatic, 38}, 0.4f, 4, WHITE);
    DrawRectangleRounded((Rectangle){18, 18, 74 + 30 * !*isAutomatic, 34}, 0.4f, 4, (Color){30, 30, 30, 255});
    DrawText(*isAutomatic ? "AUTO" : "MANUAL", 25, 25, 22, WHITE);
    if (CheckCollisionPointRec(mousePos, (Rectangle){20, 20, 70 + 30 * !*isAutomatic, 30}))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            *isAutomatic = !*isAutomatic;
        }
    }
}

void ShowCar(float speed)
{
    typedef enum
    {
        BLINKER_NONE,
        BLINKER_LEFT,
        BLINKER_RIGHT
    } BlinkerMode;

    speed = speed * 2;
    static float carX = 600;
    static float carY = 550;
    static float carRot = 270.0f;
    static BlinkerMode blinker = BLINKER_NONE;
    static float blinkTimer = 0.0f;
    static bool blinkOn = false;

    if (IsKeyPressed(KEY_Q))
    {
        if (blinker == BLINKER_LEFT)
        {
            blinker = BLINKER_NONE;
        }
        else
        {
            blinkTimer = 0.0f;
            blinkOn = true;
            blinker = BLINKER_LEFT;
        }
    }
    if (IsKeyPressed(KEY_E))
    {
        if (blinker == BLINKER_RIGHT)
        {
            blinker = BLINKER_NONE;
        }
        else
        {
            blinkTimer = 0.0f;
            blinkOn = true;
            blinker = BLINKER_RIGHT;
        }
    }

    blinkTimer += GetFrameTime();
    if (blinkTimer >= 0.5f)
    {
        blinkTimer = 0.0f;
        blinkOn = !blinkOn;
    }

    float rotSpeed = speed == 0 ? 0.0f : 1.0f;

    if (IsKeyDown(KEY_A))
        carRot -= rotSpeed;
    if (IsKeyDown(KEY_D))
        carRot += rotSpeed;

    float rad = carRot * (PI / 180.0f);
    float dirX = cosf(rad);
    float dirY = sinf(rad);

    carX += dirX * speed / 100;
    carY += dirY * speed / 100;

    DrawRectanglePro(
        (Rectangle){carX, carY, 40, 20},
        (Vector2){40 / 2, 20 / 2},
        carRot,
        (Color){0, 255, 255, 255});

    if (blinkOn && blinker == BLINKER_LEFT)
    {
        Vector2 leftOffset = {18, -8};

        float ox = leftOffset.x * cosf(rad) - leftOffset.y * sinf(rad);
        float oy = leftOffset.x * sinf(rad) + leftOffset.y * cosf(rad);

        DrawCircle(carX + ox, carY + oy, 3, YELLOW);
    }
    else if (blinkOn && blinker == BLINKER_RIGHT)
    {
        Vector2 leftOffset = {18, 8};

        float ox = leftOffset.x * cosf(rad) - leftOffset.y * sinf(rad);
        float oy = leftOffset.x * sinf(rad) + leftOffset.y * cosf(rad);

        DrawCircle(carX + ox, carY + oy, 3, YELLOW);
    }
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
    float speed = 0;
    bool isAutomatic = true;

    while (!WindowShouldClose())
    {
        SetMouseCursor(MOUSE_CURSOR_ARROW);

        Vector2 mousePos = GetMousePosition();

        BeginDrawing();
        ClearBackground((Color){20, 20, 20, 255});

        TransmissionSwitch(&isAutomatic, mousePos);

        StartStopButton(&isStarted, &rpm, mousePos);

        GearIndicator(isStarted, gear);

        Pedal(&pedal, mousePos);

        Transmission(&gear, &rpm, pedal, isAutomatic, isStarted);

        if (isStarted)
        {
            Engine(gear, &rpm, pedal);
        }

        Exhaust(&engineSound, rpm, isStarted);

        Gauge(rpm);

        Spedometer(&speed, rpm, gear);

        ShowCar(speed);

        EndDrawing();
    }

    CloseWindow();

    UnloadMusicStream(engineSound);
    CloseAudioDevice();
}