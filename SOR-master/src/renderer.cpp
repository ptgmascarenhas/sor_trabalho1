#include <vector>
#include <stdio.h>
#include <iostream>

#include "renderer.h"
#include "../lib/lodepng/lodepng.h"

typedef struct data_struct {
  int id;
  int samples;
  int start;
  int finish;
  void *rend;
} data;

// Clamp double to min/max of 0/1
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
// Clamp to between 0-255
inline int toInt(double x){ return int(clamp(x)*255+.5); }

Renderer::Renderer(Scene *scene, Camera *camera) {
    m_scene = scene;
    m_camera = camera;
    m_pixel_buffer = new Vec[m_camera->get_width()*m_camera->get_height()];
}

int Renderer::get_height (void) {
  return m_camera->get_height();
}

void *Renderer::render(int samples,int id, int start, int finish) {
    int width = m_camera->get_width();
    int height = m_camera->get_height();
    double samples_recp = 1./samples;

    // Main Loop
    for (int y = start; y < finish; y++) { // Realiza a renderizacao apenas entre valores definidos
        unsigned short Xi[3]={0,0,y*y*y};               // Stores seed for erand48

        fprintf(stderr, "\rRendering Thread '%i' (%i samples): %.2f%% ",// Prints
                id, samples, (double)y/finish*100);                     // progress

        for (int x=0; x<width; x++) {
            Vec col = Vec();

            for (int a=0; a<samples; a++){
                Ray ray = m_camera->get_ray(x, y, a>0, Xi);
                col = col + m_scene->trace_ray(ray,0,Xi);
            }
            m_pixel_buffer[(y)*width + x] = col * samples_recp;
        }
    }
}

void *Renderer::renderStatic (void *arg) {
  data *data_renderer = (data *)arg; // Converte a variavel passada de void* de volta para struct data_struct
  // Declarações e definicoes autoexplicativas
  int start = data_renderer->start;
  int finish = data_renderer->finish;
  int samples = data_renderer->samples;
  int id = data_renderer->id;
  Renderer *rend = (Renderer *)data_renderer->rend;
  rend->render(samples,id,start,finish); // Utilizando a variavel renderer (rend) principal do programa, chama a função render com os argumentos citados
}

void Renderer::save_image(const char *file_path) {
    int width = m_camera->get_width();
    int height = m_camera->get_height();

    std::vector<unsigned char> pixel_buffer;

    int pixel_count = width*height;

    for (int i=0; i<pixel_count; i++) {
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].x));
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].y));
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].z));
        pixel_buffer.push_back(255);
    }

    //Encode the image
    unsigned error = lodepng::encode(file_path, pixel_buffer, width, height);
    //if there's an error, display it
    if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;

    pixel_buffer.clear();
}
