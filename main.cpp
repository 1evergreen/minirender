#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++)
    {
        if (steep)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

Vec2i world2screen(Vec3f &v, TGAImage &image)
{
    int x = (v.x + 1) * image.get_width() / 2;
    int y = (v.y + 1) * image.get_height() / 2;
    return Vec2i{x, y};
}

Vec3f barycentric(Vec2i *pts, Vec2i P)
{
    Vec3f u = Vec3f(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-P.x)^Vec3f(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-P.y);
    if(std::abs(u.z) < 1e-2 )
        return Vec3f(-1, 1, 1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}
void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
    int xmin = std::min(std::min(pts[0].x, pts[1].x), pts[2].x);
    int xmax = std::max(std::max(pts[0].x, pts[1].x), pts[2].x);
    int ymin = std::min(std::min(pts[0].y, pts[1].y), pts[2].y);
    int ymax = std::max(std::max(pts[0].y, pts[1].y), pts[2].y);

    for (int x = xmin; x <= xmax; ++x)
    {
        for (int y = ymin; y <= ymax; ++y)
        {
            auto bary = barycentric(pts, Vec2i(x, y));
            if(bary.x < 0 || bary.y < 0 || bary.z < 0)
                continue;
            image.set(x, y, color);
        }
    }
}

int main(int argc, char **argv)
{
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("/Users/wangww/Desktop/tinyrenderer/obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec2i pts[3];
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++)
        {
            Vec3f world_coords = model->vert(face[j]);
            pts[j] = world2screen(world_coords, image);
        }
        triangle(pts, image, white);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("/Users/wangww/Desktop/tinyrenderer/outputs/output2.tga");
    delete model;
    return 0;
}
