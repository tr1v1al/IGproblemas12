// Ejemplo mínimo de código OpenGL, usa OpenGL 3.3 + GLSL 3.3 + GLFW



// includes de la librería estándard de C++
#include <cassert>   // 'assert' (enforce preconditions)
#include <cstring>   // 'strlen' (to compile shaders)
#include <iostream>  // 'cout' and such
#include <iomanip>   // set precision and such
#include <vector>    // 'std::vector' types
#include <stdlib.h>     /* srand, rand */
#include <math.h>  // atan2

// incluir cabeceras de OpenGL y GLM
#include "glincludes.h"

// incluir cabeceras auxiliares para shaders, vaos y vbos.
#include "cauce.h"      // clase 'Cauce'
#include "vaos-vbos.h"  // clases 'DescrVAO', 'DescrVBOAtribs' y 'DescrVBOInds' 

// ---------------------------------------------------------------------------------------------
// Constantes y variables globales

// constexpr GLuint
//     ind_atrib_posiciones = 0,      // índice del atributo de vértice con su posiciones (debe ser el índice 0, siempre)
//     ind_atrib_colors    = 1,      // índice del atributo de vértice con su color RGB
//     num_atribs           = 2 ;     // número de atributos que gestionan los shaders
bool
    redibujar_ventana   = true ,   // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int
    ancho_actual        = 512 ,    // ancho actual del framebuffer, en pixels
    alto_actual         = 512 ;    // alto actual del framebuffer, en pixels
DescrVAO
    * vao_ind          = nullptr , // identificador de VAO (vertex array object) para secuencia indexada
    * vao_no_ind       = nullptr , // identificador de VAO para secuencia de vértices no indexada
    * vao_glm          = nullptr ; // identificador de VAO para secuencia de vértices guardada en vectors de vec3
Cauce 
    * cauce            = nullptr ; // puntero al objeto de la clase 'Cauce' en uso.

// Si no usamos DescrVao usamos opengl directamente, ejercicios 1.4-1.6
GLuint vao_noclass = 0;
std::vector<DescrVBOAtribs *> dvbo_atributo ;
DescrVBOInds * dvbo_indices   = nullptr ; 
GLuint buffer = 0;
DescrVAO * vao_cuad = nullptr, * vao_trian = nullptr;

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// no indexado, usando la clase 'DescrVAO' (declarada en 'vaos-vbos.h')
// el triángulo se dibuja en primer lugar relleno con colores, y luego las aristas en negro


