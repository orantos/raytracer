#include <iostream>
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "hitable_list.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "image_texture.hpp"
#include "aarect.hpp"
#include "box.hpp"
#include "constant_medium.hpp"
#include "bvh.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <thread>
#include <vector>
#include <sstream>

// Global variables, to be used on main() and render_scene()
const int nx = 800;
const int ny = 800;
const int ns = 100; // Number of samples

inline vec3 de_nan(const vec3& c) {
    vec3 temp = c;
    if (!(temp[0] == temp[0])) temp[0] = 0;
    if (!(temp[1] == temp[1])) temp[1] = 0;
    if (!(temp[2] == temp[2])) temp[2] = 0;
    return temp;
}

vec3 color(const ray &r, hitable *world, int depth)
{
    hit_record rec;

    if(world->hit(r, 0.001, FLT_MAX, rec))
    {
        ray scattered;
        vec3 attenuation;

        // If the material in the hitpoint is an emitting material, the color emitted affects on the returned value.
        vec3 emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

        // Probability Density Function (PDF).
        float pdf_val;
        vec3 albedo;

        if(depth < 50 && rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val))
        {
            hitable *light_shape = new xz_rect(213, 343, 227, 332, 554, 0);
            hitable_pdf pdf_0(light_shape, rec.p);
            cosine_pdf pdf_1(rec.normal);
            mixture_pdf mix_p(&pdf_0, &pdf_1);

            // We override whatever value was written in the material call to "scatter()"
            scattered = ray(rec.p, mix_p.generate(), r.time());

            pdf_val = mix_p.value(scattered.direction());

            // Color = (Albdo * scattering_pdf(direction) * color(direction)) / pdf(direction)
            return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                   * color(scattered, world, depth + 1) / pdf_val;
            // If we want to just map a texture image, return just "attenuation".
            // return attenuation;
        }
        else
        {
            return emitted;
        }
    }
    else
    {
        // Blackground is black (it was different in earlier stages of the raytracer).
        return vec3(0, 0, 0);
    }
}

hitable *cornell_box_final_book2() {
    int nb = 20;

    hitable **list = new hitable*[30];
    hitable **boxlist = new hitable*[10000];
    hitable **boxlist2 = new hitable*[10000];
    material *white = new lambertian( new constant_texture(vec3(0.73, 0.73, 0.73)) );
    material *ground = new lambertian( new constant_texture(vec3(0.48, 0.83, 0.53)) );
    int b = 0;

    for (int i = 0; i < nb; i++)
    {
        for (int j = 0; j < nb; j++)
        {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100*(drand48()+0.01);
            float z1 = z0 + w;
            boxlist[b++] = new box(vec3(x0,y0,z0), vec3(x1,y1,z1), ground);
        }
    }

    int l = 0;

    list[l++] = new bvh_node(boxlist, b, 0, 1);
    material *light = new diffuse_light( new constant_texture(vec3(7, 7, 7)) );
    list[l++] = new xz_rect(123, 423, 147, 412, 554, light);
    vec3 center(400, 400, 200);
    list[l++] = new moving_sphere(center, center+vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));
    list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5));
    list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));
    hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5));
    list[l++] = boundary;
    list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));
    boundary = new sphere(vec3(0, 0, 0), 5000, new dielectric(1.5));
    list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));
    int nx, ny, nn;
    unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
    material *emat =  new lambertian(new image_texture(tex_data, nx, ny));
    list[l++] = new sphere(vec3(400,200, 400), 100, emat);
    texture *pertext = new noise_texture(0.1);
    list[l++] =  new sphere(vec3(220,280, 300), 80, new lambertian( pertext ));
    int ns = 1000;
    for (int j = 0; j < ns; j++)
    {
        boxlist2[j] = new sphere(vec3(165*drand48(), 165*drand48(), 165*drand48()), 10, white);
    }
    list[l++] =   new translate(new rotate_y(new bvh_node(boxlist2,ns, 0.0, 1.0), 15), vec3(-100,270,395));
    return new hitable_list(list,l);
}

hitable *cornell_smoke()
{
    hitable **list = new hitable *[8];

    int i = 0;

    material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
    material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
    material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
    material *light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));

    // Cornell Box
    list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
    list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

    hitable *b1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    hitable *b2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    // Light particles smoke
    list[i++] = new constant_medium(b1, 0.01, new constant_texture(vec3(1.0, 1.0, 1.0)));
    // Dark particles smoke
    list[i++] = new constant_medium(b2, 0.01, new constant_texture(vec3(0.0, 0.0, 0.0)));

    return new hitable_list(list, i);
}

