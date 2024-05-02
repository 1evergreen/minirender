#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

// #include 

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;
Vec3f ColorDirection = {0, 0, -1};
Vec3f camera = {0, 0, 3};


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

Vec2i world2screen(Vec3f v, const TGAImage &image)
{
    int x = (v.x + 1) * image.get_width() / 2;
    int y = (v.y + 1) * image.get_height() / 2;
    return Vec2i{x, y};
}

Vec3f barycentric(Vec2i A, Vec2i B, Vec2i C, Vec2i P)
{
    Vec3f u = Vec3f(C.x - A.x, B.x-A.x, A.x-P.x)^Vec3f(C.y-A.y, B.y-A.y, A.y-P.y);
    if(std::abs(u.z) < 1e-2 )
        return Vec3f(-1, 1, 1);
    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

float compute_intensity(Vec3f *world_coords)
{
    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
    n.normalize(); 
    return n * ColorDirection;
}
struct ZBuffer
{
    ZBuffer()=delete;
    ZBuffer(TGAImage &i) {
        m_image = new TGAImage(i.get_width(), i.get_height(), TGAImage::GRAYSCALE, 0);
    }
    ~ZBuffer() {
        delete m_image;
    }
    TGAImage *m_image;
    unsigned char get(int x, int y) const
    {
        return m_image->get_value(x, y);
    }
    void set(int x, int y, unsigned char value)
    {
        m_image->set(x, y, value);
    }

};

unsigned char compute_z(Vec3f *world_coords, Vec3f &bary, unsigned char scaling_factor = 255)
{
    float z = world_coords[0].z * bary.x + world_coords[1].z * bary.y + world_coords[2].z * bary.z;
    z = (z + 1.f) / 2.f;
    return z * scaling_factor;
}
Vec3f interpolate(Vec3f *world_coords, Vec3f &bary)
{
    return world_coords[0] * bary.x + world_coords[1] * bary.y + world_coords[2] * bary.z;
}

// transformat
Vec3f transformation(Vec3f &v)
{
    float factor = 1.f - (v.z  / camera.z);
    return Vec3f{v.x / factor, v.y / factor, v.z};

}
TGAColor compute_uv_color(Vec3f *uv_coords, Vec3f &bary, const TGAImage &texture)
{
    Vec3f coord =  uv_coords[0] * bary.x + uv_coords[1] * bary.y + uv_coords[2] * bary.z;
    int x = coord.x * texture.get_width();
    int y = coord.y * texture.get_height();
    return texture.get(x, y);
}
void triangle(Vec2i *screen_coords, Vec3f *world_coords, Vec3f *uv_coords, TGAImage &image, ZBuffer &zbuffer, const TGAImage &texture, float intensity)
{
    int xmin = std::min({screen_coords[0].x, screen_coords[1].x, screen_coords[2].x});
    xmin = std::max(0, xmin);
    int xmax = std::max({screen_coords[0].x, screen_coords[1].x, screen_coords[2].x});
    xmax = std::min(image.get_width() - 1, xmax);
    int ymin = std::min({screen_coords[0].y, screen_coords[1].y, screen_coords[2].y});
    ymin = std::max(0, ymin);
    int ymax = std::max({screen_coords[0].y, screen_coords[1].y, screen_coords[2].y});
    ymax = std::min(image.get_height() - 1, ymax);

    for (int x = xmin; x <= xmax; ++x)
    {
        for (int y = ymin; y <= ymax; ++y)
        {
            auto bary = barycentric(screen_coords[0], 
                                    screen_coords[1],
                                    screen_coords[2],
                                    Vec2i(x, y));
            auto z = compute_z(world_coords, bary);
            if(bary.x < 0 || bary.y < 0 || bary.z < 0 || z < zbuffer.get(x, y))
                continue;
            zbuffer.set(x, y, z);
            auto color = compute_uv_color(uv_coords, bary, texture);
            image.set(x, y, color * intensity);
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
        model->load_texture("/Users/wangww/Desktop/tinyrenderer/obj/african_head_diffuse.tga");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f world_coords[3];
    Vec2i screen_coords[3];
    Vec3f uv_coords[3];
    ZBuffer zbuffer(image);
    for (int _ = 0; _ < model->nfaces(); _++)
    {
        auto &face = model->face(_);
        auto &uv_face = model->uv_face(_);
        for (int j = 0; j < 3; j++)
        {
            world_coords[j] = model->vert(face[j]);
            screen_coords[j] = world2screen(transformation(world_coords[j]), image);
            uv_coords[j] = model->uv(uv_face[j]);
        }
        float intensity = compute_intensity(world_coords);
        if(intensity > 0)
            triangle(screen_coords, world_coords, uv_coords, image, zbuffer, model->texture(), intensity);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("/Users/wangww/Desktop/tinyrenderer/outputs/output4.tga");
    zbuffer.m_image->flip_vertically(); // i want to have the origin at the left bottom corner of the image

    zbuffer.m_image->write_tga_file("/Users/wangww/Desktop/tinyrenderer/outputs/zbuffer4.tga");
    delete model;
    return 0;
}