void DibujarTriangulo_NoInd( )
{
    assert( glGetError() == GL_NO_ERROR );

    // la primera vez, crear e inicializar el VAO
    if ( vao_no_ind == nullptr )
    {
        // número de vértices que se van a dibujar
        constexpr unsigned num_verts = 3 ;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        const GLfloat
            posiciones[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
            colores   [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 };

        // Crear VAO con posiciones, colores e indices
        vao_no_ind = new DescrVAO( cauce->num_atribs, new DescrVBOAtribs( cauce->ind_atrib_posiciones, GL_FLOAT, 2, num_verts, posiciones ));
        vao_no_ind->agregar( new DescrVBOAtribs( cauce->ind_atrib_colores, GL_FLOAT, 3, num_verts, colores ));    
    }
    
    assert( glGetError() == GL_NO_ERROR );

    // duibujar relleno usando los colores del VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( false );
    vao_no_ind->habilitarAtrib( cauce->ind_atrib_colores, true );
    vao_no_ind->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );

    // dibujar las líneas usando color negro
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_no_ind->habilitarAtrib( cauce->ind_atrib_colores, false );
    vao_no_ind->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo  en modo diferido,
// indexado, usando la clase  'DescrVAO' (declarada en vaos-vbos.h)
// el triángulo se dibuja en primer lugar relleno con colores, y luego las aristas en negro

void DibujarTriangulo_Ind( )
{
    assert( glGetError() == GL_NO_ERROR );

    if ( vao_ind == nullptr )
    {
         // número de vértices e índices que se van a dibujar
        constexpr unsigned num_verts = 3, num_inds  = 3 ;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        const GLfloat
            posiciones[ num_verts*2 ] = {  -0.4, -0.4,      +0.4, -0.4,     0.0, +0.4      },
            colores   [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;
        const GLuint
            indices   [ num_inds    ] = { 0, 1, 2 };

        vao_ind = new DescrVAO( cauce->num_atribs, new DescrVBOAtribs( cauce->ind_atrib_posiciones, GL_FLOAT, 2, num_verts, posiciones) );
        vao_ind->agregar( new DescrVBOAtribs( cauce->ind_atrib_colores, GL_FLOAT, 3, num_verts, colores) ) ;
        vao_ind->agregar( new DescrVBOInds( GL_UNSIGNED_INT, num_inds, indices ));
    }
   
    assert( glGetError() == GL_NO_ERROR );
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( false );
    vao_ind->habilitarAtrib( cauce->ind_atrib_colores, true );
    vao_ind->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
   
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_ind->habilitarAtrib( cauce->ind_atrib_colores, false );
    vao_ind->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// usando vectores con entradas de tipos GLM (vec2, vec3, uvec3)
// el triángulo se dibuja en primer lugar relleno con colores, y luego las aristas en negro

void DibujarTriangulo_glm( )
{    
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );

    if ( vao_glm == nullptr )
    {

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        const vector<vec2>   posiciones = {  {-0.4, -0.4},     {+0.42, -0.47},   {0.1, +0.37}    };
        const vector<vec3>   colores    = {  {1.0, 1.0, 0.0},  {0.0, 1.0, 1.0},  {1.0, 0.0, 1.0} };
        const vector<uvec3>  indices    = {  { 0, 1, 2 }};   // (un único triángulo)      

        vao_glm = new DescrVAO( cauce->num_atribs, new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones ));
        vao_glm->agregar( new DescrVBOAtribs( cauce->ind_atrib_colores, colores )) ;
        vao_glm->agregar( new DescrVBOInds( indices ) );

        assert( glGetError() == GL_NO_ERROR );
    }
   
    assert( glGetError() == GL_NO_ERROR );
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( false );
    vao_glm->habilitarAtrib( cauce->ind_atrib_colores, true );
    vao_glm->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
   
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_glm->habilitarAtrib( cauce->ind_atrib_colores, false );
    vao_glm->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 1.1 a)

void DibujarProblema1_1a(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    if ( vao_glm == nullptr )
    {
        // número de vértices que se van a dibujar
        unsigned num_verts = n ;
        float x = 0.0, y = 0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec2>   posiciones;
        for (int i = 0; i < num_verts; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y});
        }

        vao_glm = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));

        assert( glGetError() == GL_NO_ERROR );
    }
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_glm->draw( GL_LINE_LOOP );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 1.1 b)

void DibujarProblema1_1b(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    if ( vao_glm == nullptr )
    {
        // número de vértices que se van a dibujar
        unsigned num_verts = 2*n ;
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec2>   posiciones;
        for (int i = 1; i < num_verts/2+1; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({prev_x, prev_y});
            posiciones.push_back({x, y});
            prev_x = x;
            prev_y = y;
        }

        vao_glm = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_glm->draw( GL_LINES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 1.2 a)

void DibujarProblema1_2a(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    if ( vao_glm == nullptr )
    {
        // número de vértices que se van a dibujar
        unsigned num_verts = 3*n ;
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec3>   posiciones;
        for (int i = 1; i < n+1; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({+0.0, +0.0, +0.0});
            posiciones.push_back({prev_x, prev_y, +0.0});
            posiciones.push_back({x, y, +0.0});
            prev_x = x;
            prev_y = y;
        }

        vao_glm = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_glm->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 1.2 b)

void DibujarProblema1_2b(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    if ( vao_glm == nullptr )
    {
        // número de vértices que se van a dibujar
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec3> posiciones = {{+0.0, +0.0, +0.0}};
        vector<uvec3> indices;

        for (int i = 0; i < n; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y, +0.0});
        }

        for (int i = 1; i < n; ++i) {
            indices.push_back({0, i, i+1});
        }
        indices.push_back({0, n, 1});

        vao_glm = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));
        vao_glm->agregar( new DescrVBOInds( indices ) );

    }
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_glm->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 1.3

