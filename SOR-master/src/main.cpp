/*
*  Simple monte carlo path tracer in C++
*
*  Compilation:
*      $ mkdir build
*      $ cd build
*      $ cmake ..
*      $ make
*
*  Usage:
*      $ ./pathtracer <number of samples>
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include "time.h"

#include "vector.h"
#include "material.h"
#include "objects.h"
#include "camera.h"
#include "scene.h"
#include "renderer.h"

#define NUM_THREADS 4 //Inserir aqui o numero de threads

struct data_struct {
  int id;
  int samples;
  int start;
  int finish;
  void *rend;
};

int main(int argc, char *argv[]) {

  time_t start, stop;
  time(&start);               // Start execution timer
  int samples = 4;            // Default samples per pixel

  if (argc == 2) samples = atoi(argv[1]);

  Camera camera = Camera(Vec(0, -5, 2.5), Vec(0,0,1), 1280, 720);     // Create camera
  Scene scene = Scene();                                              // Create scene

  // Add objects to scene
  scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,-1000), 1000, Material())) );
  scene.add( dynamic_cast<Object*>(new Sphere(Vec(-1004,0,0), 1000, Material(DIFF, Vec(0.85,0.4,0.4)))) );
  scene.add( dynamic_cast<Object*>(new Sphere(Vec(1004,0,0), 1000, Material(DIFF, Vec(0.4,0.4,0.85)))) );
  scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,1006,0), 1000, Material())) );
  scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,110), 100, Material(EMIT, Vec(1,1,1), Vec(2.2,2.2,2.2)))) );
  scene.add( dynamic_cast<Object*>(new Mesh(Vec(), "../obj/dragon2.obj", Material(DIFF, Vec(0.9, 0.9, 0.9)))) );

  Renderer renderer = Renderer(&scene, &camera);  // Create renderer with our scene and camera

  /*----------------------------TRABALHO 1 - Sistemas Operacionais de Redes---------------------------------------------*/

  int resto = 0,       //para computar quando ainda falta para as threads pegarem no resto da divisao
      aux = 0,         //para auxiliar o resto
      n = NUM_THREADS; //para poder mexer com o numero maximo

  char img[7] = {'.','.','.','.','p','n','g'};

  int altura = renderer.get_height();   // Pega altura da imagem final

  if (NUM_THREADS > altura)             //se o n de threads for maior que a altura o programa limita isso
    n = altura;

  int particao = altura/(n);            // Divide a altura em segmentos para a renderização separada
  
  struct data_struct data_values[n];    // Cria uma struct para cada thread
  pthread_t *thread = new pthread_t[n]; // Cria NUM_THREADS threads

  for (int i = 0; i < n; i++) {         // Defini valores do struct e cria threads
    data_values[i].id = i;
    
    /*
    Foi necessario definir um resto pois caso fosse um numero grande de threads podia 
    sobrar muito na divisao que origina particao e com isso a imagem nao seria totalmente
    renderizada.

    Com um resto podemos fazer as threads iniciais pegarem uma linha a mais e as finais 
    pegarem de fato o tamanho de uma particao.
    */

    resto = altura%n - aux;             //calcula um resto para que nao sobre partes sem fazer

    if(resto == 0){                                       //as ultimas threads caem aqui
      data_values[i].start = particao*i + aux;            //e realizam somente o tamanho de uma particao de fato
      data_values[i].finish = particao*(i+1) + aux;       
    }else{                                                //as primeiras threads devem cair aqui se a divisao nao for inteira
      data_values[i].start = particao*i + aux;            //aqui elas realizam a particao dela +1 (que sera para nao faltar nada)
      data_values[i].finish = particao*(i+1) + aux + 1;   
      aux = aux + 1;
    }  

    data_values[i].samples = samples;
    data_values[i].rend = &renderer;              // Ponteiro para a variavel renderer da classe Renderer
    pthread_create(&thread[i], NULL, &Renderer::renderStatic, &data_values[i]); // Cria thread enviando a struct como argumento
  }


  for (int i = 0; i < n; i++) {
    pthread_join(thread[i], NULL);                // Espera pela finalização de todas as threads criadas
  }

  //Ccoloca no nome da imagem o numero de threads que foi usado até 999 (maximo = altura = 720)
  img[0] = n/100 + 0x30;
  n = n%100;
  img[1] = n/10 + 0x30;
  img[2] = n%10 + 0x30;

  renderer.save_image(img);                       // Save image

  // Print duration information
  time(&stop);
  double diff = difftime(stop, start);
  int hrs = (int)diff/3600;
  int mins = ((int)diff/60)-(hrs*60);
  int secs = (int)diff-(hrs*3600)-(mins*60);

  //Print nas informacoes uteis para o usuario
  printf("Executado com sucesso com: %i threads \n Tempo necessario (H:M:S): %i:%i:%i \n",n, hrs, mins, secs);
  
  return 0;

}
