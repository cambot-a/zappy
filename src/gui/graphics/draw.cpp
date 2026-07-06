/*
** EPITECH PROJECT, 2026
** zappy_raylib
** File description:
** draw
*/

#include "gui/draw.hpp"

void draw::End_Draw()
{
    EndDrawing();
}

void draw::Start_Draw()
{
    BeginDrawing();
}

void draw::Draw_text(std::string text, Vector2 pos, int fontsize, Color color)
{
    DrawText(text.c_str(), pos.x, pos.y, fontsize, color);
}

void draw::Draw_fps(int posX, int posY)
{
    DrawFPS(posX, posY);
}

void draw::Draw_rec(Vector2 pos, int width, int height, Color color)
{
    DrawRectangle(pos.x, pos.y, width, height, color);
}

void draw::Draw_rec_line(Vector2 pos, int width, int height, Color color)
{
    DrawRectangleLines(pos.x, pos.y, width, height, color);
}

void draw::Draw_grid(int slice, float spacing)
{
    DrawGrid(slice, spacing);
}

void draw::DrawTextureIn3D(Texture2D texture, Vector3 position, Vector3 normal, Vector3 up,
                     float width, float height, Rectangle source, Color tint)
{
    normal = Vector3Normalize(normal);
    up = Vector3Normalize(up);
    up = Vector3Normalize(Vector3Subtract(up, Vector3Scale(normal, Vector3DotProduct(up, normal))));
    Vector3 right = Vector3CrossProduct(normal, up);
    float hw = width  * 0.5f;
    float hh = height * 0.5f;
    Vector3 corners[4] = {
        Vector3Add(position, Vector3Add(Vector3Scale(right, -hw), Vector3Scale(up, -hh))), // bottom-left
        Vector3Add(position, Vector3Add(Vector3Scale(right,  hw), Vector3Scale(up, -hh))), // bottom-right
        Vector3Add(position, Vector3Add(Vector3Scale(right,  hw), Vector3Scale(up,  hh))), // top-right
        Vector3Add(position, Vector3Add(Vector3Scale(right, -hw), Vector3Scale(up,  hh))), // top-left
    };

    float u0 = source.x / (float)texture.width;
    float v0 = source.y / (float)texture.height;
    float u1 = (source.x + source.width) / (float)texture.width;
    float v1 = (source.y + source.height)/ (float)texture.height;
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlTexCoord2f(u0, v1); rlVertex3f(corners[0].x, corners[0].y, corners[0].z); // bottom-left
    rlTexCoord2f(u1, v1); rlVertex3f(corners[1].x, corners[1].y, corners[1].z); // bottom-right
    rlTexCoord2f(u1, v0); rlVertex3f(corners[2].x, corners[2].y, corners[2].z); // top-right
    rlTexCoord2f(u0, v0); rlVertex3f(corners[3].x, corners[3].y, corners[3].z); // top-left
    rlEnd();
    rlSetTexture(0);
}