void DibujarProblema1_3(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    // copia de 1.1 a)
    if ( vao_no_ind == nullptr )
    {
        // número de vértices que se van a dibujar
        unsigned num_verts = n ;
        float x = 0.0, y = 0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec2>   posiciones;
        for (int i = 0; i < num_verts; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y});
        }

        vao_no_ind = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));

        assert( glGetError() == GL_NO_ERROR );
    }

    assert( glGetError() == GL_NO_ERROR );

    // copia de 1.2 b)
    if ( vao_ind == nullptr )
    {
        // número de vértices que se van a dibujar
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec3> posiciones = {{+0.0, +0.0, +0.0}};
        vector<uvec3> indices;

        for (int i = 0; i < n; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y, +0.0});
        }

        for (int i = 1; i < n; ++i) {
            indices.push_back({0, i, i+1});
        }
        indices.push_back({0, n, 1});

        vao_ind = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones));
        vao_ind->agregar( new DescrVBOInds( indices ) );
    }

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { +1.0, +0.0, +0.0 });
    vao_ind->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    vao_no_ind->draw( GL_LINE_LOOP );

    assert( glGetError() == GL_NO_ERROR );    
}

// ---------------------------------------------------------------------------------------------
// Problema 1.4

void DibujarProblema1_4(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    // copia de 1.2 b)
    if ( vao_noclass == 0 )
    {
        // número de vértices que se van a dibujar
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec3> posiciones = {{+0.0, +0.0, +0.0}};
        vector<uvec3> indices;

        for (int i = 0; i < n; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y, +0.0});
        }

        for (int i = 1; i < n; ++i) {
            indices.push_back({0, i, i+1});
        }
        indices.push_back({0, n, 1});
        
        // creamos vao
        glGenVertexArrays( 1, &vao_noclass ); 
        assert( vao_noclass > 0 );
        glBindVertexArray( vao_noclass );

        // creamos vbo
        DescrVBOAtribs * vbo_pos = new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones);
        dvbo_indices = new DescrVBOInds( indices );
        dvbo_atributo.push_back(vbo_pos);
        // Lo que ancla un vbo a un vao dentro de crearVBO es glVertexAttribPointer
        // En caso de vbo de indices hacer bind lo ancla al vao activo actual
        vbo_pos->crearVBO();
        dvbo_indices->crearVBO();
    } else {
        glBindVertexArray( vao_noclass );
    }

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { +1.0, +0.0, +0.0 });
    glDrawElements(GL_TRIANGLES, dvbo_indices->leerCount(), dvbo_indices->leerType(), 0);

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    // Tenemos n+1 vertices, pero solo queremos n de ellos, sin el vertice 0
    glDrawArrays( GL_LINE_LOOP, 1, n);

    assert( glGetError() == GL_NO_ERROR ); 
    
    // Desactivamos vao 
    glBindVertexArray( 0 );  
}

// ---------------------------------------------------------------------------------------------
// Problema 1.5

void DibujarProblema1_5(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    // copia de 1.2 b)
    if ( vao_noclass == 0 )
    {
        // número de vértices que se van a dibujar
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;
        float r = 0.0, g = 0.0, b = 0.0;
        // randomizamos
        srand (time(NULL));

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        vector<vec3> posiciones = {{+0.0, +0.0, +0.0}};
        vector<vec3> colores;
        vector<uvec3> indices;

        // Generamos vertices
        for (int i = 0; i < n; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            posiciones.push_back({x, y, +0.0});
        }
        // Generamos colores
        for (int i = 0; i < n+1; ++i) {
            r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            colores.push_back({r, g, b});
        }

        // Generamos indices
        for (int i = 1; i < n; ++i) {
            indices.push_back({0, i, i+1});
        }
        indices.push_back({0, n, 1});
        
        // creamos vao
        glGenVertexArrays( 1, &vao_noclass ); 
        assert( vao_noclass > 0 );
        glBindVertexArray( vao_noclass );

        // creamos vbo
        DescrVBOAtribs * vbo_pos = new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones);
        DescrVBOAtribs * vbo_col = new DescrVBOAtribs( cauce->ind_atrib_colores, colores);
        dvbo_indices = new DescrVBOInds( indices );
        dvbo_atributo.push_back(vbo_pos);
        dvbo_atributo.push_back(vbo_col);
        // Lo que ancla un vbo a un vao dentro de crearVBO es glVertexAttribPointer
        // En caso de vbo de indices hacer bind lo ancla al vao activo actual
        for (int i = 0; i < dvbo_atributo.size(); ++i) {
            dvbo_atributo[i]->crearVBO();
        }
        dvbo_indices->crearVBO();
    } else {
        glBindVertexArray( vao_noclass );
    }

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( false );
    glDrawElements(GL_TRIANGLES, dvbo_indices->leerCount(), dvbo_indices->leerType(), 0);

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    // Deshabilitamos tabla de colores
    glDisableVertexAttribArray( cauce->ind_atrib_colores );
    // Tenemos n+1 vertices, pero solo queremos n de ellos, sin el vertice 0
    glDrawArrays( GL_LINE_LOOP, 1, n);

    assert( glGetError() == GL_NO_ERROR ); 
    
    // Desactivamos vao 
    glBindVertexArray( 0 ); 
}

