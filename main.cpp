#include <iostream>
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "hitable_list.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "image_texture.hpp"
#include "float.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


vec3 color(const ray &r, hitable *world, int depth)
{
    hit_record rec;

    if(world->hit(r, 0.001, FLT_MAX, rec))
    {
        ray scattered;
        vec3 attenuation;

        if(depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * color(scattered, world, depth + 1);
            // If we want to just map a texture image, return just "attenuation".
            // return attenuation;
        }
        else
        {
            return vec3(0, 0, 0);
        }
    }
    else
    {
        vec3 unit_direction = unit_vector(r.direction());
        float t =  0.5 * (unit_direction.y() + 1.0);

        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

hitable *earth() {
    int nx, ny, nn;
    unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    material *mat =  new lambertian(new image_texture(tex_data, nx, ny));
    return new sphere(vec3(0,0, 0), 2, mat);
}

hitable *two_perlin_spheres()
{
    texture *perlin_texture = new noise_texture(4);

    hitable **list = new hitable *[2];

    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(perlin_texture));
    list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(perlin_texture));

    return new hitable_list(list, 2);
}

hitable *two_spheres()
{
    texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)),
                       new constant_texture(vec3(0.9, 0.9, 0.9)));

    int n = 50;

    hitable **list = new hitable *[n + 1];

    list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
    list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));

    return new hitable_list(list, 2);
}

hitable *random_scene()
{
    int n = 50000;
    hitable **list = new hitable *[n+1];

    texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)),
                                           new constant_texture(vec3(0.9, 0.9, 0.9)));
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));

    int i = 1;

    for(int a = -10; a < 10; a++)
    {
        for(int b = -10; b < 10; b++)
        {
            float choose_mat = drand48();
            vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());

            if ((center - vec3(4, 0.2, 0)).length() > 0.9)
            {
                if (choose_mat < 0.8) // Diffuse
                {
                    vec3 albedo = vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48());
                    list[i++] = new moving_sphere(center, center + vec3(0, 0.5 * drand48(), 0), 0.0, 1.0, 0.2,
                                                  new lambertian(new constant_texture(albedo)));
                }
                else if (choose_mat < 0.95) // Metal
                {
                    vec3 albedo = vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48()));
                    float fuzz = 0.5 * drand48();
                    list[i++] = new sphere(center, 0.2, new metal(albedo, fuzz));
                }
                else // Glass
                {
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new hitable_list(list, i);
}

int main()
{
    int nx = 800;
    int ny = 400;
    int ns = 200; // Number of samples

    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    /*
    hitable *list[5];
    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100,  new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list, 5);
    */

    hitable *world = earth();
    //hitable *world = two_perlin_spheres();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;

    camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

    for (int j = ny - 1; j >= 0; j--)
    {
        for (int i = 0; i < nx; i++)
        {
            vec3 col{0, 0, 0};

            for(int s = 0; s < ns; s++)
            {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);

                ray r = cam.get_ray(u, v);
                col += color(r, world, 0);
            }

            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);

            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
}