hitable *cornell_box()
{
    hitable **list = new hitable *[8];

    int i = 0;

    material *red = new lambertian(new constant_texture(vec3(0.65, 0.05, 0.05)));
    material *white = new lambertian(new constant_texture(vec3(0.73, 0.73, 0.73)));
    material *green = new lambertian(new constant_texture(vec3(0.12, 0.45, 0.15)));
    material *light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));

    // Cornell Box
    list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new flip_normals(new xz_rect(213, 343, 227, 332, 554, light));
    list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

    // 2 Boxes inside the room (withut rotation or translation)
    // list[i++] = new box(vec3(130, 0, 65), vec3(295, 165, 230), white);
    // list[i++] = new box(vec3(265, 0, 295), vec3(430, 330, 460), white);

    list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    return new hitable_list(list, i);
}

hitable *simple_light()
{
    texture *pertext = new noise_texture(4);

    hitable **list = new hitable *[4];

    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
    list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
    // Lights are brighter than (1, 1, 1) to allow it to be bright enough to light things.
    list[2] = new sphere(vec3(0, 7, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
    list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));

    return new hitable_list(list, 4);
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

void render_scene(int start_row, int end_row, int *pixels)
{
    //hitable *world = cornell_box_final_book2();
    //hitable *world = cornell_smoke();
    hitable *world = cornell_box();
    //hitable *world = simple_light();
    //hitable *world = earth();
    //hitable *world = two_perlin_spheres();

    // Final scene book 2 camera settings
    //vec3 lookfrom(478, 278, -600);
    //vec3 lookat(278, 278, 0);

    // Cornell Box camera settings
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 40.0;
    // Setup until chapter 6 (included) of the second book.
    //vec3 lookfrom(20, 4, 5);
    //vec3 lookat(0, 2, 0);
    //float dist_to_focus = 10.0;
    //float aperture = 0.0;

    camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
    // Setup until chapter 6 (included) of the second book.
    //camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

    for (int j = end_row; j >= start_row; j--)
    {
        int *row = &pixels[nx * j * 3];
        for (int i = 0; i < nx; i++)
        {
            vec3 col{0, 0, 0};

            for(int s = 0; s < ns; s++)
            {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);

                ray r = cam.get_ray(u, v);
                col += de_nan(color(r, world, 0));
            }

            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);

            // Access the current pixel to store. There are 3 components per pixel.
            row[(i*3) + 0] = ir;
            row[(i*3) + 1] = ig;
            row[(i*3) + 2] = ib;
        }
    }
}

int main()
{
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    /* Multithread code starts here */
    int pixels[nx * ny * 3]; // temporary buffer to store 3 components per pixel.
    int num_threads = 4;
    int rows_per_thread = ny / num_threads;

    if ((rows_per_thread % num_threads) != 0)
    {
        std::cerr << "\nThe image height must be divisible by the number of threads used\n";
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        int start_row = rows_per_thread * i;
        int end_row;

        if (i + 1 == num_threads)
        {
            end_row = ny - 1;
        }
        else
        {
            end_row = start_row + rows_per_thread - 1;
        }

        threads.push_back(std::thread(render_scene, start_row, end_row, pixels));
    }

        for (std::thread &t : threads)
    {
        t.join();
    }

    // Write pixels to the ppm file
    for (int j = ny-1; j >= 0; j--)
    {
        int *row = &pixels[nx * j * 3];

        for (int i = 0; i < nx; i++)
        {
            std::cout << row[i*3 + 0] << " " << row[i*3 + 1] << " " << row[i*3 + 2] << "\n";
        }
    }
    // Multithread code ends here.
#if 0
    // SINGLE THREAD CODE
    /*
    hitable *list[5];
    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100,  new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list, 5);
    */

    //hitable *world = cornell_box();
    //hitable *world = cornell_box_final_book2();
    hitable *world = cornell_smoke();
    //hitable *world = simple_light();
    //hitable *world = earth();
    //hitable *world = two_perlin_spheres();

    // Final scene book 2 camera settings
    //vec3 lookfrom(478, 278, -600);
    //vec3 lookat(278, 278, 0);

    // Cornell Box camera settings
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 40.0;

    // Setup until chapter 6 (included) of the second book.
    //vec3 lookfrom(20, 4, 5);
    //vec3 lookat(0, 2, 0);
    //float dist_to_focus = 10.0;
    //float aperture = 0.0;

    camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
    // Setup until chapter 6 (included) of the second book.
    //camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

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
#endif
}