// ---------------------------------------------------------------------------------------------
// Problema 1.6

void DibujarProblema1_6(unsigned n) {
    using namespace std ;
    using namespace glm ;

    assert( glGetError() == GL_NO_ERROR );
    assert(n > 2);

    // copia de 1.2 b)
    if ( vao_noclass == 0 )
    {
        // número de vértices que se van a dibujar
        float x = 0.0, y = 0.0, prev_x = +1.0, prev_y = +0.0, angle = 360.0/n;
        float r = 0.0, g = 0.0, b = 0.0;
        // randomizamos
        srand (time(NULL));

        // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0)
        r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        vector<vec3> aos = {{+0.0, +0.0, +0.0}, {r, g, b}};
        vector<uvec3> indices;

        // Generamos vertices y colores
        for (int i = 0; i < n; ++i) {
            x = float(cos(radians(angle)*i)); 
            y = float(sin(radians(angle)*i));
            r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            aos.push_back({x, y, +0.0});
            aos.push_back({r, g, b});
        }

        // Generamos indices
        for (int i = 1; i < n; ++i) {
            indices.push_back({0, i, i+1});
        }
        indices.push_back({0, n, 1});
        
        // creamos vao
        glGenVertexArrays( 1, &vao_noclass ); 
        assert( vao_noclass > 0 );
        glBindVertexArray( vao_noclass );

        GLint size = 3;
        GLsizei count = 2*(n+1);
        GLsizeiptr tot_size = size*count*sizeof( float );
        // stride es el paso que incluye tanto pos como col, es decir 6 floats
        GLsizei stride = 2*size*sizeof(float);
        // el offset para los colores es 3 floats de las posiciones del primer vertice
        void * offset = (void*)(3*sizeof(float));

        // creamos vbo
        // generar un nuevo identificador de VBO 
        glGenBuffers( 1, &buffer ); assert( 0 < buffer );

        // fija este buffer como buffer 'activo' actualmente en el 'target' GL_ARRAY_BUFFER
        glBindBuffer( GL_ARRAY_BUFFER, buffer ); 

        // transfiere los datos desde la memoria de la aplicación al VBO en GPU
        glBufferData( GL_ARRAY_BUFFER, tot_size, aos.data(), GL_STATIC_DRAW );  
            
        // indicar, para este índice de atributo, la localización y el formato de la tabla en el buffer 
        glVertexAttribPointer( cauce->ind_atrib_posiciones, size, GL_FLOAT, 
        GL_FALSE, stride, 0  );
        glVertexAttribPointer( cauce->ind_atrib_colores, size, GL_FLOAT, 
        GL_FALSE, stride, offset);

        // desactivar el buffer
        glBindBuffer( GL_ARRAY_BUFFER, 0 );

        // por defecto, habilita el uso de esta tabla de atributos
        glEnableVertexAttribArray( cauce->ind_atrib_posiciones );
        glEnableVertexAttribArray( cauce->ind_atrib_colores );

        // comprobar que no ha habido error durante la creación del VBO
        CError();
        dvbo_indices = new DescrVBOInds( indices );
        dvbo_indices->crearVBO();
    } else {
        glBindVertexArray( vao_noclass );
    }

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( false );
    glDrawElements(GL_TRIANGLES, dvbo_indices->leerCount(), dvbo_indices->leerType(), 0);

    assert( glGetError() == GL_NO_ERROR ); 

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { 0.0, 0.0, 0.0 });
    // Deshabilitamos tabla de colores
    glDisableVertexAttribArray( cauce->ind_atrib_colores );
    // Tenemos n+1 vertices, pero solo queremos n de ellos, sin el vertice 0
    glDrawArrays( GL_LINE_LOOP, 1, n);

    assert( glGetError() == GL_NO_ERROR ); 
    
    // Desactivamos vao 
    glBindVertexArray( 0 ); 
}

// ---------------------------------------------------------------------------------------------
// Problema 2.15

void gancho() {
    using namespace std ;
    using namespace glm ;
    assert( glGetError() == GL_NO_ERROR );

    // la primera vez, crear e inicializar el VAO
    if ( vao_glm == nullptr )
    {
        const vector<vec2>   posiciones = { {+0.0, +0.0}, {+0.25, +0.0}, {+0.25, +0.25},
        {+0.0, +0.25}, {+0.0, +0.5}
        };     
        vao_glm = new DescrVAO( cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones )); 
    }

    assert( glGetError() == GL_NO_ERROR );

    // dibujar las líneas usando color negro
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    vao_glm->draw( GL_LINE_STRIP );

    assert( glGetError() == GL_NO_ERROR );  
}

// ---------------------------------------------------------------------------------------------
// Problema 2.16

void gancho_x4() {
    using namespace std ;
    using namespace glm ;
    assert( glGetError() == GL_NO_ERROR );

    // dibujar las líneas usando color negro
    for (int i = 0; i < 4; ++i) {
        gancho();
        cauce->compMM(translate(vec3{+0.0, +0.5, +0.0}));
        cauce->compMM(rotate(radians(90.0f), vec3{+0.0, +0.0, +1.0}));
    }

    assert( glGetError() == GL_NO_ERROR );  
}

// ---------------------------------------------------------------------------------------------
// Problema 2.17 a)

void gancho_2p(glm::vec3 p0, glm::vec3 p1) {
    using namespace std;
    using namespace glm;
    assert( glGetError() == GL_NO_ERROR );
    float l = length(p0-p1);
    float angle = atan2(p1.y-p0.y,p1.x-p0.x) - radians(90.0);

    cauce->resetMM();
    cauce->compMM(translate(vec3{p0.x, p0.y, 0}));
    cauce->compMM(rotate(angle, vec3{+0.0, +0.0, +1.0}));
    cauce->compMM(scale(vec3{l/0.5, l/0.5, 1}));
    gancho();

    assert( glGetError() == GL_NO_ERROR );  
}

// ---------------------------------------------------------------------------------------------
// Problema 2.18

void gancho_2pcirculo(unsigned n) {
    using namespace std;
    using namespace glm;
    assert(n>2);
    assert( glGetError() == GL_NO_ERROR );
    float angle = 360.0/n;
    vec3 p = {+0.5, +0.0, +0.0}, prev = p;
    mat3 rot = rotate(radians(angle), vec3{+0.0, +0.0, +1.0});

    for (int i = 0; i < n; ++i) {
        p = rot*p;
        gancho_2p(prev, p);
        prev = p;
    }

    assert( glGetError() == GL_NO_ERROR );  
}

// ---------------------------------------------------------------------------------------------
// Problema 2.20

void FiguraSimple() {
    using namespace std;
    using namespace glm;
    assert( glGetError() == GL_NO_ERROR );
    if (!vao_cuad) {
        vector<vec3> posiciones = {{+0.0, +0.0, +1.0}, {+0.5, +0.0, +1.0}, {+0.5, +0.5, +1.0}, 
        {+0.0, +0.0, +1.0}, {+0.0, +0.5, +1.0}, {+0.5, +0.5, +1.0}};
        vao_cuad = new DescrVAO(cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones ));
    }
    assert( glGetError() == GL_NO_ERROR );
    if (!vao_trian) {
        vector<vec3> posiciones = {{+0.1, +0.1, -1.0}, {+0.3, +0.1, -1.0}, {+0.2, +0.3, -1.0}};
        vao_trian = new DescrVAO(cauce->num_atribs, 
        new DescrVBOAtribs( cauce->ind_atrib_posiciones, posiciones ));
    }
    
    assert( glGetError() == GL_NO_ERROR );
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( true );
    cauce->pushColor();
    cauce->fijarColor( { 0.0, 0.0, 1.0 });
    vao_cuad->draw( GL_TRIANGLES );

    cauce->popColor();
    assert( glGetError() == GL_NO_ERROR );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    vao_trian->draw( GL_TRIANGLES );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// Problema 2.21

void FiguraCompleja() {
    using namespace std;
    using namespace glm;
    cauce->compMM(scale(vec3(+0.25, +0.25, +1.0)));
    cauce->pushMM();
        cauce->compMM(translate(vec3{+1.0, +1.0, +0.0}));
        cauce->compMM(rotate(radians(-45.0f), vec3{+0.0, +0.0, +1.0}));
        cauce->compMM(scale(vec3{sqrt(2), -sqrt(2), +1.0}));
        FiguraSimple();
    cauce->popMM();
    cauce->pushMM();
        cauce->compMM(translate(vec3{+1.5, +0.5, +0.0}));
        cauce->compMM(scale(vec3{+2.0, -1.0, +1.0}));
        FiguraSimple();
    cauce->popMM();
    FiguraSimple();
}

// ---------------------------------------------------------------------------------------------
// Problema 2.22

void Tronco() {
    using namespace std;
    using namespace glm;
    assert( glGetError() == GL_NO_ERROR );
    cauce->pushMM();
    cauce->compMM(scale(vec3{+0.1, +0.1, +1.0}));
    if (!vao_ind) {
        vector<vec2> posiciones = {
            {+0.0, +0.0}, {+1.0, +0.0}, {+1.0, +1.0},
            {+2.0, +2.0}, {+1.5, +2.5}, {+0.5, +1.5},
            {+0.0, +3.0}, {-0.5, +3.0}, {+0.0, +1.5}
        };
        vector<uvec3> indices = {
            {8, 5, 7}, {5, 6, 7}, {8, 2, 5},
            {5, 2, 4}, {2, 3, 4}, {0, 1, 8},
            {1, 2, 8}
        };
        vao_ind = new DescrVAO(cauce->num_atribs, 
        new DescrVBOAtribs(cauce->ind_atrib_posiciones, posiciones));
        vao_ind->agregar(new DescrVBOInds(indices));
    }
    if (!vao_no_ind) {
        vector<vec2> posiciones = {
            {+0.0, +0.0}, {+0.0, +1.5}, {+0.0, +1.5}, 
            {-0.5, +3.0}, {+0.0, +3.0}, {+0.5, +1.5}, 
            {+0.5, +1.5}, {+1.5, +2.5}, {+2.0, +2.0}, 
            {+1.0, +1.0}, {+1.0, +1.0}, {+1.0, +0.0}
        };
        vao_no_ind = new DescrVAO(cauce->num_atribs, 
        new DescrVBOAtribs(cauce->ind_atrib_posiciones, posiciones));
    }
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { 0.5, 0.5, 1.0 });
    vao_ind->draw( GL_TRIANGLES );

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    cauce->fijarUsarColorPlano( true );
    cauce->fijarColor( { 0.0, 0.0, 1.0 });
    vao_no_ind->draw( GL_LINES );

    cauce->popMM();
}

// ---------------------------------------------------------------------------------------------
// Problema 2.23
void Arbol(unsigned n) {
    using namespace std;
    using namespace glm;
    // Caso base
    if (n == 0) {
        return;
    }
    Tronco();
    // Llamada recursiva izda
    cauce->pushMM();
        cauce->compMM(translate(vec3{-0.5*0.1, +3.0*0.1, +0.0}));
        cauce->compMM(scale(vec3{+0.5, +0.5, +1.0}));
        Arbol(n-1);
    cauce->popMM();
    // Llamada recursiva derecha
    cauce->pushMM();
        cauce->compMM(translate(vec3{+1.5*0.1, +2.5*0.1, +0.0}));
        cauce->compMM(rotate(radians(-45.0f), vec3{+0.0, +0.0, +1.0}));
        cauce->compMM(scale(vec3{sqrt(0.5), sqrt(0.5), +1.0}));
        Arbol(n-1);
    cauce->popMM();
}

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar el contenido en la ventana

void VisualizarFrame( )
{
    using namespace std ;
    using namespace glm ;

    // comprobar y limpiar variable interna de error
    assert( glGetError() == GL_NO_ERROR );

    // usar (acrivar) el objeto programa (no es necesario hacerlo en 
    // cada frame si solo hay uno de estos objetos, pero se incluye 
    // para hacer explícito que el objeto programa debe estar activado)
    cauce->activar();

    // establece la zona visible (toda la ventana)
    glViewport( 0, 0, ancho_actual, alto_actual );

    // fija la matriz de transformación de posiciones de los shaders 
    // (la hace igual a la matriz identidad)
    cauce->resetMM();

    // fija la matriz de proyeccion (la hace igual a la matriz identidad)
    cauce->fijarMatrizProyeccion( glm::mat4(1.0) );

    // limpiar la ventana
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // habilitar EPO por Z-buffer (test de profundidad)
    glDisable( GL_DEPTH_TEST );

    // evitamos deformacion problema 1.8
    //cauce->fijarRegionVisible(-float(ancho_actual)/512, -float(alto_actual)/512, -1.0, 
    //float(ancho_actual)/512, float(alto_actual)/512, +1.0);

    // Dibujar un triángulo, es una secuncia de vértice no indexada.
    //DibujarTriangulo_NoInd();
    

    // usa el color plano para el segundo triángulo
    //cauce->fijarUsarColorPlano( true );

    // dibujar triángulo indexado (rotado y luego desplazado) 
    /*
    cauce->pushMM();
        cauce->compMM( translate( vec3{ 0.4f, 0.1f, -0.1f}  ));
        cauce->compMM( rotate(  radians(23.0f), vec3{ 0.0f, 0.0f, 1.0f}   ));
        DibujarTriangulo_Ind();     // indexado

    cauce->popMM();
    */
    // dibujar un triángulo usando vectores de GLM
    //DibujarTriangulo_glm() ;

    //DibujarProblema1_6(5);
    //gancho_2p(vec3{+0.0, +0.0, +0.0}, vec3{+0.3, -0.3, +0.0});
    //gancho_2pcirculo(12);
    //FiguraCompleja();
    Arbol(5);
    // Limpiamos memoria
   for( unsigned i = 1 ; i < dvbo_atributo.size() ; i++ )
   {  
      delete dvbo_atributo[i] ;
      dvbo_atributo[i] = nullptr ; 
   }
   if (dvbo_indices != nullptr ) {
    delete dvbo_indices ;
    dvbo_indices = nullptr ; 
   }
   
    if (buffer != 0) {
        CError();
      glDeleteBuffers( 1, &buffer );
      CError();
      buffer = 0 ; // probablemente innecesario
    }

   if ( vao_noclass != 0 )
   {
      CError();
      glDeleteVertexArrays( 1, &vao_noclass );
      CError();
      vao_noclass = 0 ; // probablemente innecesario
   }

    // comprobar y limpiar variable interna de error
    assert( glGetError() == GL_NO_ERROR );

    // esperar a que termine 'glDrawArrays' y entonces presentar el framebuffer actualizado
    glfwSwapBuffers( ventana_glfw );

}


// ---------------------------------------------------------------------------------------------
// función que se invoca cada vez que cambia el número de pixels del framebuffer
// (cada vez que se redimensiona la ventana)

void FGE_CambioTamano( GLFWwindow* ventana, int nuevo_ancho, int nuevo_alto )
{
    using namespace std ;
    //cout << "FGE cambio tamaño, nuevas dimensiones: " << nuevo_ancho << " x " << nuevo_alto << "." << endl ;

    ancho_actual      = nuevo_ancho ;
    alto_actual       = nuevo_alto ;

    redibujar_ventana = true ; // fuerza a redibujar la ventana
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que se pulse o levante una tecla.

void FGE_PulsarLevantarTecla( GLFWwindow* ventana, int key, int scancode, int action, int mods )
{
    using namespace std ;
    //cout << "FGE pulsar levantar tecla, número de tecla == " << key << "." << endl ;
    // si se pulsa la tecla 'ESC', acabar el programa
    if ( key == GLFW_KEY_ESCAPE )
        terminar_programa = true ;
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que se pulse o levante un botón del ratón

void FGE_PulsarLevantarBotonRaton( GLFWwindow* ventana, int button, int action, int mods )
{
    // nada, por ahora

}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que cambie la posición del puntero

void FGE_MovimientoRaton( GLFWwindow* ventana, double xpos, double ypos )
{
    // nada, por ahora
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que mueva la rueda del ratón.

void FGE_Scroll( GLFWwindow* ventana, double xoffset, double yoffset )
{
    // nada, por ahora
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cuando se produzca un error de GLFW

void ErrorGLFW ( int error, const char * description )
{
    using namespace std ;
    cerr
        << "Error en GLFW. Programa terminado" << endl
        << "Código de error : " << error << endl
        << "Descripción     : " << description << endl ;
    exit(1);
}
// ---------------------------------------------------------------------------------------------
// código de inicialización de GLFW

void InicializaGLFW( int argc, char * argv[] )
{
    using namespace std ;

    // intentar inicializar, terminar si no se puede
    glfwSetErrorCallback( ErrorGLFW );
    if ( ! glfwInit() )
    {
        cout << "Imposible inicializar GLFW. Termino." << endl ;
        exit(1) ;
    }

    // especificar versión de OpenGL y parámetros de compatibilidad que se querrán
   // (pedimos opengl 330, tipo "core" (sin compatibilidad con versiones anteriores)
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 ); 
   glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE ); // indica que tambien debe funcionar si se usa con un driver con version superior a la 3.3
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // indica que no es compatible hacia atrás con versiones previas a la 3.3

    // especificar que función se llamará ante un error de GLFW
    glfwSetErrorCallback( ErrorGLFW );

    // crear la ventana (var. global ventana_glfw), activar el rendering context
    ventana_glfw = glfwCreateWindow( 512, 512, "IG ejemplo mínimo (OpenGL 3+)", nullptr, nullptr );
    glfwMakeContextCurrent( ventana_glfw ); // necesario para OpenGL

    // leer y guardar las dimensiones del framebuffer en pixels
    glfwGetFramebufferSize( ventana_glfw, &ancho_actual, &alto_actual );

    // definir cuales son las funciones gestoras de eventos (con nombres 'FGE_....')
    glfwSetFramebufferSizeCallback( ventana_glfw, FGE_CambioTamano );
    glfwSetKeyCallback            ( ventana_glfw, FGE_PulsarLevantarTecla );
    glfwSetMouseButtonCallback    ( ventana_glfw, FGE_PulsarLevantarBotonRaton);
    glfwSetCursorPosCallback      ( ventana_glfw, FGE_MovimientoRaton );
    glfwSetScrollCallback         ( ventana_glfw, FGE_Scroll );
}

// ---------------------------------------------------------------------------------------------
// función para inicializar GLEW (necesario para las funciones de OpenGL ver 2.0 y posteriores)
// en macOS no es necesario (está vacía)

void InicializaGLEW()
{
#ifndef __APPLE__
    using namespace std ;
    GLenum codigoError = glewInit();
    if ( codigoError != GLEW_OK ) // comprobar posibles errores
    {
        cout << "Imposible inicializar ’GLEW’, mensaje recibido: " << endl
             << (const char *)glewGetErrorString( codigoError ) << endl ;
        exit(1);
    }
    else
        cout << "Librería GLEW inicializada correctamente." << endl << flush ;

#endif
}

// ---------------------------------------------------------------------------------------------

void InicializaOpenGL()
{
    using namespace std ;
    
    assert( glGetError() == GL_NO_ERROR );

    cout  << "Datos de versión e implementación de OpenGL" << endl
         << "    Implementación de : " << glGetString(GL_VENDOR)  << endl
         << "    Hardware          : " << glGetString(GL_RENDERER) << endl
         << "    Versión de OpenGL : " << glGetString(GL_VERSION) << endl
         << "    Versión de GLSL   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl ;

    
    InicializaGLEW(); // En linux y windows, fija punteros a funciones de OpenGL version 2.0 o superiores
    
    
    assert( glGetError() == GL_NO_ERROR );
    
    glClearColor( 1.0, 1.0, 1.0, 0.0 ); // color para 'glClear' (blanco, 100% opaco)
    glDisable( GL_CULL_FACE );          // dibujar todos los triángulos independientemente de su orientación
    cauce = new Cauce() ;            // crear el objeto programa (variable global 'cauce')
    
    assert( cauce != nullptr );
    assert( glGetError() == GL_NO_ERROR );
}
// ---------------------------------------------------------------------------------------------

void BucleEventosGLFW()
{
    while ( ! terminar_programa )
    {   
        if ( redibujar_ventana )
        {   
            VisualizarFrame();
            redibujar_ventana = false; // (evita que se redibuje continuamente)
        }
        glfwWaitEvents(); // esperar evento y llamar FGE (si hay alguna)
        terminar_programa = terminar_programa || glfwWindowShouldClose( ventana_glfw );
    }
}
// ---------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    using namespace std ;
    cout << "Programa mínimo de OpenGL 3.3 o superior" << endl ;

    InicializaGLFW( argc, argv ); // Crea una ventana, fija funciones gestoras de eventos
    InicializaOpenGL() ;          // Compila vertex y fragment shaders. Enlaza y activa programa. Inicializa GLEW.
    BucleEventosGLFW() ;          // Esperar eventos y procesarlos hasta que 'terminar_programa == true'
    glfwTerminate();              // Terminar GLFW (cierra la ventana)

    cout << "Programa terminado normalmente." << endl ;
}
